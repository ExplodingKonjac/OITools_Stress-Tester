#include "tester.h"

namespace bp=boost::process;
namespace asio=boost::asio;

namespace Tester
{

void compileFiles()
{
	using Handler=std::function<void(const boost::system::error_code&,std::size_t)>;

	std::ofstream fout("compile.log");
	std::mutex locker;
	auto compileOne=[&](const std::string &name,const std::string &extra_opt)
	{
		asio::io_context ios;
		bp::async_pipe ap(ios);
		bp::child proc(
			"g++ \""+name+".cpp\" -o \""+name+"\" "+extra_opt,
			bp::std_err>ap,ios,
			bp::on_exit=[&](auto...){ ap.close(); }
		);
		std::string msg,buf(512,0);
		Handler func=[&](auto ec,auto siz)
		{
			msg.append(buf.data(),siz);
			if(!ec) asio::async_read(ap,asio::buffer(buf),func);
		};
		asio::async_read(ap,asio::buffer(buf),func);
		ios.run();
		proc.wait();
		int ret=proc.exit_code();
		locker.lock();
		if(ret==0) fout<<name<<".cpp: successfully compiled. Compiler messages:\n"<<msg;
		else fout<<name<<".cpp: compile error. Compiler messages:\n"<<msg;
		fout.flush();
		if(ret==0) printMessage("%s.cpp has been compiled.",name.c_str());
		else quitFailed("Compilation error on %s.cpp. See compile.log for details.",name.c_str());
		locker.unlock();
	};
	std::vector<std::thread> vec;
	if(opt.compile_gen) vec.emplace_back(compileOne,opt.gen_name,"");
	if(opt.compile_chk) vec.emplace_back(compileOne,opt.chk_name,"");
	vec.emplace_back(compileOne,opt.pro_name,opt.compile_opt);
	vec.emplace_back(compileOne,opt.std_name,opt.compile_opt);
	for(auto &i: vec) i.join();
	fout.close();
	std::fputc('\n',stderr);
}

int checkResult(Runner *run,bool ignore_re=false)
{
	auto &res=run->wait();
	auto name=run->getName().c_str();
	switch(res.type)
	{
	 case RunnerResult::TLE:
		printColor(TextAttr::fg_yellow,"%s Time Limit Exceeded",name);
		if(res.time_used==(size_t)-1)
			fprintf(stderr," (killed)\n");
		else
			fprintf(stderr," (%zums/%zums)\n",res.time_used,run->getTimeLimit());
		return 1;
	 case RunnerResult::MLE:
	 	printColor(TextAttr::fg_yellow,"%s Memory Limit Exceeded",name);
		fprintf(stderr," (%.2lfMB/%.2lfMB)\n",res.memory_used/1024.0/1024.0,run->getMemoryLimit()/1024.0/1024.0);
		return 2;
	 case RunnerResult::RE:
		if(ignore_re) break;
		printColor(TextAttr::fg_red,"%s Runtime Error (%u)\n",name,res.exit_code);
		return 3;
	 case RunnerResult::KILLED:
		printColor(TextAttr::fg_purple,"%s Terminated\n",name);
		return -1;
	 case RunnerResult::UKE:
		printColor(TextAttr::bg_purple|TextAttr::fg_white,"%s Unknown Error\n",name);
		return -1;
	 case RunnerResult::OK:
		break;
	}
	return 0;
}

void main(const std::vector<const char*> &args)
{
	if(args.size()>1)
		printNote("Redundant arguments ignored.");
	if(args.empty())
		quitError("Missing testee code.");
	opt.pro_name=args[0];

	compileFiles();

	Runner *gen_run=new Runner("Generator",opt.gen_name,opt.tl_gen,opt.ml_gen),
		   *chk_run=new Runner("Checker",opt.chk_name,opt.tl_chk,opt.ml_chk,true),
		   *pro_run=new Runner("Testee",opt.pro_name,opt.tl,opt.ml),
		   *std_run=new Runner("Standard",opt.std_name,opt.tl,opt.ml);
	gen_run->setOutputFile(opt.file+".in");
	pro_run->setInputFile(opt.file+".in");
	std_run->setInputFile(opt.file+".in");
	pro_run->setOutputFile(opt.file+".out");
	std_run->setOutputFile(opt.file+".ans");
	chk_run->setErrorFile(opt.file+".log");

	volatile bool force_quit=false;
	static std::function<void()> tryQuit=[&]
	{
		force_quit=true;
		pro_run->terminate();
		std_run->terminate();
		gen_run->terminate();
		chk_run->terminate();
	};
	std::signal(SIGINT,[](int x){ tryQuit(); });

	std::vector<std::string> chk_argu{opt.file+".in",opt.file+".out",opt.file+".ans"};
	for(std::size_t id=1;id<=opt.test_cnt;id++)
	{
		std::fprintf(stderr,"Testcase #%zu: ",id);

		gen_run->start();
		if(checkResult(gen_run)) goto bad;
		pro_run->start();
		std_run->start();
		if(checkResult(pro_run)) goto bad;
		if(checkResult(std_run)) goto bad;
		chk_run->start(chk_argu);
		if(checkResult(chk_run,true)) goto bad;

		if(chk_run->getLastResult().type!=RunnerResult::RE)
			printColor(TextAttr::fg_green|TextAttr::intensity,"Accepted\n");
		else
		{
			std::ifstream inf(opt.file+".log");
			std::string msg(256,0);
			inf.read(msg.data(),msg.size());
			msg.resize(inf.gcount());
			if(msg.size()==256) msg+="...";
			msg="Failed on testcase #"+std::to_string(id)+" (256 bytes only):\n"+msg+'\n'+
				"See "+opt.file+".log for detail.";
			printColor(TextAttr::fg_red|TextAttr::intensity,"Wrong Answer\n");
#if defined(_WIN32)
			MessageBox(nullptr,msg.c_str(),"Oops",MB_ICONERROR);
#elif defined(__linux__)
			bp::system(bp::search_path("zenity"),"--error","--text="+msg);
#endif
			break;
		}
		if(force_quit) // could be also enterd by goto
		{
			bad: if(force_quit)
			{
				std::fprintf(stderr,"Quit testing? (y/n): ");
				std::string s;
				std::getline(std::cin,s);
				if(s=="y" || s=="Y") break;
				force_quit=false;
				std::signal(SIGINT,[](int x){ tryQuit(); });
			}
			else break;
		}
	}
	delete gen_run;
	delete pro_run;
	delete std_run;
	delete chk_run;
}

} // namespace Tester
