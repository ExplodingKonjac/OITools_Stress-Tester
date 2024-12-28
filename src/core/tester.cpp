#include "tester.h"

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;

Tester::~Tester()
{
	for(auto &t: threads)
		if(t.joinable())
			t.join();
	if(fs::exists(prefix))
	{
		boost::system::error_code ec;
		fs::remove_all(prefix,ec);
		if(ec)
			msg.error("failed to remove temp directory ({0}): {1}",ec.value(),ec.message());
	}
}

void Tester::judgingThread(std::size_t id)
{
	Judger &judger=*judgers[id];
	while(!flag_stop && tot<opt.test_cnt)
	{
		JudgeResult res=judger.judge();
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_pause.wait(lock,[this] {
			return !flag_pause;
		});
		if(!flag_stop && res.type!=JudgeResult::TERM)
		{
			if(res.type!=JudgeResult::OK)
				flag_stop.store(true);
			result_q.emplace(id,res);
			tot++;
			cond_q.notify_one();
		}
	}
}

fs::path Tester::getExePath(const std::string &name,bool in_path)
{
	bp::environment::value val(fs::current_path().string());
	if(in_path)
		val.push_back(bp::environment::get("PATH"));
	std::unordered_map<bp::environment::key,bp::environment::value> new_env{
		{"PATH",val},
		{"PATHEXT",bp::environment::get("PATHEXT")}
	};
	return bp::environment::find_executable(name,new_env);
}

void Tester::compileOne(const std::string &filename,const std::vector<std::string> &extra_opt,std::mutex &mtx,std::ofstream &fout,std::exception_ptr &ep)
{
	try
	{
		std::vector<std::string> compiler_opt{filename+".cpp","-o",filename};
		compiler_opt+=extra_opt;

		as::io_context ctx;
		as::readable_pipe pipe(ctx);
		bp::process proc(
			ctx,
			bp::environment::find_executable("g++"),
			compiler_opt,
			bp::process_stdio{{},{},pipe}
		);

		std::string error_message;
		boost::system::error_code ec;
		as::read(pipe,as::dynamic_buffer(error_message),ec);

		int exit_code=proc.wait();

		std::lock_guard lock(mtx);
		if(exit_code==0)
		{
			msg.print("{0}.cpp: successfully compiled\n",filename);
			fout<<std::format("{0}.cpp: successfully compiled\ncompiler messages:\n",filename);
			fout<<error_message<<'\n';
		}
		else
		{
			msg.print("{0}.cpp: compile error\n",filename);
			fout<<std::format("{0}.cpp: compile error\ncompiler messages:\n",filename);
			fout<<error_message<<'\n';
			ep=std::make_exception_ptr(std::runtime_error(
				std::format("compile error on {0}.cpp",filename)
			));
		}
	}
	catch(const std::exception& e)
	{
		std::lock_guard lock(mtx);
		ep=std::make_exception_ptr(std::runtime_error(
			std::format("failed when compiling {0}.cpp: {1}",filename,e.what())
		));
	}
}

void Tester::compileExecutables()
{
	std::vector<std::thread> threads;
	std::mutex mtx;
	std::ofstream fout("compile.log");
	std::exception_ptr ep;

	auto addone=[&](const std::string &name,const std::vector<std::string> &opt) {
		threads.emplace_back(&Tester::compileOne,name,opt,std::ref(mtx),std::ref(fout),std::ref(ep));
	};

	addone(opt.exe_name,opt.opt_compile_exe);
	addone(opt.std_name,opt.opt_compile_exe);
	if(opt.compile_gen)
		addone(opt.gen_name,opt.opt_compile_gen);
	if(opt.compile_chk)
		addone(opt.chk_name,opt.opt_compile_chk);
	for(auto &t: threads)
		t.join();

	if(ep) std::rethrow_exception(ep);
}

fs::path Tester::createTempDirectory()
{
	fs::path res(".stress-"+fs::unique_path().string());
	boost::system::error_code ec;
	fs::create_directory(res,ec);
	if(ec)
		throw std::runtime_error(
			std::format("failed to create temp directory ({0}): {1}",ec.value(),ec.message())
		);
	return res;
}

void Tester::handleWrongAnswer(std::size_t idx,std::size_t id)
{
	std::string chk_msg(256,'\0'),hint_msg;
	std::ifstream inf(judgers[id]->getLogPath());

	inf.read(chk_msg.data(),chk_msg.size());
	std::size_t sz=inf.gcount();
	if(sz==chk_msg.size())
		hint_msg=std::format("Failed on testcase #{0} (256 bytes only):\n{1}...\nSee {2}.log for detail.",idx,chk_msg,opt.file);
	else
		hint_msg=std::format("Failed on testcase #{0}:\n{1}",idx,chk_msg,opt.file);

	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=9},"Wrong Answer\n");
