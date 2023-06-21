#pragma once
#include "lib.hpp"

struct Options
{
	std::string file,pro_name,std_name,gen_name,chk_name;
	std::string compile_opt;
	SIZE_T tl,ml,tl_gen,ml_gen,tl_chk,ml_chk,test_cnt;
	Options();
};

extern Options opt;