#pragma once

#include "lib.hpp"
#include <boost/process.hpp>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#endif

struct RunnerResult
{
	enum Types{ UKE=0,OK,TLE,MLE,RE,KILLED };
	Types type;
	std::string name;
	unsigned exit_code;
	std::size_t time_used,memory_used;
	RunnerResult();
	RunnerResult(Types _tp,const std::string &_n,unsigned _e,std::size_t _t,std::size_t _m);
};

class Runner
{
 private:
	std::string name;
	boost::filesystem::path exe;
	std::size_t tl,ml;
	std::string fn_in,fn_out,fn_err;
	bp::child proc;
	bool is_terminated;

 public:
	Runner(const std::string &_name,const std::string &_exe,std::size_t _tl,std::size_t _ml,const std::string &_inf,const std::string &_ouf,const std::string &erf);
	void terminate();
	RunnerResult run(const std::vector<std::string> &args);
};