#include "runner.h"

namespace bp=boost::process;


RunnerResult::RunnerResult():
	type(OK),exit_code(-1),time_used(-1),memory_used(-1)
{}
RunnerResult::RunnerResult(Types _tp,unsigned _e,std::size_t _t,std::size_t _m):
	type(_tp),exit_code(_e),time_used(_t),memory_used(_m)
{}

Runner::Runner(const std::string &_name,const std::string &_app,std::size_t _tl,std::size_t _ml):
	name(_name),app(_app),tl(_tl),ml(_ml),
	fn_in("__nul__"),fn_out("__nul__"),fn_err("__nul__"),
	proc(),watcher(),res()
{}
Runner::~Runner() { terminate(); }

void Runner::setInputFile(const std::string &file) { fn_in=file; }

void Runner::setOutputFile(const std::string &file) { fn_out=file; }

void Runner::setErrorFile(const std::string &file) { fn_err=file; }

const std::string &Runner::getName() { return name; }

const RunnerResult &Runner::getLastResult() { return res; }

std::size_t Runner::getTimeLimit() { return tl; }

std::size_t Runner::getMemoryLimit() { return ml; }

bool Runner::running() { return proc.valid() && proc.running(); }

void Runner::start(const std::string &args)
{
	if(running())
		throw std::runtime_error("start process while running.");
	FILE *fp_in=(fn_in=="__std__"?stdin:fn_in=="__nul__"?nullptr:std::fopen(fn_in.c_str(),"r")),
		 *fp_out=(fn_out=="__std__"?stdout:fn_out=="__nul__"?nullptr:std::fopen(fn_out.c_str(),"w")),
		 *fp_err=(fn_err=="__std__"?stderr:fn_err=="__nul__"?nullptr:std::fopen(fn_err.c_str(),"w"));
	proc=bp::child(app+" "+args,bp::std_in<fp_in,bp::std_out>fp_out,bp::std_err>fp_err);
	watcher=std::thread(watching,this,fp_in,fp_out,fp_err);
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
	res=RunnerResult{static_cast<RunnerResult::Types>(0),0,0,0};
	auto handle=proc.native_handle();
#if defined(_WIN32)
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
			goto end_watch;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	if(res.type==RunnerResult::KILLED)
		goto end_watch;
	if(proc.running())
	{
		proc.terminate();
		res.type=RunnerResult::TLE;
		res.time_used=tl+200;
		goto end_watch;
	}
	res.time_used=getTime();
	if(res.time_used>tl)
	{
		res.type=RunnerResult::TLE;
		goto end_watch;
	}
	res.memory_used=getMemory();
	if(res.memory_used>ml)
	{
		res.type=RunnerResult::MLE;
		goto end_watch;
	}
	proc.wait();
	res.exit_code=proc.exit_code();
	if(res.exit_code!=0)
	{
		res.type=RunnerResult::RE;
		goto end_watch;
	}
#elif defined(__linux__)
	
#endif
	end_watch:;
	if(fp_in && fp_in!=stdin) fclose(fp_in);
	if(fp_out && fp_out!=stdout) fclose(fp_out);
	if(fp_err && fp_err!=stderr) fclose(fp_err);
}