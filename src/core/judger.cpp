#include "judger.h"

#include <boost/scope/scope_exit.hpp>

#include <syncstream>
#include <format>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__linux__)
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <poll.h>
#endif

namespace bp=boost::process::v2;
namespace fs=boost::filesystem;
namespace as=boost::asio;

Judger::Judger(const std::string &_id,
			   const fs::path &_prefix,
			   const fs::path &_exe,
			   const fs::path &_std,
			   const fs::path &_gen,
			   const fs::path &_chk):
	prefix(fs::absolute(_prefix)),
	exe_path(_exe),
	std_path(_std),
	gen_path(_gen),
	chk_path(_chk),
	input_path(prefix/(_id+".in")),
	output_path(prefix/(_id+".out")),
	answer_path(prefix/(_id+".ans")),
	log_path(prefix/(_id+".log")),
	flag_stop{},
	cur_proc(nullptr),
	mtx_cur_proc{}
{}

ProcessInfo Judger::watchProcess(std::size_t time_limit,
								 std::size_t memory_limit)
{
	ProcessInfo res;

#if defined(_WIN32)
	HANDLE handle=cur_proc->native_handle();
	auto getTimeUsage=[&]()->std::size_t {
		FILETIME creation_time,exit_time,kernel_time,user_time;
		GetProcessTimes(
			handle,
			&creation_time,
			&exit_time,
			&kernel_time,
			&user_time
		);
		return (std::uint64_t(user_time.dwHighDateTime)<<32|user_time.dwLowDateTime)/10'000;
	};
	auto getMemoryUsage=[&]()->std::size_t {
		PROCESS_MEMORY_COUNTERS_EX mem_info{sizeof(mem_info)};
		GetProcessMemoryInfo(
			handle,
			reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&mem_info),
			sizeof(mem_info)
		);
		return std::max(mem_info.PeakWorkingSetSize,mem_info.PrivateUsage);
	};

	auto lim=std::chrono::steady_clock::now()+std::chrono::milliseconds(time_limit+200);
	DWORD wait_res=1;
	while(std::chrono::steady_clock::now()<=lim &&
		  (wait_res=WaitForSingleObject(handle,15))!=WAIT_OBJECT_0)
	{
		std::size_t mem=getMemoryUsage();
		if(mem>memory_limit)
		{
			cur_proc->terminate();
			res.memory_used=mem;
			res.type=ProcessInfo::Types::MLE;
			return res;
		}
	}
	if(flag_stop)
	{
		res.type=ProcessInfo::Types::TERM;
		return res;
	}
	if(wait_res!=WAIT_OBJECT_0)
	{
		cur_proc->terminate();
		res.type=ProcessInfo::Types::TLE;
		return res;
	}

	res.exit_code=cur_proc->wait();
	res.time_used=getTimeUsage();
	res.memory_used=getMemoryUsage();

#elif defined(__linux__)
	rlimit mem_lim{memory_limit+1024,memory_limit+1024};
	prlimit(pid,RLIMIT_AS,&mem_lim,nullptr);
	prlimit(pid,RLIMIT_STACK,&mem_lim,nullptr);

	int proc_fd=syscall(SYS_pidfd_open,cur_proc->id(),0);
	pollfd poll_fd{proc_fd,POLLHUP|POLLIN};

	int poll_res=poll(&proc_fd,1,time_limit+100);
	if(cur_proc->running())
	{
		cur_proc->terminate();
		res.type=ProcessInfo::TLE;
		return res;
	}
	if(flag_stop)
	{
		res.type=ProcessInfo::TERM;
		return res;
	}
	rusage usage{};
	int status;
	wait4(pid,&status,0,&usage);

	res.exit_code=WIFEXITED(status)?WEXITSTATUS(status):
				  WIFSIGNALED(status)?WTERMSIG(status):
				  status;
	res.time_used=usage.ru_utime.tv_sec+usage.ru_utime.tv_usec/1000;
	res.memory_used=usage.ru_maxrss*1024;
