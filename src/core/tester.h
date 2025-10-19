#pragma once

#include "config.hpp"
#include "message.hpp"
#include "common.hpp"
#include "options.h"
#include "judger.h"

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/scope/scope_exit.hpp>

#include <csignal>
#include <mutex>
#include <atomic>
#include <thread>
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
	struct ACResult
	{};
	struct WAResult
	{
		std::string msg;
	};
	struct TLEResult
	{
		std::string_view name;
		std::uint64_t time_used;
		std::uint64_t time_limit;
	};
	struct MLEResult
	{
		std::string_view name;
		std::uint64_t memory_used;
		std::uint64_t memory_limit;
	};
	struct REResult
	{
		std::string_view name;
		int exit_code;
	};
	struct UKEResult
	{
		std::string name;
	};
	using ResultType=std::variant<
		std::exception_ptr,
		ACResult,
		WAResult,
		TLEResult,
		MLEResult,
		REResult,
		UKEResult
	>;

	class JudgeThread
	{
	 public:
		JudgeThread(Tester &parent,std::size_t id);
		~JudgeThread()=default;
		void run(std::stop_token);
		void interrupt();
		void wait();
		const Judger &judger();

	 private:
		Tester &_parent;
		std::size_t _id;
		Judger _judger;
		std::jthread _thread;
	};

	static void compileOne(const std::string &filename,
						   const std::vector<std::string> &extra_opt,
						   std::mutex &mtx,
						   std::ofstream &fout,
						   std::exception_ptr &ep);
	static fs::path getExePath(const std::string &name,bool in_path=false);
	static void compileExecutables();
	static fs::path createTempDirectory();
	void judgingThread(std::size_t id);
	void handleResult(std::size_t idx,std::size_t id,const std::exception_ptr &ep);
	void handleResult(std::size_t idx,std::size_t id,const ACResult &res);
	void handleResult(std::size_t idx,std::size_t id,const WAResult &res);
	void handleResult(std::size_t idx,std::size_t id,const TLEResult &res);
	void handleResult(std::size_t idx,std::size_t id,const MLEResult &res);
	void handleResult(std::size_t idx,std::size_t id,const REResult &res);
	void handleResult(std::size_t idx,std::size_t id,const UKEResult &res);
	void moveFiles(std::size_t id);

	fs::path prefix,exe_path,std_path,gen_path,chk_path;
	std::atomic<std::size_t> tot;
	std::vector<std::unique_ptr<JudgeThread>> threads;
	std::atomic<bool> flag_stop,flag_pause;
	std::queue<std::pair<std::size_t,ResultType>> result_q;
	std::mutex mtx_q;
	std::condition_variable cond_q;

 public:
	Tester()=default;
	~Tester();
	void start();
};
