#include "tester_new.h"

namespace bp=boost::process::v2;
namespace as=boost::asio;
namespace fs=boost::filesystem;

Tester::Tester()
{}

void JudgingThread::judgingThread(int id,Judger &judger)
{
	while(!stop_flag && tot<opt.test_cnt)
	{
		JudgeResult res=judger.judge();
		std::unique_lock<std::mutex> lock(mtx_q);
		if(pause_flag)
		{
			cond_pause.wait(lock,[this] {
				return !pause_flag;
			});
			continue;
		}
		if(!stop_flag)
		{
			result_q.emplace(i,res);
			tot++;
			cond_q.notify_one();
			if(res.type!=JudgeResult::OK)
			{
				stop_flag=true;
				break;
			}
		}
	}
}

void Tester::compileOne(const std::string &filename,const std::vector<std::string> &extra_opt,std::mutex &mtx)
{
	std::vector<std::string> compiler_opt{filename+".cpp","-o",filename.string()};
	compiler_opt.insert(compiler_opt.end(),extra_opt.begin(),extra_opt.end());
	as::io_context io_context;
	as::streambuf output_buf;

	bp::process proc(
		io_context,
		bp::environment::find_executable("g++"),
		compiler_opt,
		bp::process_stdio{{},{},output_buf}
	);
	io_context.run();
	std::string output(as::buffers_begin(output_buf),as::buffers_end(output_buf));
	proc.wait();
	
	std::lock_guard lock(mtx);
}

void Tester::compileExecutables()
{
	std::vector<std::thread> threads;
	std::mutex mtx;

	threads.emplace(Tester::compileOne,opt.exe_name,opt.compiler_opt,mtx);
	threads.emplace(Tester::compileOne,opt.std_name,opt.compiler_opt,mtx);
	if(opt.compile_gen)
		threads.emplace(Tester::compileOne,opt.gen_name,{},mtx);
	if(opt.compile_chk)
		threads.emplace(Tester::compileOne,opt.chk_name,{},mtx);
	
	for(auto &t: threads)
		t.join();
}

void Tester::start()
{

	q=std::queue<JudgeResult>{};
	tot=0;
	stop_flag=false;
	pause_flag=false;

	std::vector<std::thread> threads;
	std::vector<Judger> judgers;

	fs::path exe_path=opt.exe_name,
			 std_path=opt.std_name,
			 gen_path=(opt.compile_gen?opt.gen_name:bp::environment::find_executable(opt.gen_name)),
			 chk_path=(opt.compile_gen?opt.gen_name:bp::environment::find_executable(opt.gen_name));
	for(std::size_t i=0;i<opt.thread_cnt;i++)
	{
		judgers.emplace_back(std::to_string(i),prefix,exe_path,std_path,gen_path,chk_path);
		threads.emplace_back(&Tester::testSingle,this,i,judgers.back());
	}

	std::size_t idx=0;
	std::cerr<<std::setprecision(2)<<std::fixed;
	while(!stop_flag && tot<opt.threads_cnt)
	{
		std::unique_lock<std::mutex> lock(mtx_q);
		cond_pause.wait(lock,[this] {
			return !result_q.empty();
		});
		auto [id,res]=q.front();
		q.pop();
		std::cerr<<"#"<<idx<<": ";
		switch(res.type)
		{
		 case JudgeResult::OK:
			std::cerr<<"Accepted.\n";
			break;

		 case JudgeResult::WA:
			std::cerr<<"Wrong Answer.\n";
			stop_flag=true;
			break;

		 case JudgeResult::EXE_ERR:
			std::cerr<<"Testee "
			goto finally;

		 case JudgeResult::STD_ERR:
			std::cerr<<"Standard "
			goto finally;

		 case JudgeResult::GEN_ERR:
			std::cerr<<"Generator "
			goto finally;

		 case JudgeResult::CHK_ERR:
			std::cerr<<"Checker "
			goto finally;
		
		 finally:
			switch (res.info.type)
			{
			 case ProcessInfo::TLE:
			 	std::cerr<<"time limit exceeded";
				if(res.info.time_used==-1sz)
					std::cerr<<" (killed)\n";
				else
					std::cerr<<" ("<<res.info.time_used<<"ms)\n";
				break;

			 case ProcessInfo::MLE:
				std::cerr<<"memory limit exceeded ("<<res.info.memory_used/1024.0/1024.0<<"MB)\n";
				break;

			 case ProcessInfo::RE:
				std::cerr<<"runtime error ("<<res.info.exit_code<<")\n"
				break;

			 default:
				std::cerr<<"unknown error\n"
			}
		}
	}

	for(auto &t: threads)
		t.join();
}
