#include "runner.h"

namespace bp=boost::process;
namespace fs=boost::filesystem;

RunnerResult::RunnerResult():
	type(OK),exit_code(-1),time_used(-1),memory_used(-1)
{}
RunnerResult::RunnerResult(Types _tp,unsigned _e,std::size_t _t,std::size_t _m):
	type(_tp),exit_code(_e),time_used(_t),memory_used(_m)
{}

Runner::Runner(const std::string &_name,const std::string &_app,std::size_t _tl,std::size_t _ml,bool _env):
	name(_name),app(bp::search_path(_app,getSearchPaths(_env))),tl(_tl),ml(_ml),
	fn_in("__nul__"),fn_out("__nul__"),fn_err("__nul__"),
	proc(),watcher(),res()
{
	if(app.empty())
		quitError("Cannot find executable '%s'.",_app.c_str());
}
Runner::~Runner() { terminate(); }

void Runner::setInputFile(const std::string &file)
{ fn_in=file; }

void Runner::setOutputFile(const std::string &file)
{ fn_out=file; }

void Runner::setErrorFile(const std::string &file)
{ fn_err=file; }

void Runner::setName(const std::string &_name)
{ name=_name; }

void Runner::setTimeLimit(std::size_t _tl)
{ tl=_tl; }

void Runner::setMemoryLimit(std::size_t _ml)
{ ml=_ml; }

const std::string &Runner::getName()const
{ return name; }

const std::string &Runner::getInputFile()const
{ return fn_in; }

const std::string &Runner::getOutputFile()const
{ return fn_out; }

const std::string &Runner::getErrorFile()const
{ return fn_err; }

std::size_t Runner::getTimeLimit()const
{ return tl; }

std::size_t Runner::getMemoryLimit()const
{ return ml; }

const RunnerResult &Runner::getLastResult()const
{ return res; }

bool Runner::running()
{ return proc.valid() && proc.running(); }

std::vector<fs::path> Runner::getSearchPaths(bool env)
{
	std::vector<fs::path> res;
	if(env) res=boost::this_process::path();
	res.emplace_back(".");
	return res;
}

void Runner::start(const std::vector<std::string> &args)
{
	if(running())
		throw std::runtime_error("start process while running.");
	FILE *fp_in=nullptr,*fp_out=nullptr,*fp_err=nullptr;
#define FW_ARGS std::forward<decltype(args)>(args)...
	auto createChild=[&](auto &&...args)
	{
		proc=bp::child(FW_ARGS);
		watcher=std::thread(&Runner::watching,this,fp_in,fp_out,fp_err);
	};
	auto redirectInput=[&](auto &&f,auto &&...args)
	{
		if(fn_in=="__std__") fp_in=stdin,f(FW_ARGS);
		else if(fn_in=="__nul__") f(FW_ARGS,bp::std_in<bp::null);
		else fp_in=fopen(fn_in.c_str(),"r"),f(FW_ARGS,bp::std_in<fp_in);
	};
	auto redirectOutput=[&](auto &&f,auto &&...args)
	{
		if(fn_out=="__std__") fp_out=stdout,f(FW_ARGS);
		else if(fn_out=="__nul__") f(FW_ARGS,bp::std_out>bp::null);
		else fp_out=fopen(fn_out.c_str(),"w"),f(FW_ARGS,bp::std_out>fp_out);
	};
	auto redirectError=[&](auto &&f,auto &&...args)
	{
		if(fn_err=="__std__") fp_err=stderr,f(FW_ARGS);
		else if(fn_err=="__nul__") f(FW_ARGS,bp::std_err>bp::null);
		else fp_err=fopen(fn_err.c_str(),"w"),f(FW_ARGS,bp::std_err>fp_err);
	};
	std::invoke(redirectInput,
				redirectOutput,
				redirectError,
				createChild,bp::exe=app,bp::args=args);
#undef FW_ARGS
}

void Runner::terminate()
{
	res.type=RunnerResult::KILLED;
	if(running()) proc.terminate();
	if(watcher.joinable()) watcher.join();
}

