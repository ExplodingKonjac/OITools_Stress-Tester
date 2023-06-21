#include "core.h"
#include "helper.h"
#include "tester.h"
#include "cleaner.h"

void parseOption(const std::string &s)
{
	auto pos=s.find('=');
	std::string key=(pos==s.npos?s:s.substr(0,pos)),val;
	auto convert=[&](auto &res,const std::string &s)
	{
		static std::istringstream in;
		in.clear(),in.str(s);
		in>>res;
		if(in.fail()) quitError("Failed to read value \'%s\'",s.c_str());
	};
	auto getval=[&]()
	{
		if(pos==s.npos)
			quitError("Missing value for key \'%s\'",key.c_str());
		val=s.substr(pos+1);
	};
	switch(strhash(key.c_str()))
	{
	 case strhash("-std"):
		getval(),opt.std_name=val;
		break;
	 case strhash("-gen"):
		getval(),opt.gen_name=val;
		break;
	 case strhash("-chk"):
		getval(),opt.chk_name=val;
		break;
	 case strhash("-tl"):
	 	getval(),convert(opt.tl,val);
		break;
	 case strhash("-tl-gen"):
	 	getval(),convert(opt.tl_gen,val);
		break;
	 case strhash("-tl-chk"):
	 	getval(),convert(opt.tl_chk,val);
		break;
	 case strhash("-ml"):
	 	getval(),convert(opt.ml,val),opt.ml<<=20;
		break;
	 case strhash("-ml-gen"):
	 	getval(),convert(opt.ml_gen,val),opt.ml<<=20;
		break;
	 case strhash("-ml-chk"):
	 	getval(),convert(opt.ml_chk,val),opt.ml<<=20;
		break;
	 case strhash("-file"):
	 	getval(),opt.file=val;
		break;
	 case strhash("-opt"):
	 	opt.compile_opt="";
		getval();
		for(std::size_t p=0,nxt;p!=val.npos;)
		{
			nxt=s.find(',',p);
			std::string s;
			if(nxt==s.npos) s=val.substr(p),p=nxt;
			else s=val.substr(p,nxt-p),p=nxt+1;
			opt.compile_opt+=('"'+s+'"');
		}
		break;
	 case strhash("-cnt"):
	 	getval();
	 	if(val=="unlimited") opt.test_cnt=SIZE_MAX;
		else convert(opt.test_cnt,val.c_str());
		break;
	 default:
		quitError("Unknown option %s",key.c_str());
	}
}

int main(int argc,char *argv[])
{
	if(argc<2) quitError("Invalid usage. Use \"stress help\" to get help.");
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