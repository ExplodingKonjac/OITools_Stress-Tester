#include "options.h"

namespace po=boost::program_options;

Options::Options():
	run_type(NONE),
	file("data"),
	exe_name("code"),
	std_name("std"),
	gen_name("gen"),
	chk_name("chk-wcmp"),
	compiler_opt{"-std=c++17","-O2"},
	gen_opt{},
	tl(1000),ml(512),
	tl_gen(5000),ml_gen(2048),
	tl_chk(5000),ml_chk(2048),
	test_cnt(SIZE_MAX),
	thread_cnt(1),
	compile_gen(true),compile_chk(false)
{}

Options opt{};

void Options::parse(int argc,char *argv[])
{
	po::options_description generic_desc("Generic options");
	generic_desc.add_options()
		("help,h",po::value<std::string>()->default_value("generic"),"get help information")
		("version","get version information")
	;
	po::options_description hidden_desc("Hidden options");
	hidden_desc.add_options()
		("arguments","positional arguments")
	;
	po::positional_options_description positional_desc{};
	positional_desc.add("arguments",-1);

	po::options_description test_desc("`test' command options");
	test_desc.add_options()
		("file,f",po::value<std::string>()->default_value("data"))
		("std,s",po::value<std::string>()->default_value("std"))
		("gen,g",po::value<std::string>()->default_value("gen"))
		("chk,c",po::value<std::string>()->default_value("chk-wcmp"))
		("tl,T",po::value<std::size_t>()->default_value(1000))
		("ml,M",po::value<std::size_t>()->default_value(512))
		("tl-gen",po::value<std::size_t>()->default_value(5000))
		("ml-gen",po::value<std::size_t>()->default_value(2048))
		("tl-chk",po::value<std::size_t>()->default_value(5000))
		("ml-chk",po::value<std::size_t>()->default_value(2048))
		("compiler-opt,C",po::value<std::vector<std::string>>())
		("gen-opt,G",po::value<std::vector<std::string>>())
		("test-count,n",po::value<std::string>()->default_value("infinity"))
		("jobs,j",po::value<std::size_t>()->default_value(1))
		("compile-gen",po::bool_switch()->default_value(true))
		("compile-chk",po::bool_switch()->default_value(false))
	;
	po::options_description all_desc("All options");
	all_desc.add(generic_desc).add(hidden_desc).add(test_desc);

	po::variables_map vm;
	po::store(po::command_line_parser(argc,argv).options(all_desc).positional(positional_desc).run(),vm);
	vm.notify();
}