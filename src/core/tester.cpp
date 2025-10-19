#include "tester.h"

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;
namespace ph=std::placeholders;

Tester::~Tester()
{
	flag_stop=true;
	for(auto &t: threads)
	{
		t->interrupt();
		t->wait();
	}
	if(fs::exists(prefix))
	{
		boost::system::error_code ec;
		fs::remove_all(prefix,ec);
		if(ec) msg.error("failed to remove temp directory ({0}): {1}",ec.value(),ec.message());
	}
}

Tester::JudgeThread::JudgeThread(Tester &parent,std::size_t id):
	_parent(parent),
	_id(id),
	_judger(
		std::to_string(id),
		parent.prefix,
		parent.exe_path,
		parent.std_path,
		parent.gen_path,
		parent.chk_path
	),
	_thread(std::bind(&run,this,ph::_1))
{}

void Tester::JudgeThread::run(std::stop_token stoken) try
{
	auto check_proc_info=[&](std::string_view name,
							const ProcessInfo &info,
							std::uint64_t tl,
							std::uint64_t ml)->ResultType {
		switch(info.type)
		{
		 case ProcessInfo::Types::TLE:
			return TLEResult{"Generator",info.time_used,opt.tl_gen};

		 case ProcessInfo::Types::MLE:
			return MLEResult{"Generator",info.memory_used,opt.ml_gen};

		 case ProcessInfo::Types::RE:
			return REResult{"Generator",info.exit_code};

		 default:
			return UKEResult{"Generator"};
		}
	};
	auto check_judge_res=[&](const JudgeResult &res)->ResultType {
		switch(res.type)
		{
		 case JudgeResult::Types::OK:
			return ACResult{};

		 case JudgeResult::Types::WA:
			return [&] {
				std::ifstream log_stream(_judger.getLogPath());
				std::string buffer(256,'\0');

				log_stream.read(buffer.data(),buffer.size());
				auto sz=log_stream.gcount();
				if(sz==buffer.size()) buffer+="...";
				else buffer.resize(sz);

				return WAResult{buffer};
			}();

		 case JudgeResult::Types::GEN_ERR:
			return check_proc_info("Generator",res.gen_info,opt.tl_gen,opt.ml_gen);
		
		 case JudgeResult::Types::EXE_ERR:
			return check_proc_info("Testee",res.exe_info,opt.tl,opt.ml);
		
		 case JudgeResult::Types::STD_ERR:
			return check_proc_info("Checker",res.std_info,opt.tl,opt.ml);

		 case JudgeResult::Types::CHK_ERR:
			return check_proc_info("Checker",res.chk_info,opt.tl_chk,opt.ml_chk);
		}
		assert(0);
		__builtin_unreachable();
	};

	while(_parent.tot<opt.test_cnt)
	{
		_parent.flag_pause.wait(true);
		if(_parent.flag_stop) break;

		JudgeResult judge_res=_judger.judge();
		if(_parent.flag_stop) break;

		if(judge_res.type!=JudgeResult::Types::TERM)
		{
			auto res=check_judge_res(judge_res);

			std::lock_guard lock(_parent.mtx_q);
			if(judge_res.type!=JudgeResult::Types::OK)
				_parent.flag_stop=true;
			_parent.tot++;
			_parent.result_q.emplace(_id,std::move(res));
			_parent.cond_q.notify_one();
		}
	}
}
catch(const std::exception &e)
{
	std::lock_guard lock(_parent.mtx_q);
	_parent.result_q.emplace(_id,std::current_exception());
	_parent.cond_q.notify_one();
}

void Tester::JudgeThread::interrupt()
{ _judger.terminate(); }

void Tester::JudgeThread::wait()
{ if(_thread.joinable()) _thread.join(); }

const Judger &Tester::JudgeThread::judger()
{ return _judger; }

fs::path Tester::getExePath(const std::string &name,bool in_path)
{
	bp::environment::value val(fs::current_path().string());
	if(in_path)
		val.push_back(bp::environment::get("PATH"));
	std::unordered_map<bp::environment::key,bp::environment::value> new_env{
		{"PATH",val}
#ifdef _WIN32
		,{"PATHEXT",bp::environment::get("PATHEXT")}
#endif
	};
	return bp::environment::find_executable(name,new_env);
}

