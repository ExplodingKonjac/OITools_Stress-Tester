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
	fin("__nul__"),fout("__nul__"),ferr("__nul__"),
	proc(),watcher(),res()
{}
Runner::~Runner() { terminate(); }

void Runner::setInputFile(const std::string &file) { fin=file; }

void Runner::setOutputFile(const std::string &file) { fout=file; }

void Runner::setErrorFile(const std::string &file) { ferr=file; }

const std::string &Runner::getName() { return name; }

const RunnerResult &Runner::getLastResult() { return res; }

bool Runner::running() { return proc.valid() && proc.running(); }

void Runner::start(const std::string &args)
{
	if(running())
		throw std::runtime_error("start process while running.");
#define FW_ARGS std::forward<decltype(args)>(args)...
	auto createChild=[&](auto &&...args)
	{
		proc=bp::child(FW_ARGS);
	};
	auto redirectInput=[&](auto &f,auto &&...args)
	{
		if(fin=="__std__") f(FW_ARGS,bp::std_in<stdin);
		else if(fin=="__nul__") f(FW_ARGS,bp::std_in<bp::null);
		else f(FW_ARGS,bp::std_in<fin);
	};
	auto redirectOutput=[&](auto &f,auto &&...args)
	{
		if(fout=="__std__") f(FW_ARGS,bp::std_out>stdout);
		else if(fout=="__nul__") f(FW_ARGS,bp::std_out>bp::null);
		else f(FW_ARGS,bp::std_out>fout);
	};
	auto redirectError=[&](auto &f,auto &&...args)
	{
		if(ferr=="__std__") f(FW_ARGS,bp::std_err>stderr);
		else if(ferr=="__nul__") f(FW_ARGS,bp::std_err>bp::null);
		else f(FW_ARGS,bp::std_out>ferr);
	};
#undef FW_ARGS
	res.type=static_cast<RunnerResult::Types>(0);
	std::invoke(redirectInput,
				redirectOutput,
				redirectError,
				createChild,
				app+" "+args);
	watcher=std::thread(watching,this);
}

void Runner::terminate()
{
	res.type=RunnerResult::KILLED;
	if(running() && proc.valid())
		proc.terminate();
	if(watcher.joinable()) watcher.join();
}

const RunnerResult &Runner::wait()
{
	if(watcher.joinable())
		watcher.join();
	return res;
}

void Runner::watching()
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
#elif defined(__unix__)
	
#endif
}