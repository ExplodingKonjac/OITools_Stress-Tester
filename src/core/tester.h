#pragma once

#include "config.hpp"

#include "message.hpp"
#include "options.h"
#include "judger.h"

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/scope/scope_exit.hpp>

#include <csignal>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <format>
#include <memory>
#include <functional>
#include <queue>
#include <exception>

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;

class Tester
{
 private:
	std::queue<std::pair<std::size_t,JudgeResult>> result_q;
	std::size_t tot;
	std::mutex mtx_q;
	std::condition_variable cond_q,cond_pause;
	fs::path prefix,exe_path,std_path,gen_path,chk_path;
	std::vector<std::thread> threads;
	std::vector<Judger> judgers;
	std::atomic<bool> stop_flag,pause_flag;

	static void compileOne(const std::string &filename,const std::vector<std::string> &extra_opt,std::mutex &mtx,std::ofstream &fout,std::exception_ptr &ep);
	static fs::path getExePath(const std::string &name,bool in_path=false);
	static void compileExecutables();
	static fs::path createTempDirectory();
	void judgingThread(std::size_t id);
	void handleWrongAnswer(std::size_t idx,std::size_t id);
	void handleBadResult(const std::string &name,const ProcessInfo &info,std::size_t tl,std::size_t ml);
	void moveFiles(std::size_t id);

 public:
	Tester();
	~Tester();
	void start();
};
