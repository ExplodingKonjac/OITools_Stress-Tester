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

static const option long_opts[]={
	{"file",required_argument,0,'f'},
	{"std",required_argument,0,'s'},
	{"gen",required_argument,0,'g'},
	{"chk",required_argument,0,'c'},
	{"time-limit",required_argument,0,'T'},
	{"memory-limit",required_argument,0,'M'},
	{"tl-gen",required_argument,0,501},
	{"ml-gen",required_argument,0,502},
	{"tl-chk",required_argument,0,503},
	{"ml-chk",required_argument,0,504},
	{"compile-opt",required_argument,0,'p'},
	{"count",required_argument,0,'n'},
	{"compile-gen",optional_argument,0,505},
	{"compile-chk",optional_argument,0,506},
	{nullptr,0,0,0}
};
static const char short_opts[]=
	"f:s:g:c:T:M:p:n:"
;
void parseOptions(int argc,char *argv[])
{
	std::istringstream is;
	is>>std::boolalpha;
	auto convert=[&](auto &res,const char *val)
	{
		is.clear(),is.str(val);
		is>>res;
		if(is.fail()) quitError("Failed to read value %s for option %s.",val,argv[optind-1]);
	};
	int arg,idx;
	opterr=0;
	while(~(arg=getopt_long(argc,argv,short_opts,long_opts,&idx)))
	{
		switch(arg)
		{
		 case 'f':
			opt.file=optarg;
			break;
		 case 's':
		 	opt.std_name=optarg;
			break;
		 case 'g':
		 	opt.gen_name=optarg;
			break;
		 case 'c':
			opt.chk_name=optarg;
			break;
		 case 'T':
			convert(opt.tl,optarg);
			break;
		 case 'M':
			convert(opt.ml,optarg);
			break;
		 case 'p':
			opt.compile_opt=optarg;
			break;
		 case 'n':
		 	if(!strcmp(optarg,"infinite"))
				opt.test_cnt=-1;
			else convert(opt.test_cnt,optarg);
			break;
		 case 501:
			convert(opt.tl_gen,optarg);
			break;
		 case 502:
			convert(opt.ml_gen,optarg);
			break;
		 case 503:
			convert(opt.tl_chk,optarg);
			break;
		 case 504:
			convert(opt.ml_chk,optarg);
			break;
		 case 505:
			if(!optarg) opt.compile_gen=true;
			else convert(opt.compile_gen,optarg);
			break;
		 case 506:
			if(!optarg) opt.compile_chk=true;
			else convert(opt.compile_chk,optarg);
			break;
		 default:
			quitError("Unknown option '%s'. Use 'stress help' to get help.",argv[optind-1]);
			break;
		}
	}
}