#if defined(_WIN32)
	MessageBox(nullptr,hint_msg.c_str(),"Oops",MB_ICONERROR);
#elif defined(__linux__)
	as::io_context ctx;
	bp::process msgbox(
		ctx,
		bp::environment::find_executable("zenity"),
		{"--error","--title=Oops","--text",hint_msg}
	);
	msgbox.wait();
#endif
}

void Tester::handleBadResult(const std::string &name,const ProcessInfo &info,std::size_t tl,std::size_t ml)
{
	switch(info.type)
	{
	 case ProcessInfo::TLE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"{0} time limit exceeded",name);
		if(info.time_used==(std::size_t)-1)
			msg.print(" (killed)\n");
		else
			msg.print(" ({0:.2}ms/{1:.2}ms)\n",info.time_used/1024.0,tl/1024.0);
		break;

	 case ProcessInfo::MLE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"{0} memory limit exceeded",name);
		msg.print(" ({0:.2}MB/{1:.2}MB)\n",info.memory_used/1024.0/1024.0,ml/1024.0/1024.0);
		break;

	 case ProcessInfo::RE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=9},"{0} runtime error",name);
		msg.print(" ({})\n",info.exit_code);
		break;

	 default:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=5},"{0} unknown error\n",name);
	}
}

void Tester::moveFiles(std::size_t id)
{
	auto tryMove=[&](const fs::path &from,const fs::path &to) {
		boost::system::error_code ec;
		if(fs::exists(to) && fs::is_regular_file(to))
			fs::remove(to,ec);
		fs::rename(from,to,ec);
		if(ec) msg.error("failed to get {0} ({1}): {2}",to.filename().string(),ec.value(),ec.message());
	};

	const Judger &judger=*judgers[id];
	tryMove(judger.getInputPath(),opt.file+".in");
	tryMove(judger.getOutputPath(),opt.file+".out");
	tryMove(judger.getAnswerPath(),opt.file+".ans");
	tryMove(judger.getLogPath(),opt.file+".log");
}

void Tester::start()
{
	compileExecutables();

	result_q={};
	tot=0;
	flag_stop.store(false);
	flag_pause.store(false);

	exe_path=getExePath(opt.exe_name);
	std_path=getExePath(opt.std_name);
	gen_path=getExePath(opt.gen_name,!opt.compile_gen);
	chk_path=getExePath(opt.chk_name,!opt.compile_chk);
	prefix=createTempDirectory();

	threads.clear();
	judgers.clear();
	for(std::size_t i=0;i<opt.thread_cnt;i++)
		judgers.push_back(std::make_unique<Judger>(std::to_string(i),prefix,exe_path,std_path,gen_path,chk_path));
	for(std::size_t i=0;i<opt.thread_cnt;i++)
		threads.emplace_back(&Tester::judgingThread,this,i);

	static std::function<void()> tryQuit;
	tryQuit=[&] {
		flag_pause.store(true);
		for(auto &judger: judgers)
			judger->terminate();
		cond_q.notify_one();
	};
	std::signal(SIGINT,[](int){ tryQuit(); });

	std::size_t idx=0;
	while(!flag_stop && idx<opt.test_cnt)
	{
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_q.wait(lock,[this] {
			return !result_q.empty() || flag_pause;
		});
		if(flag_pause)
		{
			lock.unlock();

			std::string res;
			std::cout<<"quit testing? (y/n): ";
			std::getline(std::cin,res);
			flag_pause.store(false);
			if(res=="y" || res=="Y")
			{
				flag_stop.store(true);
				cond_pause.notify_all();
				break;
			}
			std::signal(SIGINT,[](int){ tryQuit(); });
			cond_pause.notify_all();
			continue;
		}

		auto [id,result]=result_q.front();
		result_q.pop();
		lock.unlock();

		msg.print("Testcase #{0}: ",++idx);
		switch(result.type)
		{
		 case JudgeResult::OK:
			msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=10},"Accepted.\n");
			break;

		 case JudgeResult::WA:
			handleWrongAnswer(idx,id);
			goto bad_result;

		 case JudgeResult::EXE_ERR:
			handleBadResult("Testee",result.info,opt.tl,opt.ml);
			goto bad_result;

		 case JudgeResult::STD_ERR:
			handleBadResult("Standard",result.info,opt.tl,opt.ml);
			goto bad_result;

		 case JudgeResult::GEN_ERR:
			handleBadResult("Generator",result.info,opt.tl_gen,opt.ml_gen);
			goto bad_result;

		 case JudgeResult::CHK_ERR:
			handleBadResult("Checher",result.info,opt.tl_chk,opt.ml_chk);
			goto bad_result;
		
		 case JudgeResult::TERM:
			msg.warning("well sth impossible just happened...");
			break;

		 bad_result:
			moveFiles(id);
			break;
		}
	}
}
