#include "options.h"

using namespace std::string_literals;

class Converter
{
 private:
	const option *long_options;
	int &long_idx,&c;

	void reportInvalid()
	{
		std::string err_msg="invalid argument `"s+optarg+"' -- ";
		if(long_idx==-1) err_msg+=c;
		else err_msg+=long_options[long_idx].name;
		msg.error("{0}",err_msg);
	}

 public:
	Converter(const option *_long_options,int &_long_idx,int &_c):
		long_options(_long_options),long_idx(_long_idx),c(_c)
	{}
	template<std::integral IntT>
	bool operator ()(IntT &res)
	{
		std::istringstream iss(optarg);
		iss>>res;
		if(iss.fail() || !iss.eof())
			return reportInvalid(),false;
		return true;
	}
	bool operator ()(bool &res,bool default_value=true)
	{
		if(!optarg)
			res=default_value;
		else if(!strcasecmp(optarg,"true") || !strcasecmp(optarg,"yes") || !strcasecmp(optarg,"on"))
			res=true;
		else if(!strcasecmp(optarg,"false") || !strcasecmp(optarg,"no") || !strcasecmp(optarg,"off"))
			res=false;
		else
			return reportInvalid(),false;
		return true;
	}
	bool operator ()(std::vector<std::string> &res,char *bg=nullptr)
	{
		char *token=nullptr;
		res.clear();
		while((token=strtok(bg,",")))
			res.emplace_back(token);
		return true;
	}
};

enum _OptionType
{
	OPT_HELP='h',
	OPT_VERSION='V',
	OPT_FILE='f',
	OPT_STD='s',
	OPT_GEN='g',
	OPT_CHK='c',
	OPT_TL='T',
	OPT_ML='M',
	OPT_TESTS='n',
	OPT_JOBS='j',
	OPT_USAGE=500,
	OPT_TL_GEN,
	OPT_ML_GEN,
	OPT_TL_CHK,
	OPT_ML_CHK,
	OPT_COMPILE_GEN,
	OPT_COMPILE_CHK
};

Options opt{};

OptionParser::OptionParser(int _argc,char *_argv[]):
	argc(_argc),argv(new char*[_argc+1]{}),info(_argc,_argv)
{
	for(int i=0;i<_argc;i++)
	{
		int len=strlen(_argv[i]);
		argv[i]=new char[len+1];
		strcpy(argv[i],_argv[i]);
	}
}

OptionParser::~OptionParser()
{
	for(int i=0;i<argc;i++)
		delete[] argv[i];
	delete[] argv;
}

void OptionParser::handleError(const option *long_options)
{
	if(optopt)
	{
		const option *x=nullptr;
		for(auto i=long_options;i->name;i++)
			if(i->val==optopt)
				{ x=i; break; }
		if(!x)
			msg.error("unrecognized option -- {0}",(char)optopt);
		else if(x->has_arg==no_argument)
			msg.error("option doesn't take an argument -- {0}",x->name);
		else if(argv[optind-1][1]=='-')
			msg.error("option requires an argument -- {0}",x->name);
		else
			msg.error("option requires an argument -- {0}",(char)optopt);
	}
	else
		msg.error("unrecognized option -- {0}",argv[optind-1]+2);
}

bool OptionParser::parseGeneric(Options &result)
{
	static const option long_options[]={
		{"help",optional_argument,0,OPT_HELP},
		{"usage",no_argument,0,OPT_USAGE},
		{"version",no_argument,0,OPT_VERSION},
		{0}
	};
	while(true)
	{
		int long_idx=-1,c=getopt_long(argc,argv,"?h::V",long_options,&long_idx);
		if(c==-1) break;

		switch(c)
		{
		 case OPT_HELP:
			if(!optarg)
				std::cout<<info.helpGenericText();
			else if(!strcmp(optarg,"test"))
				std::cout<<info.helpTestText();
			else if(!strcmp(optarg,"clean"))
				std::cout<<info.helpCleanText();
			else if(!strcmp(optarg,"checkers"))
				std::cout<<info.helpCheckersText();
			else
				return msg.error("unknown help type"),false;
			return true;
		 case OPT_VERSION:
		 	std::cout<<info.versionInfo();
			return true;
		 case OPT_USAGE:
			std::cout<<info.usageText();
			return true;
		 case '?':
			handleError(long_options);
			return false;
		}
	}
	return false;
}

