#pragma once
#include "lib.hpp"

struct Options
{
	std::string file,pro_name,std_name,gen_name,chk_name;
	std::string compile_opt;
	SIZE_T tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt;
	bool compile_gen,compile_chk;
	Options();
};

extern Options opt;