#pragma once

#include "config.hpp"
#include "message.hpp"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <exception>
#include <argp.h>

struct Options
{
	std::vector<std::string> args;
	std::string file,exe_name,std_name,gen_name,chk_name;
	std::vector<std::string> compiler_opt,gen_opt;
	std::size_t tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt,thread_cnt;
	bool compile_gen,compile_chk;
};

extern Options opt;

class OptionParser
{
 private:
	static const argp_option opt_generic[],opt_test[],opt_clean[],opt_checkers[];
	static const argp argp_generic,argp_test,argp_clean,argp_checkers;
	static const char *version_info;

	Options result;

	static void printHelp(const char *arg,argp_state *state);
	static void printUsage(argp_state *state);
	static int parseOption(int key,char *arg,argp_state *state);

 public:
	Options parse(int argc,char *argv[]);
};