const RunnerResult &Runner::wait()
{
	if(watcher.joinable())
		watcher.join();
	return res;
}

void Runner::watching(FILE *fp_in,FILE *fp_out,FILE *fp_err)
{
	struct ScopeGuard
	{
		std::function<void()> f;
		~ScopeGuard() { f(); }
	};
	ScopeGuard guard{[&] {
		if(fp_in && fp_in!=stdin) fclose(fp_in);
		if(fp_out && fp_out!=stdout) fclose(fp_out);
		if(fp_err && fp_err!=stderr) fclose(fp_err);
	}};
	res=RunnerResult{static_cast<RunnerResult::Types>(0),0,0,0};
#if defined(_WIN32)
	auto handle=proc.native_handle();
	auto getMemory=[&]
	{
		PROCESS_MEMORY_COUNTERS_EX mem_info{sizeof(mem_info)};
		GetProcessMemoryInfo(handle,(PROCESS_MEMORY_COUNTERS*)&mem_info,sizeof(mem_info));
		return std::max(mem_info.PeakWorkingSetSize,mem_info.PrivateUsage);
	};
	auto getTime=[&]
	{
		FILETIME creation_time,exit_time,kernel_time,user_time;
		GetProcessTimes(handle,&creation_time,&exit_time,&kernel_time,&user_time);
		std::size_t ms=0;
		ms=(std::size_t(user_time.dwHighDateTime)<<32|user_time.dwLowDateTime)/10000;
		return ms;
	};
	auto lim=std::chrono::steady_clock::now()+std::chrono::milliseconds(tl+200);
	while(std::chrono::steady_clock::now()<=lim)
	{
		if(!proc.running()) break;
		res.memory_used=getMemory();
		if(res.memory_used>ml)
		{
			proc.terminate();
			res.type=RunnerResult::MLE;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	if(res.type==RunnerResult::KILLED)
		return;
	if(proc.running())
	{
		proc.terminate();
		res.type=RunnerResult::TLE;
		res.time_used=-1;
		return;
	}
	proc.wait();
	res.time_used=getTime();
	if(res.time_used>tl)
	{
		res.type=RunnerResult::TLE;
		return;
	}
	res.memory_used=getMemory();
	if(res.memory_used>ml)
	{
		res.type=RunnerResult::MLE;
		return;
	}
	res.exit_code=proc.exit_code();
	if(res.exit_code!=0)
	{
		res.type=RunnerResult::RE;
		return;
	}
#elif defined(__linux__)
	auto pid=proc.id();
	rlimit tim_lim{tl/1000+1,tl/1000+1},mem_lim{ml,ml};
	prlimit(pid,RLIMIT_CPU,&tim_lim,nullptr);
	prlimit(pid,RLIMIT_AS,&mem_lim,nullptr);
	prlimit(pid,RLIMIT_STACK,&mem_lim,nullptr);
	int status=0;
	rusage usage{};
	wait4(pid,&status,0,&usage);
	if(res.type==RunnerResult::KILLED)
		return;
	if(WIFSIGNALED(status) && WTERMSIG(status)==SIGXCPU)
	{
		res.type=RunnerResult::TLE;
		res.time_used=-1;
		return;
	}
	res.time_used=usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1000;
	if(res.time_used>tl)
	{
		res.type=RunnerResult::TLE;
		return;
	}
	res.memory_used=usage.ru_maxrss*1024;
	if(res.memory_used>ml)
	{
		res.type=RunnerResult::MLE;
		return;
	}
	res.exit_code=status;
	if(res.exit_code!=0)
	{
		res.type=RunnerResult::RE;
		if(WIFEXITED(status))
			res.exit_code=WEXITSTATUS(status);
		else if(WIFSIGNALED(status))
			res.exit_code=WTERMSIG(status);
		else if(WIFSTOPPED(status))
			res.exit_code=WSTOPSIG(status);
		return;
	}
#endif
}