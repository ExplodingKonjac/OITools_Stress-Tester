#include "options.h"
#include <argp.h>

Options opt{};

const argp_option OptionParser::opt_generic[]={
	{0,0,0,0,"Generic options:",1},
	{"help",'?',"{test|clean|checkers}",OPTION_ARG_OPTIONAL,"Display help message of a specified section, or this information if no arguments are provided."},
	{"usage",500,0,0,"Give a short usage message."},
	{"version",'V',0,0,"Print version information."},
	{0}
};

const argp OptionParser::argp_generic={
	opt_generic,
	parseOption
};

const argp_option OptionParser::opt_test[]={
	{0,0,0,0,"Options for subcommand 'test':",1},
	{"file",'f',"<file>",0,"Set the filename of the files produced to <file>.in/out/ans/log."},
	{"std",'s',"<std>",0,"Set the standard code to <std>.cpp. Default value is `std'."},
	{"gen",'g',"<gen>",0,"Set the generator to <gen>. Default value is `gen'."},
	{"chk",'c',"<chk>",0,"Set the checker to <chk>. Default value is `chk'."},
	{"tl",'T',"<tl>",0,"Set the time limit for testee and standard code to <tl>ms. Default value is `1000'."},
	{"ml",'M',"<ml>",0,"Set the memory limit for testee and standard code to <ml>MB. Default value is `512'."},
	{"tl-gen",501,"<tl>",0,"Set the time limit for generator to <tl>ms. Default value is `5000'."},
	{"ml-gen",502,"<ml>",0,"Set the memory limit for generator to <ml>MB. Default value is `2048'."},
	{"tl-chk",503,"<tl>",0,"Set the time limit for checker to <tl>ms. Default value is `5000'."},
	{"ml-chk",504,"<ml>",0,"Set the memory limit for checker to <ml>MB. Default value is `2048'."},
	{"tests",'n',"{<num>|infinity}",0,"Set the number of testcases to <num> or `infinity'. Default value is `infinity'."},
	{"jobs",'j',"<num>",0,"Set the number of threads used in testing. Default value is `1'."},
	{"compile-gen",505,"<boolean>",OPTION_ARG_OPTIONAL,"Whether to compile <gen>.cpp. Default value is `true'. It is set to `true' if no argument provided."},
	{"compile-chk",506,"<boolean>",OPTION_ARG_OPTIONAL,"Whether to compile <chk>.cpp. Default value is `false'. It is set to `true' if no argument provided."},
	{"-Wc,<args>",0,0,OPTION_DOC,"Comma seperated extra arguments or options that will be passed to compiler. Default value is `-std=c++20,-O2'. Note that it will REPLACE the arguments with <args> but NOT append <args> to it."},
	{"-Wg,<args>",0,0,OPTION_DOC,"Comma seperated extra arguments or options that will be passed to generator. By default it's empty. Note that it will REPLACE the arguments with <args> but NOT append <args> to it."},
	{0,'W',"<args>",OPTION_HIDDEN},
	{0}
};

const argp OptionParser::argp_test={
	opt_test,
	parseOption,
    0,
    "Perform stress test on <code>.cpp.\v"
	"About testee/standard:\n"
	"  Testee/Standard program should read from STDIN and write into STDOUT. Their STDIN and STDOUT will be automatically redirected to some files, of which filenames are not guaranteed, so DON'T read from or write into any files.\n"
	"\n"
	"About generator:\n"
	"  Generator should write into STDOUT, which will be redirected to the input file. You can pass arguments or options to the generator through -Wg.\n"
	"\n"
	"About checker:\n"
	"  Checker code is recommended to be written with testlib. See <https://github.com/MikeMirzayanov/testlib/> for details. Otherwise you should make your checker able to be run in the format 'chk <input> <output> <answer>', and return 0 when the output is accepted and non-zero otherwise. STDERR of checker will be redirected to log file."
};

const argp_option OptionParser::opt_clean[]={
	{0,0,0,0,"Options for subcommand `clean':",1},
	{"file",'f',"<file>",0,"<file>.in/out/ans/log and compile.log will be cleaned."},
	{0}
};

const argp OptionParser::argp_clean={
	opt_clean,
	parseOption,
	0,
	"Clean the file produced during testing."
};

