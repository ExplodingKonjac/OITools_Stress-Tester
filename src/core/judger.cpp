#include "judger.h"

namespace bp=boost::process;
namespace fs=boost::filesystem;

Judger::Judger(const std::string &_id,
			   const fs::path &_prefix,
			   const fs::path &_exe,
			   const fs::path &_std,
			   const fs::path &_gen,
			   const fs::path &_chk):
	id(_id),
	prefix(_prefix),
	exe(_exe),
	std(_std),
	gen(_gen),
	chk(_chk)
{}

RunnerResult Judger::judge()
{
	fs::path input_file=prefix/fs::path(id+".in"),
			 output_file=prefix/fs::path(id+".out"),
			 answer_file=prefix/fs::path(id+".ans"),
			 log_file=prefix/fs::path(id+".log");
	stopped=false;

	ProcessInfo gen_info=runProgram(
		gen,
		{},
		opt.tl_gen,
		opt.ml_gen,
		fs::path{},
		input_file,
		fs::path{}
	);
	if(gen_info.type!=ProcessInfo::OK)
		return RunnerResult{RunnerResult::GEN_ERR,gen_info};

	ProcessInfo exe_info=runProgram(
		exe,
		{},
		opt.tl,
		opt.ml,
		input_file,
		output_file,
		fs::path{}
	);
	if(exe_info.type!=ProcessInfo::OK)
		return RunnerResult{RunnerResult::EXE_ERR,exe_info};

	ProcessInfo std_info=runProgram(
		std,
		{},
		opt.tl,
		opt.ml,
		input_file,
		answer_file,
		fs::path{}
	);
	if(std_info.type!=ProcessInfo::OK)
		return RunnerResult{RunnerResult::STD_ERR,std_info};

	ProcessInfo chk_info=runProgram(
		chk,
		{input_file.string(),output_file.string(),answer_file.string()},
		opt.tl_chk,
		opt.ml_chk,
		fs::path{},
		fs::path{},
		log_file
	);
	if(chk_info.type==ProcessInfo::RE)
		return RunnerResult{RunnerResult::WA,std_info};
	else if(chk_info.type!=ProcessInfo::OK)
		return RunnerResult{RunnerResult::GEN_ERR,gen_info};
	
	return RunnerResult{RunnerResult::OK,std_info};
}

ProcessInfo Judger::watchProcess(bp::child &proc,
								 std::size_t time_limit,
								 std::size_t memory_limit)
{
	ProcessInfo res;

#if defined(_WIN32)
	HANDLE handle=proc.native_handle();

	auto getTimeUsage=[&]()->std::size_t
	{
		FILETIME creation_time,exit_time,kernel_time,user_time;
		GetProcessTimes(handle,&creation_time,&exit_time,&kernel_time,&user_time);
		return (std::size_t(user_time.dwHighDateTime)<<32|user_time.dwLowDateTime)/10000;
	};
	auto getMemoryUsage=[&]()->std::size_t
	{
		PROCESS_MEMORY_COUNTERS_EX mem_info{sizeof(mem_info)};
		GetProcessMemoryInfo(handle,(PROCESS_MEMORY_COUNTERS*)&mem_info,sizeof(mem_info));
		return std::max(mem_info.PeakWorkingSetSize,mem_info.PrivateUsage);
	};
	
	auto lim=std::chrono::steady_clock::now()+std::chrono::milliseconds(time_limit+200);
	while(std::chrono::steady_clock::now()<=lim && proc.running())
	{
		std::size_t mem=getMemoryUsage();
		if(mem>memory_limit)
		{
			proc.terminate();
			res.type=ProcessInfo::MLE;
			return res;
		}
		if(stopped)
		{
			proc.terminate();
			res.type=ProcessInfo::KILLED;
			return res;
		}
		Sleep(15);
	}
	if(proc.running())
	{
		proc.terminate();
		res.type=ProcessInfo::TLE;
		return res;
	}
	proc.wait();

	res.exit_code=proc.exit_code();
	res.time_used=getTimeUsage();
	res.memory_used=getMemoryUsage();

	if(res.exit_code!=0)
		res.type=ProcessInfo::RE;
	else if(res.time_used>time_limit)
		res.type=ProcessInfo::TLE;
	else if(res.memory_used>memory_limit)
		res.type=ProcessInfo::MLE;
	return res;

#elif defined(__linux__)
	auto pid=proc.pid();
	
	rlimit tim_lim{time_limit/1000+1,time_limit/1000+1},mem_lim{memory_limit,memory_limit};
	prlimit(pid,RLIMIT_CPU,&tim_lim,nullptr);
	prlimit(pid,RLIMIT_AS,&mem_lim,nullptr);
	prlimit(pid,RLIMIT_STACK,&mem_lim,nullptr);

	int status=0;
	rusage usage{};
	while(!wait4(pid,&status,0,&usage))
	{
		if(stopped)
		{
			kill(pid,SIGKILL);
			wait(NULL);
			res.type=ProcessInfo::KILLED;
			return res;
		}
		usleep(15000);
	}
	if(WIFSIGNALED(status) && WTERMSIG(status)==SIGXCPU)
	{
		res.type=RunnerResult::TLE;
		return res;
	}

	res.exit_code=status;
	res.time_used=usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1000;
	res.memory_used=usage.ru_maxrss*1024;

	if(res.time_used>time_limit)
		res.type=RunnerResult::TLE;
	else if(res.memory_used>ml)
		res.type=RunnerResult::MLE;
	else if(res.exit_code!=0)
	{
		res.type=RunnerResult::RE;
		if(WIFEXITED(status))
			res.exit_code=WEXITSTATUS(status);
		else if(WIFSIGNALED(status))
			res.exit_code=WTERMSIG(status);
		else if(WIFSTOPPED(status))
			res.exit_code=WSTOPSIG(status);
	}
	return res;
#endif
}

ProcessInfo Judger::runProgram(const fs::path &target,
							   const std::vector<std::string> &args,
							   std::size_t time_limit,
							   std::size_t memory_limit,
							   const fs::path &inf,
							   const fs::path &ouf,
							   const fs::path &erf)
{
	bp::child proc;

#define FW_ARGS std::forward<decltype(args)>(args)...
	auto createChild=[&proc](auto &&...args)
	{
		proc=bp::child(FW_ARGS);
	};
	auto redirectInput=[&inf](auto &&f,auto &&...args)
	{
		if(inf.empty())
			f(FW_ARGS,bp::std_in<bp::null);
		else f(FW_ARGS,bp::std_in<inf);
	};
	auto redirectOutput=[&ouf](auto &&f,auto &&...args)
	{
		if(ouf.empty())
			f(FW_ARGS,bp::std_out>bp::null);
		else f(FW_ARGS,bp::std_out>ouf);
	};
	auto redirectError=[&erf](auto &&f,auto &&...args)
	{
		if(erf.empty())
			f(FW_ARGS,bp::std_err>bp::null);
		else f(FW_ARGS,bp::std_err>erf);
	};
	std::invoke(
		redirectInput,
		redirectOutput,
		redirectError,
		createChild,
		bp::exe=target,bp::args=args
	);
#undef FW_ARGS
	
	ProcessInfo res=watchProcess(proc,time_limit,memory_limit);
	return res;
}