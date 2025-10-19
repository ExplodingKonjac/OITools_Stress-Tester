#pragma once

#include "base/config.hpp"
#include "base/common.hpp"
#include "options.h"

#include <boost/process.hpp>
#include <boost/filesystem.hpp>

#include <mutex>
#include <atomic>

struct ProcessInfo
{
	enum class Types{ UKE=-1,OK,TLE,MLE,RE,TERM };
	Types type=Types::UKE;
	int exit_code=-1;
	std::uint64_t time_used=-1,memory_used=-1;
};

struct JudgeResult
{
	enum class Types{ OK=0,WA,GEN_ERR,STD_ERR,EXE_ERR,CHK_ERR,TERM };
	Types type=Types::OK;
	ProcessInfo gen_info{};
	ProcessInfo exe_info{};
	ProcessInfo std_info{};
	ProcessInfo chk_info{};
};

class Judger
{
 private:
	ProcessInfo runProgram(const boost::filesystem::path &target,
						   const std::vector<std::string> &args,
						   std::size_t time_limit,
						   std::size_t memory_limit,
						   const boost::filesystem::path &inf,
						   const boost::filesystem::path &ouf,
						   const boost::filesystem::path &erf);
	ProcessInfo watchProcess(std::size_t time_limit,std::size_t memory_limit);

	boost::filesystem::path prefix,
							exe_path,
							std_path,
							gen_path,
							chk_path,
							input_path,
							output_path,
							answer_path,
							log_path;
	std::atomic<bool> flag_stop;
	boost::process::v2::process *cur_proc;
	std::mutex mtx_cur_proc;

 public:
	Judger(const std::string &_id,
		   const boost::filesystem::path &_prefix,
		   const boost::filesystem::path &_exe,
		   const boost::filesystem::path &_std,
		   const boost::filesystem::path &_gen,
		   const boost::filesystem::path &_chk);
	JudgeResult judge();
	void terminate();
	boost::filesystem::path getInputPath()const;
	boost::filesystem::path getOutputPath()const;
	boost::filesystem::path getAnswerPath()const;
	boost::filesystem::path getLogPath()const;
};