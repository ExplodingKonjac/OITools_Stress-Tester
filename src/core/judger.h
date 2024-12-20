#pragma once

#include "defines.hpp"
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

namespace bp=boost::process::v2;
namespace fs=boost::filesystem;

#if defined(_WIN32)
extern const fs::path null_path;
#elif defined(__linux__)
extern const fs::path null_path;
#endif

struct ProcessInfo
{
	enum Types{ UKE=-1,OK,TLE,MLE,RE,TERM };
	Types type=UKE;
	unsigned exit_code=-1;
	std::size_t time_used=-1,memory_used=-1;
};

struct JudgeResult
{
	enum Types{ OK=0,WA,GEN_ERR,STD_ERR,EXE_ERR,CHK_ERR,TERM };
	Types type=OK;
	ProcessInfo info;
};

class Judger
{
 private:
	std::string id;
	bool stopped;
	fs::path prefix,exe_path,std_path,gen_path,chk_path,input_path,output_path,answer_path,log_path;

	ProcessInfo runProgram(const fs::path &target,const std::vector<std::string> &args,std::size_t time_limit,std::size_t memory_limit,const fs::path &prefix,const fs::path &inf,const fs::path &ouf,const fs::path &erf);
	ProcessInfo watchProcess(bp::process &proc,std::size_t time_limit,std::size_t memory_limit);

 public:
	Judger::Judger(const std::string &_id,const fs::path &_prefix,const fs::path &_exe,const fs::path &_std,const fs::path &_gen,const fs::path &_chk);
	JudgeResult judge();
	void terminate();
	fs::path getInputPath()const;
	fs::path getOutputPath()const;
	fs::path getAnswerPath()const;
	fs::path getLogPath()const;
};