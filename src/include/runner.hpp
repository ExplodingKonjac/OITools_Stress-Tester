#pragma once
#include "lib.hpp"

struct RunnerResult
{
	enum Types{ OK,TLE,MLE,RE,KILLED };
	Types type;
	DWORD exit_code,time_used,memory_used;
	RunnerResult():
		type(OK),exit_code(-1),time_used(-1),memory_used(-1)
	{}
	RunnerResult(Types _tp,DWORD _e,DWORD _t,DWORD _m):
		type(_tp),exit_code(_e),time_used(_t),memory_used(_m)
	{}
};

class Runner
{
 protected:
	std::string name,app;
	SIZE_T tl,ml;
	HANDLE hin,hout,herr;
	PROCESS_INFORMATION pi;
	std::thread running_thread;
	bool running,is_killed;
	RunnerResult res;

	void waiting()
	{
		running=true,is_killed=false;

		auto getMemory=[&]
		{
			PROCESS_MEMORY_COUNTERS mem_info;
			initmem(mem_info);
			GetProcessMemoryInfo(pi.hProcess,&mem_info,sizeof(mem_info));
			return std::max(mem_info.PeakWorkingSetSize,
							mem_info.PeakPagefileUsage);
		};
		std::thread mem_watcher([&]
		{
			while(true)
			{
				auto mem=getMemory();
				if(mem>ml) { TerminateProcess(pi.hProcess,0);break; }
				pthread_testcancel();
				Sleep(10);
			}
		});
		ResumeThread(pi.hThread);
		auto bg=std::chrono::steady_clock::now();
		if(WaitForSingleObject(pi.hProcess,tl+200)!=WAIT_OBJECT_0)
			TerminateProcess(pi.hProcess,0);
		auto ed=std::chrono::steady_clock::now();

		pthread_cancel(mem_watcher.native_handle());
		mem_watcher.join();
		res.time_used=std::chrono::duration_cast<std::chrono::milliseconds>(ed-bg).count();
		res.memory_used=getMemory();
		GetExitCodeProcess(pi.hProcess,&res.exit_code);
		if(is_killed) res.type=RunnerResult::KILLED;
		else if(res.time_used>tl) res.type=RunnerResult::TLE;
		else if(res.memory_used>ml) res.type=RunnerResult::MLE;
		else if(res.exit_code!=0) res.type=RunnerResult::RE;
		else res.type=RunnerResult::OK;

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		running=false;
	}

 public:
	Runner(const std::string &_name,const std::string &_app,SIZE_T _tl,SIZE_T _ml):
		name(_name),app(_app),tl(_tl),ml(_ml),
		hin(nullptr),hout(nullptr),herr(nullptr),
		running_thread(),running(false),res()
	{}
	~Runner()
	{
		if(running) TerminateProcess(pi.hProcess,-1);
		if(running_thread.joinable()) running_thread.join();
		CloseHandle(hin);
		CloseHandle(hout);
		CloseHandle(herr);
	}

	void setInputFile(const std::string &s)
	{
		if(hin) CloseHandle(hin);
		if(s=="__std__") hin=hStdin();
		else if(s=="__nul__") hin=nullptr;
		else hin=openFile(s,'r');
	}

	void setOutputFile(const std::string &s)
	{
		if(hin) CloseHandle(hout);
		if(s=="__std__") hout=hStdout();
		else if(s=="__nul__") hout=nullptr;
		else hout=openFile(s,'w');
	}

	void setErrorFile(const std::string &s)
	{
		if(herr) CloseHandle(herr);
		if(s=="__std__") herr=hStderr();
		else if(s=="__nul__") herr=nullptr;
		else herr=openFile(s,'w');
	}

	const std::string &getName()
	{ return name; }

	void start(const std::string &args="")
	{
		if(running)
			throw std::runtime_error("start process while running.");
		if(running_thread.joinable())
			running_thread.join();

		STARTUPINFO si;
		initmem(si),initmem(pi);
		si.dwFlags|=STARTF_USESTDHANDLES;
		resetFile(hin),si.hStdInput=hin;
		resetFile(hout,true),si.hStdOutput=hout;
		resetFile(herr,true),si.hStdError=herr;
		if(!CreateProcess(nullptr,(app+' '+args).data(),nullptr,nullptr,true,CREATE_SUSPENDED,nullptr,nullptr,&si,&pi))
			quitError("Failed to create process of %s (%u).",app.c_str(),GetLastError());

		running_thread=std::thread(waiting,this);
	}

	const RunnerResult &wait()
	{
		if(running_thread.joinable())
			running_thread.join();
		return res;
	}

	void terminate()
	{
		if(!running) return;
		is_killed=true;
		TerminateProcess(pi.hProcess,-1);
	}

	const RunnerResult &getLastResult()
	{ return res; }
};
