#pragma once

#include "message.hpp"

#include <boost/program_options.hpp>

#include <string>

class Options
{
 private:


 public:
	enum RunType{ NONE,VERSION,HELP,TEST,CLEAN };
	RunType run_type;
	std::string file,exe_name,std_name,gen_name,chk_name;
	std::vector<std::string> compiler_opt,gen_opt;
	std::size_t tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt,thread_cnt;
	bool compile_gen,compile_chk;

	Options();
	void parse(int argc,char *argv[]);
};

extern Options opt;
