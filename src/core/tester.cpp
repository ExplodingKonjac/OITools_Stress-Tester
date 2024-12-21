#include "tester.h"

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;

Tester::Tester()
{}

Tester::~Tester()
{
	for(auto &t: threads)
		if(t.joinable())
			t.join();
	if(fs::exists(prefix))
	{
		boost::system::error_code ec;
		fs::remove(prefix,ec);
		if(ec)
			msg.error("failed to remove temp directory ({0}): {1}",ec.value(),ec.message());
	}
}

void Tester::judgingThread(int id)
{
	Judger &judger=judgers[id];
	while(!stop_flag && tot<opt.test_cnt)
	{
		JudgeResult res=judger.judge();
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_pause.wait(lock,[this] {
			return !pause_flag;
		});
		if(!stop_flag)
		{
			if(res.type!=JudgeResult::OK)
				stop_flag=true;
			result_q.emplace(id,res);
			tot++;
			cond_q.notify_one();
		}
	}
}

fs::path Tester::getExePath(const std::string &name,bool in_path=false)
{
	bp::environment::value val(".");
	if(in_path)
	{
		auto path_value=bp::environment::get("PATH");
		val.push_back(path_value);
	}
	std::unordered_map<bp::environment::key,bp::environment::value> new_env{
		{"PATH",val}
	};
	return bp::environment::find_executable(name,bp::process_environment(new_env));
}

void Tester::compileOne(const std::string &filename,const std::vector<std::string> &extra_opt,std::mutex &mtx,std::ofstream &fout,std::exception_ptr &ep)
{
	try
	{
		std::vector<std::string> compiler_opt{filename+".cpp","-o",filename};
		compiler_opt.insert(compiler_opt.end(),extra_opt.begin(),extra_opt.end());

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
			fout<<std::format("{0}.cpp: successfully compiled\ncompiler messages:\n");
			fout<<error_message<<'\n';
		}
		else
		{
			msg.print("{0}.cpp: compile error\n",filename);
			fout<<std::format("{0}.cpp: compile error\ncompiler messages:\n");
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
		std::thread t(&Tester::compileOne,name,opt,mtx,fout,ep);
		threads.push_back(std::move(t));
	};

	addone(opt.exe_name,opt.compiler_opt);
	addone(opt.std_name,opt.compiler_opt);
	if(opt.compile_gen)
		addone(opt.gen_name,{});
	if(opt.compile_chk)
		addone(opt.chk_name,{});
	for(auto &t: threads)
		t.join();

	if(ep) std::rethrow_exception(ep);
}

fs::path Tester::createTempDirectory()
{
	fs::path res("stress-"+fs::unique_path().string());
	boost::system::error_code ec;
	fs::create_directory(res,ec);
	if(ec)
		throw std::runtime_error(
			std::format("failed to create temp directory ({0}): {1}",ec.value(),ec.message())
		);
	return res;
}

void Tester::handleWrongAnswer(std::size_t idx,int id)
{
	std::string chk_msg(256,'\0'),hint_msg;
	std::ifstream inf(judgers[id].getLogPath());

	inf.read(chk_msg.data(),chk_msg.size());
	std::size_t sz=inf.gcount();
	if(sz==chk_msg.size())
		hint_msg=std::format("Failed on testcase #{0} (256 bytes only):\n{1}...\nSee {2}.log for detail.",idx,chk_msg,opt.file);
	else
		hint_msg=std::format("Failed on testcase #{0}:\n{1}",idx,chk_msg,opt.file);

	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=9},"Wrong Answer.\n");
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
	msg.print("{} ",name);
	switch(info.type)
	{
	 case ProcessInfo::TLE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"time limit exceeded ");
		if(info.time_used==(std::size_t)-1)
			msg.print(" (killed)\n");
		else
			msg.print(" ({0:.2}ms/{1:.2}ms)\n",info.time_used/1024.0,tl/1024.0);
		break;

	 case ProcessInfo::MLE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"memory limit exceeded ");
		msg.print(" ({0:.2}MB/{1:.2}MB)\n",info.memory_used/1024.0/1024.0,ml/1024.0/1024.0);
		break;

	 case ProcessInfo::RE:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=9},"runtime error");
		msg.print(" ({})\n",info.exit_code);
		break;

	 default:
		msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=5},"unknown error\n");
	}
}

void Tester::moveFiles(int id)
{
	auto tryMove=[&](const fs::path &from,const fs::path &to) {
		boost::system::error_code ec;
		fs::rename(from,to,ec);
		if(ec)
			err.error("failed to get {0} ({1}): {2}",to.filename(),ec.value(),ec.message());
	};

	const Judger &judger=judgers[id];
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
	stop_flag=false;
	pause_flag=false;

	exe_path=getExePath(opt.exe_name),
	std_path=getExePath(opt.std_name),
	gen_path=getExePath(opt.gen_name,!opt.compile_gen),
	chk_path=getExePath(opt.chk_name,!opt.compile_chk),
	prefix=createTempDirectory();

	threads.clear();
	judgers.clear();
	for(std::size_t i=0;i<opt.thread_cnt;i++)
		judgers.emplace_back(std::to_string(i),prefix,exe_path,std_path,gen_path,chk_path);
	for(std::size_t i=0;i<opt.thread_cnt;i++)
		threads.emplace_back(&Tester::judgingThread,this,i);

	static std::function<void()> tryQuit;
	tryQuit=[&] {
		pause_flag=true;
		for(auto &judger: judgers)
			judger.terminate();
		cond_q.notify_one();
	};
	std::signal(SIGINT,[](int){ tryQuit(); });

	for(std::size_t idx=0;!stop_flag && idx<opt.thread_cnt;idx++)
	{
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_q.wait(lock,[this] {
			return !result_q.empty() || pause_flag;
		});
		if(pause_flag)
		{
			std::string res;
			
			msg.print("quit testing? (y/n): ");
			std::getline(std::cin,res);
			if(res=="y" || res=="Y")
			{
				stop_flag=true;
				cond_pause.notify_all();
				break;
			}
			stop_flag=false;
			std::signal(SIGINT,[](int){ tryQuit(); });
			cond_pause.notify_all();
		}

		auto [id,result]=result_q.front();
		result_q.pop();
		lock.unlock();

		msg.print("Testcase #{0}: ",idx);
		switch(result.type)
		{
		 case JudgeResult::OK:
			msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=10},"Accepted.\n");
			break;

		 case JudgeResult::WA:
			handleWrongAnswer(idx,id);
			break;

		 case JudgeResult::EXE_ERR:
			handleBadResult("Testee",result.info,opt.tl,opt.ml);
			break;

		 case JudgeResult::STD_ERR:
			handleBadResult("Standard",result.info,opt.tl,opt.ml);
			break;

		 case JudgeResult::GEN_ERR:
			handleBadResult("Generator",result.info,opt.tl_gen,opt.ml_gen);
			break;

		 case JudgeResult::CHK_ERR:
			handleBadResult("Checher",result.info,opt.tl_chk,opt.ml_chk);
			break;
		}
	}
}
