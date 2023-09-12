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

bool Runner::running() { return proc.valid() && proc.running(); }

void Runner::start(const std::string &args)
{
	if(running())
		throw std::runtime_error("start process while running.");
	FILE *fp_in=(fn_in=="__std__"?stdin:fn_in=="__nul__"?nullptr:std::fopen(fn_in.c_str(),"r")),
		 *fp_out=(fn_out=="__std__"?stdout:fn_out=="__nul__"?nullptr:std::fopen(fn_out.c_str(),"w")),
		 *fp_err=(fn_err=="__std__"?stderr:fn_err=="__nul__"?nullptr:std::fopen(fn_err.c_str(),"w"));
	res.type=static_cast<RunnerResult::Types>(0);
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
	[[maybe_unused]] auto pid=proc.id();
	auto handle=proc.native_handle();
#if defined(_WIN32)
	auto bg=std::chrono::steady_clock::now();
	auto getMemory=[&]
	{
		PROCESS_MEMORY_COUNTERS mem_info{0};
		mem_info.cb=sizeof(mem_info);
		GetProcessMemoryInfo(handle,&mem_info,sizeof(mem_info));
		return std::max(mem_info.PeakWorkingSetSize,
						mem_info.PeakPagefileUsage);
	};
	auto getTime=[&]
	{
		auto cur=std::chrono::steady_clock::now();
		auto dur=std::chrono::duration_cast<std::chrono::milliseconds>(cur-bg);
		return (size_t)dur.count();
	};
	while(proc.running() && getTime()<=tl+200)
	{
		if(getMemory()>ml)
			proc.terminate();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	res.exit_code=proc.exit_code();
	res.time_used=getTime();
	res.memory_used=getMemory();
	if(static_cast<int>(res.type)==0)
	{
		if(res.time_used>tl) res.type=RunnerResult::TLE;
		else if(res.memory_used>ml) res.type=RunnerResult::MLE;
		else if(res.exit_code!=0) res.type=RunnerResult::RE;
		else res.type=RunnerResult::OK;
	}
#elif defined(__linux__)
	
#endif
	if(fp_in && fp_in!=stdin) fclose(fp_in);
	if(fp_out && fp_out!=stdout) fclose(fp_out);
	if(fp_err && fp_err!=stderr) fclose(fp_err);
}