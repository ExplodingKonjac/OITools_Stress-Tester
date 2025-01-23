#include "judger.h"

namespace bp=boost::process::v2;
namespace fs=boost::filesystem;
namespace as=boost::asio;

#if defined(_WIN32)
const fs::path null_path("NUL");
#elif defined(__linux__)
const fs::path null_path("/dev/null");
#endif

Judger::Judger(const std::string &_id,
			   const fs::path &_prefix,
			   const fs::path &_exe,
			   const fs::path &_std,
			   const fs::path &_gen,
			   const fs::path &_chk):
	id(_id),
	prefix(_prefix),
	exe_path(_exe),
	std_path(_std),
	gen_path(_gen),
	chk_path(_chk),
	input_path(_prefix/fs::path(_id+".in")),
	output_path(_prefix/fs::path(_id+".out")),
	answer_path(_prefix/fs::path(_id+".ans")),
	log_path(_prefix/fs::path(_id+".log")),
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
		GetProcessTimes(handle,&creation_time,&exit_time,&kernel_time,&user_time);
		return (std::size_t(user_time.dwHighDateTime)<<32|user_time.dwLowDateTime)/10000;
	};
	auto getMemoryUsage=[&]()->std::size_t {
		PROCESS_MEMORY_COUNTERS_EX mem_info{sizeof(mem_info)};
		GetProcessMemoryInfo(handle,(PROCESS_MEMORY_COUNTERS*)&mem_info,sizeof(mem_info));
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
			res.type=ProcessInfo::MLE;
			return res;
		}
	}
	if(flag_stop)
	{
		res.type=ProcessInfo::TERM;
		return res;
	}
	if(wait_res!=WAIT_OBJECT_0)
	{
		cur_proc->terminate();
		res.type=ProcessInfo::TLE;
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
		res.type=ProcessInfo::TLE;
	else if(res.memory_used>memory_limit)
		res.type=ProcessInfo::MLE;
	else if(res.exit_code!=0)
		res.type=ProcessInfo::RE;
	else
		res.type=ProcessInfo::OK;
	return res;
}

ProcessInfo Judger::runProgram(const fs::path &target,
							   const std::vector<std::string> &args,
							   std::size_t time_limit,
							   std::size_t memory_limit,
							   const fs::path &prefix,
							   const fs::path &inf,
							   const fs::path &ouf,
							   const fs::path &erf)
{
	std::unique_lock<std::mutex> lock(mtx_cur_proc);
	as::io_context ctx;
	cur_proc=std::make_unique<bp::process>(
		ctx,
		target,args,
		bp::process_stdio{inf,ouf,erf},
		bp::process_start_dir{prefix}
	);
	lock.unlock();
	ProcessInfo res=watchProcess(time_limit,memory_limit);
	lock.lock();
	cur_proc.reset(nullptr);
	return res;
}

JudgeResult Judger::judge()
{
	boost::scope::make_scope_exit([this] {
		for(auto &p: {input_path,output_path,answer_path,log_path})
			fs::remove(p);
	});
	flag_stop.store(false);

	ProcessInfo gen_info=runProgram(
		gen_path,
		opt.opt_gen,
		opt.tl_gen,opt.ml_gen*1024*1024,
		prefix,null_path,input_path,null_path
	);
	if(flag_stop)
		return JudgeResult{JudgeResult::TERM};
	else if(gen_info.type!=ProcessInfo::OK)
		return JudgeResult{JudgeResult::GEN_ERR,gen_info};

	ProcessInfo exe_info=runProgram(
		exe_path,
		opt.opt_exe,
		opt.tl,opt.ml*1024*1024,
		prefix,input_path,output_path,null_path
	);
	if(flag_stop)
		return JudgeResult{JudgeResult::TERM};
	else if(exe_info.type!=ProcessInfo::OK)
		return JudgeResult{JudgeResult::EXE_ERR,exe_info};

	ProcessInfo std_info=runProgram(
		std_path,
		opt.opt_exe,
		opt.tl,opt.ml*1024*1024,
		prefix,input_path,answer_path,null_path
	);
	if(flag_stop)
		return JudgeResult{JudgeResult::TERM};
	else if(std_info.type!=ProcessInfo::OK)
		return JudgeResult{JudgeResult::STD_ERR,std_info};

	ProcessInfo chk_info=runProgram(
		chk_path,
		std::vector{input_path.filename().string(),output_path.filename().string(),answer_path.filename().string()}+opt.opt_chk,
		opt.tl_chk,opt.ml_chk*1024*1024,
		prefix,null_path,null_path,log_path
	);
	if(flag_stop)
		return JudgeResult{JudgeResult::TERM};
	else if(chk_info.type==ProcessInfo::RE)
		return JudgeResult{JudgeResult::WA,exe_info};
	else if(chk_info.type!=ProcessInfo::OK)
		return JudgeResult{JudgeResult::CHK_ERR,chk_info};

	return JudgeResult{JudgeResult::OK,exe_info};
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
