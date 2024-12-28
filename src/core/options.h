#pragma once

#include "config.hpp"
#include "message.hpp"
#include "info.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <exception>
#include <sstream>

#include <unistd.h>
#include <getopt.h>

struct Options
{
	std::vector<std::string> args;
	std::string file,exe_name,std_name,gen_name,chk_name;
	std::vector<std::string> opt_compile_exe,opt_compile_gen,opt_compile_chk,opt_exe,opt_gen,opt_chk;
	std::size_t tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt,thread_cnt;
	bool compile_gen,compile_chk;
};

extern Options opt;

class OptionParser
{
 private:
	int argc;
	char **argv;
	Info info;

	void handleError(const option *long_options);
	bool parseGeneric(Options &result);
	bool parseTest(Options &result);
	bool parseClean(Options &result);

 public:
	OptionParser(int argc,char *argv[]);
	~OptionParser();
	bool parse(Options &result);
};