const argp_option OptionParser::opt_checkers[]={ // just for help message
	{0,0,0,0,"Built-in checkers:",2},
	{"chk-caseicmp",0,0,OPTION_DOC,"Compare int64 numbers with testcases."},
	{"chk-casencmp",0,0,OPTION_DOC,"Compare int64 sequences with testcases."},
	{"chk-casewcmp",0,0,OPTION_DOC,"Compare sequences of tokens with testcases."},
	{"chk-dcmp",0,0,OPTION_DOC,"Compare doubles (1e-6 absolute or relative error allowed)."},
	{"chk-fcmp",0,0,OPTION_DOC,"Compare as sequences of lines."},
	{"chk-hcmp",0,0,OPTION_DOC,"Compare signed huge integers."},
	{"chk-icmp",0,0,OPTION_DOC,"Compare int numbers."},
	{"chk-lcmp",0,0,OPTION_DOC,"Compare long int numbers."},
	{"chk-ncmp",0,0,OPTION_DOC,"Compare sequences of long long numbers."},
	{"chk-nyesno",0,0,OPTION_DOC,"Compare two sequences of `YES'/`NO's (case insensitive)."},
	{"chk-rcmp",0,0,OPTION_DOC,"Compare doubles (1.5e-6 absolute error allowed)."},
	{"chk-rcmp4",0,0,OPTION_DOC,"Compare sequences of doubles (1e-4 absolute or relative error allowed)."},
	{"chk-rcmp6",0,0,OPTION_DOC,"Compare sequences of doubles (1e-6 absolute or relative error allowed)."},
	{"chk-rcmp9",0,0,OPTION_DOC,"Compare sequences of doubles (1e-9 absolute or relative error allowed)."},
	{"chk-rncmp",0,0,OPTION_DOC,"Compare sequences of doubles (1.5e-5 absolute error allowed)."},
	{"chk-uncmp",0,0,OPTION_DOC,"Compare unordered sequences of long long numbers."},
	{"chk-wcmp",0,0,OPTION_DOC,"Compare sequences of tokens (it can handle most situations)."},
	{"chk-yesno",0,0,OPTION_DOC,"Compare `YES'/`NO's (case insensitive)."},
	{0}
};

const argp OptionParser::argp_checkers={ // just for help message
	opt_checkers,
	parseOption,
	0,
	"A number of built-in checkers are provided. Note that the directory where built-in checkers are installed should be added to PATH, so that they could be recognized.\v"
	"See <https://github.com/MikeMirzayanov/testlib/tree/master/checkers> for details."
};

const char *OptionParser::version_info=
	"oit-stress " OIT_STRESS_VERSION "\n"
	"Copyright (C) 2023 ExplodingKonjac\n"
	"\n"
	"This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n";

void OptionParser::printHelp(const char *arg,argp_state *state)
{
	if(!arg)
	{
		std::printf("Usage: %s {test|clean} [options...] [args...]\n",state->argv[0]);
		std::printf("   or: %s --help={test|clean|checkers}\n\n",state->argv[0]);
		argp_help(&argp_generic,stdout,ARGP_HELP_LONG|ARGP_HELP_DOC,0);
	}
	else if(!strcmp(arg,"test"))
	{
		std::printf("Usage: %s test [options...] <code>\n\n",state->argv[0]);
		argp_help(&argp_test,stdout,ARGP_HELP_LONG|ARGP_HELP_DOC,0);
	}
	else if(!strcmp(arg,"clean"))
	{
		std::printf("Usage: %s clean [options...]\n\n",state->argv[0]);
		argp_help(&argp_clean,stdout,ARGP_HELP_LONG|ARGP_HELP_DOC,0);
	}
	else if(!strcmp(arg,"checkers"))
		argp_help(&argp_checkers,stdout,ARGP_HELP_LONG|ARGP_HELP_DOC,0);
	else
		argp_failure(state,1,EINVAL,"invalid argument '%s' for --help.",arg);
}

void OptionParser::printUsage(argp_state *state)
{
	std::printf("Usage: %s {test|clean} [options...] [args...]\n",state->argv[0]);
	std::printf("   or: %s --help={test|clean|checkers}\n",state->argv[0]);
}

