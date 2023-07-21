#include "tester.h"

namespace Tester
{

void compileFiles()
{
	namespace bp=boost::process;
	namespace asio=boost::asio;
	using Handler=std::function<void(const boost::system::error_code&,std::size_t)>;

	std::ofstream fout("compile.log");
	std::mutex locker;
	auto compileOne=[&](const std::string &name)
	{
		asio::io_context ios;
		bp::async_pipe ap(ios);
		bp::child proc(
			"g++ "+name+".cpp -o "+name+" "+opt.compile_opt,
			bp::std_err>ap,
			ios,
			bp::on_exit=[&](auto...){ ap.close(); }
		);
		std::string msg;
		std::vector<char> buf(512);
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
	if(opt.compile_gen) vec.emplace_back(compileOne,opt.gen_name);
	if(opt.compile_chk) vec.emplace_back(compileOne,opt.chk_name);
	vec.emplace_back(compileOne,opt.pro_name);
	vec.emplace_back(compileOne,opt.std_name);
	for(auto &i: vec) i.join();
	fout.close();
}

int checkResult(Runner *run,bool ignore_re=false)
{
	auto &res=run->getLastResult();
	auto name=run->getName().c_str();
	switch(res.type)
	{
	 case RunnerResult::TLE:
		printColor(TextAttr::fg_yellow,"%s Time Limit Exceeded\n",name);
		return 1;
	 case RunnerResult::MLE:
	 	printColor(TextAttr::fg_yellow,"%s Memory Limit Exceeded\n",name);
		return 2;
	 case RunnerResult::RE:
		if(ignore_re) break;
		printColor(TextAttr::fg_red,"%s Runtime Error (%u)\n",name,res.exit_code);
		return 3;
	 case RunnerResult::KILLED:
		printColor(TextAttr::fg_purple,"Terminated\n");
		return -1;
	 case RunnerResult::OK:
		break;
	}
	return 0;
}

void main(const std::vector<const char*> &args)
{
	if(args.size()>1)
		printMessage("Redundant arguments ignored.");
	if(args.empty())
		quitError("Missing testee code.");
	opt.pro_name=args[0];

	compileFiles();
	std::fputc('\n',stderr);

	Runner *gen_run=new Runner("Generator",opt.gen_name,opt.tl_gen,opt.ml_gen),
		   *chk_run=new Runner("Checker",opt.chk_name,opt.tl_chk,opt.ml_chk),
		   *pro_run=new Runner("Testee",opt.pro_name,opt.tl,opt.ml),
		   *std_run=new Runner("Standard",opt.std_name,opt.tl,opt.ml);
	gen_run->setOutputFile(opt.file+".in");
	pro_run->setInputFile(opt.file+".in");
	std_run->setInputFile(opt.file+".in");
	pro_run->setOutputFile(opt.file+".out");
	std_run->setOutputFile(opt.file+".ans");
	chk_run->setErrorFile(opt.file+".log");

	volatile bool force_quit=false;
	std::mutex mut;
	std::unique_lock<std::mutex> lk(mut);
	std::condition_variable cv;

	static std::function<void(int)> tryQuit;
	static auto p_tryQuit=[](int signum){ tryQuit(signum); };
	tryQuit=[&](int signum)
	{
		force_quit=true;
		pro_run->terminate();
		std_run->terminate();
		gen_run->terminate();
		chk_run->terminate();
		cv.wait(lk);
		std::string s;
		std::fprintf(stderr,"Quit testing? (y/n): ");
		std::getline(std::cin,s);
		force_quit=(s=="y" || s=="Y");
		cv.notify_all();
		std::signal(SIGINT,p_tryQuit);
	};
	std::signal(SIGINT,p_tryQuit);

	std::string chk_argu(opt.file+".in "+opt.file+".out "+opt.file+".ans");
	for(SIZE_T id=0;id<opt.test_cnt;id++)
	{
		std::fprintf(stderr,"Testcase #%llu: ",id);

		gen_run->start();
		gen_run->wait();
		if(checkResult(gen_run)) goto bad;
		pro_run->start();
		std_run->start();
		pro_run->wait();
		if(checkResult(pro_run)) goto bad;
		std_run->wait();
		if(checkResult(std_run)) goto bad;
		chk_run->start(chk_argu);
		chk_run->wait();
		if(checkResult(chk_run,true)) goto bad;

		if(false)
		{
			bad: if(force_quit)
			{
				cv.notify_all();
				cv.wait(lk);
				if(force_quit) break;
			}
			else break;
		}

		if(chk_run->getLastResult().type==RunnerResult::RE)
		{
			std::ifstream inf(opt.file+".log");
			std::vector<char> buf(256);
			inf.read(buf.data(),buf.size());
			auto siz=inf.gcount();
			std::string msg(buf.data(),buf.data()+siz);
			if(siz==256) msg+="...";
			msg="Failed on testcase #"+std::to_string(id)+" (256 bytes only):\n"+msg+'\n'+
				"See "+opt.file+".log for detail.";
			printColor(TextAttr::fg_red|TextAttr::intensity,"Wrong Answer\n");
			MessageBox(nullptr,msg.c_str(),"Oops",MB_ICONERROR);
			break;
		}
		printColor(TextAttr::fg_green|TextAttr::intensity,"Accepted\n");
	}
	delete gen_run;
	delete pro_run;
	delete std_run;
	delete chk_run;
}

} // namespace Tester
