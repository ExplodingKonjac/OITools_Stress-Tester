#pragma once

#include "lib.hpp"
#include "options.h"

#include <boost/process.hpp>
#include <boost/filesystem.hpp>

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

namespace bp=boost::process;
namespace fs=boost::filesystem;

struct ProcessInfo
{
	enum Types{ UKE=-1,OK,TLE,MLE,RE,KILLED };
	Types type=UKE;
	unsigned exit_code=-1;
	std::size_t time_used=-1,memory_used=-1;
};

struct RunnerResult
{
	enum Types{ OK=0,WA,GEN_ERR,STD_ERR,EXE_ERR,CHK_ERR };
	Types type=OK;
	ProcessInfo info;
};

class Judger
{
 private:
	std::string id;
	bool stopped;
	fs::path prefix,exe,std,gen,chk;

	RunnerResult judge();
	ProcessInfo runProgram(const fs::path &target,const std::vector<std::string> &args,std::size_t time_limit,std::size_t memory_limit,const fs::path &inf,const fs::path &ouf,const fs::path &erf);
	ProcessInfo watchProcess(bp::child &proc,std::size_t time_limit,std::size_t memory_limit);

 public:
	Judger::Judger(const std::string &_id,const fs::path &_prefix,const fs::path &_exe,const fs::path &_std,const fs::path &_gen,const fs::path &_chk);
};