#pragma once

#include "lib.hpp"
#include <getopt.h>

struct Options
{
	enum RunType{ NONE,VERSION,HELP,TEST,CLEAN };
	RunType run_type;
	std::string file,exe_name,std_name,gen_name,chk_name;
	std::vector<std::string> compiler_opt;
	std::size_t tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt,threads_cnt;
	bool compile_gen,compile_chk;
	Options();
};

extern Options opt;

void parseOptions(int argc,char *argv[]);