#endif

	if(res.time_used>time_limit)
		res.type=ProcessInfo::Types::TLE;
	else if(res.memory_used>memory_limit)
		res.type=ProcessInfo::Types::MLE;
	else if(res.exit_code!=0)
		res.type=ProcessInfo::Types::RE;
	else
		res.type=ProcessInfo::Types::OK;
	return res;
}

ProcessInfo Judger::runProgram(const fs::path &target,
							   const std::vector<std::string> &args,
							   std::size_t time_limit,
							   std::size_t memory_limit,
							   const fs::path &inf,
							   const fs::path &ouf,
							   const fs::path &erf)
{
	auto redirect=[](auto &target,auto &p) {
		if(p.empty()) target=nullptr;
		else target=p;
	};
	bp::process_stdio io{};
	redirect(io.in,inf);
	redirect(io.out,ouf);
	redirect(io.err,erf);
	as::io_context ctx;
	bp::process proc{ctx,target,args,io,bp::process_start_dir{prefix}};

	std::osyncstream(std::cout)<<std::format("target: {}, pid: {}\n",target.string(),proc.id());

	{
		std::lock_guard lock(mtx_cur_proc);
		cur_proc=&proc;
	}
	auto clean_guard=boost::scope::scope_exit([&] {
		std::lock_guard lock(mtx_cur_proc);
		cur_proc=nullptr;
	});
	return watchProcess(time_limit,memory_limit);
}

JudgeResult Judger::judge()
{
	boost::scope::scope_exit clean_guard([this] {
		for(auto &p: {input_path,output_path,answer_path,log_path})
			fs::remove(p);
	});
	flag_stop.store(false);

	JudgeResult res{JudgeResult::Types::OK};

	res.gen_info=runProgram(
		gen_path,
		opt.opt_gen,
		opt.tl_gen,opt.ml_gen*1024*1024,
		{},input_path,{}
	);
	if(flag_stop)
		goto ret_term;
	else if(res.gen_info.type!=ProcessInfo::Types::OK)
		return res.type=JudgeResult::Types::GEN_ERR,res;

	res.exe_info=runProgram(
		exe_path,
		opt.opt_exe,
		opt.tl,opt.ml*1024*1024,
		input_path,output_path,{}
	);
	if(flag_stop)
		goto ret_term;
	else if(res.exe_info.type!=ProcessInfo::Types::OK)
		return res.type=JudgeResult::Types::EXE_ERR,res;

	res.std_info=runProgram(
		std_path,
		opt.opt_exe,
		opt.tl,opt.ml*1024*1024,
		input_path,answer_path,{}
	);
	if(flag_stop)
		goto ret_term;
	else if(res.std_info.type!=ProcessInfo::Types::OK)
		return res.type=JudgeResult::Types::STD_ERR,res;

	res.chk_info=runProgram(
		chk_path,
		std::vector{input_path.string(),output_path.string(),answer_path.string()}+opt.opt_chk,
		opt.tl_chk,opt.ml_chk*1024*1024,
		{},{},log_path
	);
	if(flag_stop)
		goto ret_term;
	else if(res.chk_info.type==ProcessInfo::Types::RE)
		return res.type=JudgeResult::Types::WA,res;
	else if(res.chk_info.type!=ProcessInfo::Types::OK)
		return res.type=JudgeResult::Types::CHK_ERR,res;

	return res;
	ret_term: return {JudgeResult::Types::TERM};
}

void Judger::terminate()
{
	std::lock_guard lock(mtx_cur_proc);
	flag_stop.store(true);
	if(cur_proc && cur_proc->running())
		cur_proc->terminate();
}

fs::path Judger::getInputPath()const
{ return input_path; }

fs::path Judger::getOutputPath()const
{ return output_path; }

fs::path Judger::getAnswerPath()const
{ return answer_path; }

fs::path Judger::getLogPath()const
{ return log_path; }
