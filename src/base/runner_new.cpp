#include "runner_new.h"

namespace bp=boost::process;
namespace fs=boost::filesystem;

RunnerResult::RunnerResult():
	type(UKE),exit_code(-1),time_used(-1),memory_used(-1)
{}

RunnerResult::RunnerResult(Types _tp,const std::string &_n,unsigned _e,std::size_t _t,std::size_t _m):
	type(_tp),name(_n),exit_code(_e),time_used(_t),memory_used(_m)
{}

static fs::path getExePath(const std::string &exe)
{
	auto paths=boost::this_process::path();
	paths.emplace_back(".");
	return bp::search_path(exe,paths);
}

Runner::Runner(const std::string &_name,const std::string &_exe,std::size_t _tl,std::size_t _ml,const std::string &_inf,const std::string &_ouf,const std::string &erf):
	name(_name),exe(_exe),tl(_tl),ml(_ml),fn_in(_inf),fn_out(_ouf),fn_err(erf),proc(),is_terminated(false)
{}

void Runner::terminate()
{
	if(proc.valid() && proc.running())
		proc.terminate();
	is_terminated=true;
}

RunnerResult Runner::run(const std::vector<std::string> &args)
{
#define FW_ARGS std::forward<decltype(args)>(args)...
	auto createChild=[&](auto &&...args)
	{
		proc=bp::child(FW_ARGS);
	};
	auto redirectInput=[&](auto &&f,auto &&...args)
	{
		if(fn_in=="__std__") f(FW_ARGS);
		else if(fn_in=="__nul__") f(FW_ARGS,bp::std_in<bp::null);
		else f(FW_ARGS,bp::std_in<fn_in);
	};
	auto redirectOutput=[&](auto &&f,auto &&...args)
	{
		if(fn_out=="__std__") f(FW_ARGS);
		else if(fn_out=="__nul__") f(FW_ARGS,bp::std_out>bp::null);
		else f(FW_ARGS,bp::std_out>fn_out);
	};
	auto redirectError=[&](auto &&f,auto &&...args)
	{
		if(fn_err=="__std__") f(FW_ARGS);
		else if(fn_err=="__nul__") f(FW_ARGS,bp::std_err>bp::null);
		else f(FW_ARGS,bp::std_err>fn_err);
	};
	std::invoke(redirectInput,redirectOutput,redirectError,createChild,bp::exe=exe,bp::args=args);
#undef FW_ARGS
	bp::pid_t pid=proc.id();
	RunnerResult res;
#if defined(__linux__) // linux complement
	rlimit tim_lim{tl/1000+1,tl/1000+1},mem_lim{ml,ml};
	int status=0;
	rusage usage{};
	prlimit(pid,RLIMIT_CPU,&tim_lim,nullptr);
	prlimit(pid,RLIMIT_AS,&mem_lim,nullptr);
	prlimit(pid,RLIMIT_STACK,&mem_lim,nullptr);
	wait4(pid,&status,0,&usage);
	if(is_terminated)
		res.type=RunnerResult::KILLED;
	else if(WIFSIGNALED(status) && WTERMSIG(status)==SIGXCPU)
	{
		res.type=RunnerResult::TLE;
		res.time_used=-1;
	}
	else
	{
		res.time_used=usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1000;
		res.memory_used=usage.ru_maxrss*1024;
		res.exit_code=status;
		if(res.time_used>tl)
			res.type=RunnerResult::TLE;
		else if(res.memory_used>ml)
			res.type=RunnerResult::MLE;
		else if(status!=0)
		{
			res.type=RunnerResult::RE;
			if(WIFEXITED(status))
				res.exit_code=WEXITSTATUS(status);
			else if(WIFSIGNALED(status))
				res.exit_code=WTERMSIG(status);
			else if(WIFSTOPPED(status))
				res.exit_code=WSTOPSIG(status);
		}
	}
#elif defined(_WIN32) // windows complement
	
#endif
}