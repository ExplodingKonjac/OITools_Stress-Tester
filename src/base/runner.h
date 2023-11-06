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
	enum Types{ OK=1,TLE,MLE,RE,KILLED,UKE };
	Types type;
	unsigned exit_code;
	std::size_t time_used,memory_used;
	RunnerResult();
	RunnerResult(Types _tp,unsigned _e,std::size_t _t,std::size_t _m);
};

class Runner
{
 private:
	std::string name;
	boost::filesystem::path app;
	std::size_t tl,ml;
	std::string fn_in,fn_out,fn_err;
	boost::process::child proc;
	std::thread watcher;
	RunnerResult res;

	void watching(FILE *inf,FILE *ouf,FILE *erf);
	static std::vector<boost::filesystem::path> getSearchPaths(bool env);

 public:
	Runner(const std::string &_name,const std::string &_app,std::size_t _tl,std::size_t _ml,bool _env=false);
	~Runner();
	void setInputFile(const std::string &file);
	void setOutputFile(const std::string &file);
	void setErrorFile(const std::string &file);
	void setName(const std::string &name);
	void setTimeLimit(std::size_t tl);
	void setMemoryLimit(std::size_t ml);
	const std::string &getInputFile()const;
	const std::string &getOutputFile()const;
	const std::string &getErrorFile()const;
	const std::string &getName()const;
	std::size_t getTimeLimit()const;
	std::size_t getMemoryLimit()const;
	const RunnerResult &getLastResult()const;
	bool running();
	void start(const std::vector<std::string> &args={});
	void terminate();
	const RunnerResult &wait();
};