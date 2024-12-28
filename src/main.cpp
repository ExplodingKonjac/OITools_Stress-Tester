#include "core/options.h"
#include "core/tester.h"
#include "core/cleaner.h"

int main(int argc,char *argv[])
{
	try
	{
		OptionParser parser(argc,argv);
		if(!parser.parse(opt) || opt.args.empty())
			return 0;
		if(opt.args[0]=="test")
		{
			if(opt.args.size()<2)
				throw std::runtime_error("missing CODE for subcommand `test'");
			if(opt.args.size()>2)
				msg.warning("redundant arguments ignored");
			opt.exe_name=opt.args[1];

			Tester{}.start();
		}
		else if(opt.args[0]=="clean")
		{
			if(opt.args.size()>1)
				msg.warning("redundant arguments ignored");

			Cleaner{}.start();
		}
		else msg.error("unknown subcommand `{0}'",opt.args[0]);
	}
	catch(const std::runtime_error &e)
	{
		msg.fatal("{0}",e.what());
		return 1;
	}
	return 0;
}