void Tester::compileOne(const std::string &filename,
						const std::vector<std::string> &extra_opt,
						std::mutex &mtx,
						std::ofstream &fout,
						std::exception_ptr &ep) try
{
	std::vector<std::string> compiler_opt{filename+".cpp","-o",filename};
	compiler_opt.insert(compiler_opt.end(),extra_opt.begin(),extra_opt.end());

	as::io_context ctx;
	as::readable_pipe pipe(ctx);
	bp::process proc(
		ctx,
		bp::environment::find_executable("g++"),
		compiler_opt,
		bp::process_stdio{nullptr,nullptr,pipe}
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

void Tester::compileExecutables()
{
	std::vector<std::jthread> threads;
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

void Tester::handleResult(std::size_t idx,std::size_t id,const std::exception_ptr &ep)
{
	msg.fatal("Exception caught in thread #{}",id);
	std::rethrow_exception(ep);
}

void Tester::handleResult(std::size_t idx,std::size_t id,const ACResult &res)
{
	msg.print("Testcase #{}: ",idx);
	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=10},"Accepted\n");
}

void Tester::handleResult(std::size_t idx,std::size_t id,const WAResult &res)
{
	auto hint_msg=std::format("Failed on testcase #{}: {}",idx,res.msg);
	if(res.msg.size()>256)
		hint_msg+=std::format("(256 bytes only\nSee {}.log for detail.",opt.file);
	
	msg.print("Testcase #{}: ",idx);
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
#else
	msg.print("{}\n",hint_msg);
#endif
}

void Tester::handleResult(std::size_t idx,std::size_t id,const TLEResult &res)
{
	msg.print("Testcase #{}: ",idx);
	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"{} Time Limit Exceeded",res.name);
	if(res.time_used==static_cast<std::uint64_t>(-1))
		msg.print(" (killed)\n");
	else
		msg.print(" ({}ms/{}ms)\n",res.time_used,res.time_limit);
}

void Tester::handleResult(std::size_t idx,std::size_t id,const MLEResult &res)
{
	msg.print("Testcase #{}: ",idx);
	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=11},"{} Memory Limit Exceeded",res.name);
	msg.print(" ({:.2}MB/{}MB)\n",res.memory_used/1024.0/1024.0,res.memory_limit);
}

void Tester::handleResult(std::size_t idx,std::size_t id,const REResult &res)
{
	msg.print("Testcase #{}: ",idx);
	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=9},"{} Runtime Error",res.name);
	msg.print(" ({})\n",res.exit_code);
}

void Tester::handleResult(std::size_t idx,std::size_t id,const UKEResult &res)
{
	msg.print("Testcase #{}: ",idx);
	msg.print(TextAttr::FOREGROUND,TextAttr{.foreground=5},"{} Unknown Error\n",res.name);
}

void Tester::moveFiles(std::size_t id)
{
	auto tryMove=[&](const fs::path &from,const fs::path &to) {
		try
		{
			if(fs::exists(to) && fs::is_regular_file(to))
				fs::remove(to);
			fs::rename(from,to);
		}
		catch(const fs::filesystem_error &e)
		{
			auto ec=e.code();
			msg.error("failed to get {} ({}): {}",to.filename().string(),ec.value(),ec.message());
		}
	};

	auto &judger=threads[id]->judger();
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
	for(std::size_t i=0;i<opt.thread_cnt;i++)
		threads.push_back(std::make_unique<JudgeThread>(*this,i));

	static std::function<void()> tryQuit{};
	tryQuit=[&] {
		if(!flag_pause)
		{
			flag_pause.store(true);
			for(auto &t: threads)
				t->interrupt();
			cond_q.notify_one();
		}
		std::signal(SIGINT,[](int){ tryQuit(); });
	};
	std::signal(SIGINT,[](int){ tryQuit(); });

	for(std::size_t idx=1;!flag_stop && idx<=opt.test_cnt;idx++)
	{
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_q.wait(lock,[this] {
			return !result_q.empty() || flag_pause;
		});
		if(flag_pause)
		{
			std::string res;
			msg.print("quit testing? (y/n): ");
			std::getline(std::cin,res);
			std::cin.clear();

			if(res=="y" || res=="Y")
			{
				flag_stop=true;
				flag_pause=false;
				flag_pause.notify_all();
				break;
			}
			flag_pause=false;
			flag_pause.notify_all();
			continue;
		}

		auto [id,result]=std::move(result_q.front());
		result_q.pop();
		lock.unlock();

		std::visit([&](auto &&res) {
			using T=std::remove_cvref_t<decltype(res)>;
			handleResult(idx,id,std::forward<T>(res));
			if constexpr(!std::is_same_v<T,ACResult>)
			{
				threads[id]->wait();
				moveFiles(id);
			}
		},result);
	}
}
