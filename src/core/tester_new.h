#pragma once

#include "defines.hpp"

#include "options.h"
#include "judger.h"

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <mutex>

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;

class Tester
{
 private:
	std::queue<std::pair<int,JudgeResult>> result_q;
	std::size_t tot;
	std::mutex mtx_q;
	std::condition_variable cond_q,cond_pause;
	bool stop_flag,pause_flag;

	static void compileOne(const std::string &filename,const std::vector<std::string> &extra_opt,std::mutex &mtx);
	void judgingThread(int id,Judger &judger);
	void compileExecutables();
	fs::path createTempDirectory();

 public:
	Tester();
	void start();
};
