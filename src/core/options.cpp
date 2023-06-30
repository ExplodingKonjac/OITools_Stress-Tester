#include "options.h"

Options::Options():
	file("data"),
	pro_name("code"),
	std_name("std"),
	gen_name("gen"),
	chk_name("chk-wcmp"),
	compile_opt("-std=c++17 -O2"),
	tl(1000),ml(512<<20),
	tl_gen(5000),ml_gen(2048<<20),
	tl_chk(5000),ml_chk(2048<<20),
	test_cnt(SIZE_MAX),
	compile_gen(true),compile_chk(false)
{}

Options opt{};