bool OptionParser::parseTest(Options &result)
{
	static const option long_options[]={
		{"file",required_argument,0,OPT_FILE},
		{"std",required_argument,0,OPT_STD},
		{"gen",required_argument,0,OPT_GEN},
		{"chk",required_argument,0,OPT_CHK},
		{"tl",required_argument,0,OPT_TL},
		{"ml",required_argument,0,OPT_ML},
		{"tl-gen",required_argument,0,OPT_TL_GEN},
		{"ml-gen",required_argument,0,OPT_ML_GEN},
		{"tl-chk",required_argument,0,OPT_TL_CHK},
		{"ml-chk",required_argument,0,OPT_ML_CHK},
		{"tests",required_argument,0,OPT_TESTS},
		{"jobs",required_argument,0,OPT_JOBS},
		{"compile-gen",optional_argument,0,OPT_COMPILE_GEN},
		{"compile-chk",optional_argument,0,OPT_COMPILE_CHK},
		{0}
	};
	while(true)
	{
		int long_idx=-1,c=getopt_long(argc,argv,"?f:s:g:c:T:M:n:j:W:X:",long_options,&long_idx);
		if(c==-1) break;
		Converter convert(long_options,long_idx,c);
		char *first_token;

		switch(c)
		{
		 case OPT_FILE:
			result.file=optarg;
			break;
		 case OPT_STD:
			result.std_name=optarg;
			break;
		 case OPT_GEN:
			result.gen_name=optarg;
			break;
		 case OPT_CHK:
			result.chk_name=optarg;
			break;
		 case OPT_TL:
			if(!convert(result.tl)) return false;
			break;
		 case OPT_ML:
			if(!convert(result.ml)) return false;
			break;
		 case OPT_TL_GEN:
			if(!convert(result.tl_gen)) return false;
			break;
		 case OPT_ML_GEN:
			if(!convert(result.ml_gen)) return false;
			break;
		 case OPT_TL_CHK:
			if(!convert(result.tl_chk)) return false;
			break;
		 case OPT_ML_CHK:
			if(!convert(result.ml_chk)) return false;
			break;
		 case OPT_TESTS:
			if(!strcmp(optarg,"infinity"))
				result.test_cnt=SIZE_MAX;
			else if(!convert(result.test_cnt)) return false;
			break;
		 case OPT_JOBS:
			if(!convert(result.thread_cnt)) return false;
			break;
		 case OPT_COMPILE_GEN:
			if(!convert(result.compile_gen)) return false;
			break;
		 case OPT_COMPILE_CHK:
			if(!convert(result.compile_chk)) return false;
			break;
		 case 'W': case 'X':
			first_token=strtok(optarg,",");
			if(!strcmp(first_token,"e"))
				convert(c=='W'?result.opt_compile_exe:result.opt_exe);
			else if(!strcmp(first_token,"g"))
				convert(c=='W'?result.opt_compile_gen:result.opt_gen);
			else if(!strcmp(first_token,"c"))
				convert(c=='W'?result.opt_compile_chk:result.opt_chk);
			else
			{
				msg.error("unrecognized option -- {0}{1}",(char)c,first_token);
				return false;
			}
		 	break;
		 case '?':
			handleError(long_options);
			return false;
		}
	}
	while(optind<argc)
		result.args.emplace_back(argv[optind++]);
	return true;
}

bool OptionParser::parseClean(Options &result)
{
	static const option long_options[]={
		{"file",required_argument,0,OPT_FILE},
		{0}
	};
	while(true)
	{
		int long_idx=-1,c=getopt_long(argc,argv,"?f:s:g:c:T:M:n:j:W:",long_options,&long_idx);
		if(c==-1) break;
		
		switch(c)
		{
		 case OPT_FILE:
			result.file=optarg;
			break;
		 case '?':
			handleError(long_options);
			return false;
		}
	}
	while(optind<argc)
		result.args.emplace_back(argv[optind++]);
	return true;
}

bool OptionParser::parse(Options &result)
{
	result=Options{
		{},
		"data","code","std","gen","chk-wcmp",
		{"-std=c++20","-O2"},{"-std=c++20","-O2"},{"-std=c++20","-O2"},
		{},{},{},
		1000,512,
		5000,2048,
		5000,2048,
		SIZE_MAX,1,
		true,false
	};
	opterr=0;

	bool res=false;
	if(argc>1 && !strcmp(argv[1],"test"))
		res=parseTest(result);
	else if(argc>1 && !strcmp(argv[1],"clean"))
		res=parseClean(result);
	else
		res=parseGeneric(result);
	if(!res)
		std::cout<<info.shortHintText();
	return res;
}