int OptionParser::parseOption(int key,char *arg,argp_state *state)
{
	OptionParser &parser=*static_cast<OptionParser*>(state->input);
	const char *token;
	auto arg2num=[&](auto &res,const char *opt) {
		try { res=std::stoull(arg); }
	 	catch(...) { argp_failure(state,1,EINVAL,"invalid argument '%s' for %s",arg,opt); }
	};
	auto arg2bool=[&](auto &res,const char *opt) {
		if(!arg || !strcasecmp(arg,"true") || !strcasecmp(arg,"yes") || !strcasecmp(arg,"on"))
			res=true;
		else if(!strcasecmp(arg,"false") || !strcasecmp(arg,"no") || !strcasecmp(arg,"off"))
			res=false;
		else
			argp_failure(state,1,EINVAL,"invalid argument '%s' for %s",arg,opt);
	};

	switch(key)
	{
	 case '?': // --help
		printHelp(arg,state);
		exit(0);
	 case 500: // --usage
		printUsage(state);
		exit(0);
	 case 'V': // --version
		fputs(version_info,stdout);
		exit(0);
	 case 'f': // --file
		parser.result.file=arg;
		break;
	 case 's': // --std
		parser.result.std_name=arg;
		break;
	 case 'g': // --gen
		parser.result.gen_name=arg;
		break;
	 case 'c': // --chk
		parser.result.chk_name=arg;
		break;
	 case 'T': // --tl
		arg2num(parser.result.tl,"--tl");
		break;
	 case 'M': // --ml
	 	arg2num(parser.result.ml,"--ml");
		break;
	 case 501: // --tl-gen
		arg2num(parser.result.tl_chk,"--tl-gen");
		break;
	 case 502: // --ml-gen
	 	arg2num(parser.result.ml_chk,"--ml-gen");
		break;
	 case 503: // --tl-chk
		arg2num(parser.result.tl_chk,"--tl-chk");
		break;
	 case 504: // --ml-chk
	 	arg2num(parser.result.ml_chk,"--ml-chk");
		break;
	 case 'n': // --tests
		if(!strcmp(arg,"infinity"))
			parser.result.test_cnt=SIZE_MAX;
		else
			arg2num(parser.result.test_cnt,"--tests");
		break;
	 case 'j': // --jobs
		arg2num(parser.result.thread_cnt,"--jobs");
		break;
	 case 505: // --compile-gen
		arg2bool(parser.result.compile_gen,"--compile-gen");
		break;
	 case 506: // --compile-chk
		arg2bool(parser.result.compile_chk,"--compile-chk");
		break;
	 case 'W':
		token=strtok(arg,",");
		if(strcmp(token,"c") && strcmp(token,"a"))
			argp_error(state,"invalid option -- '-W%s'",token);
		else
		{
			std::vector<std::string> &target=(key==507?parser.result.compiler_opt:parser.result.gen_opt);
			target.clear();
			while((token=strtok(NULL,",")))
				target.emplace_back(token);
			break;
		}
	 case ARGP_KEY_ARG:
		parser.result.args.emplace_back(arg);
		break;
	 default:
		break;
	}
	return 0;
}

Options OptionParser::parse(int argc,char *argv[])
{
	result=Options{
		{},
		"data","code","std","gen","chk-wcmp",
		{"-std=c++20","-O2"},{},
		1000,512,
		5000,2048,
		5000,2048,
		SIZE_MAX,1,
		true,false
	};
	if(argc>1 && !strcmp(argv[1],"test"))
		argp_parse(&argp_test,argc,argv,ARGP_NO_HELP|ARGP_IN_ORDER,0,this);
	else if(argc>1 && !strcmp(argv[1],"clean"))
		argp_parse(&argp_clean,argc,argv,ARGP_NO_HELP|ARGP_IN_ORDER,0,this);
	else
	{
		argp_parse(&argp_generic,argc,argv,ARGP_NO_HELP|ARGP_NO_ERRS,0,this);
		if(argc==1)
			throw std::runtime_error("no subcommand provided");
		else
			throw std::runtime_error(std::format("unrecognized subcommand `{0}'",argv[1]));
	}
	return result;
}