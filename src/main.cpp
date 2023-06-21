#include "core.h"
#include "helper.h"
#include "tester.h"
#include "cleaner.h"

static std::istringstream sstr;
void parseOption(const std::string &s)
{
	auto convert=[&](auto &res,const std::string &s)
	{
		sstr.clear();
		sstr.str(s);
		sstr>>res;
		if(sstr.fail()) quitError("Failed to read value '%s'",s.c_str());
	};
	auto pos=s.find('=');
	std::string key=(pos==s.npos?s:s.substr(0,pos)),val;
	if(pos!=s.npos) val=s.substr(pos+1);
	switch(strhash(key.c_str()))
	{
	 case strhash("-std"):
		opt.std_name=val;
		break;
	 case strhash("-gen"):
		opt.gen_name=val;
		break;
	 case strhash("-chk"):
		opt.chk_name=val;
		break;
	 case strhash("-tl"):
	 	convert(opt.tl,val);
		break;
	 case strhash("-tl-gen"):
	 	convert(opt.tl_gen,val);
		break;
	 case strhash("-tl-chk"):
	 	convert(opt.tl_chk,val);
		break;
	 case strhash("-ml"):
	 	convert(opt.ml,val),opt.ml<<=20;
		break;
	 case strhash("-ml-gen"):
	 	convert(opt.ml_gen,val),opt.ml<<=20;
		break;
	 case strhash("-ml-chk"):
	 	convert(opt.ml_chk,val),opt.ml<<=20;
		break;
	 case strhash("-file"):
	 	opt.file=val;
		break;
	 case strhash("-opt"):
		opt.compile_opt=val;
		break;
	 case strhash("-cnt"):
	 	if(val=="unlimited") opt.test_cnt=SIZE_MAX;
		else convert(opt.test_cnt,val.c_str());
		break;
	 case strhash("-cp-gen"):
		convert(opt.compile_gen,val);
		break;
	 case strhash("-cp-chk"):
		convert(opt.compile_chk,val);
		break;
	 default:
		quitError("Unknown option '%s'",key.c_str());
	}
}

int main(int argc,char *argv[])
{
	if(argc<2) quitError("Invalid usage. Use \"stress help\" to get help.");
	sstr>>std::boolalpha;
	switch(strhash(argv[1]))
	{
	 case strhash("help"):
	 	Helper::main();
	 	break;
	 case strhash("test"):
		if(argc<3)
			quitError("Missing name of program.");
		opt.pro_name=argv[2];
		for(int i=3;i<argc;i++)
			parseOption(std::string(argv[i]));
		Tester::main();
		break;
	 case strhash("clean"):
	 	for(int i=3;i<argc;i++)
			parseOption(std::string(argv[i]));
		Cleaner::main();
		break;
	 default:
	 	quitError("Unknown module name %s. Use \"stress help\" to get help.",argv[1]);
		break;
	}
	return 0;
}