#include "core/options.h"
#include "core/info.h"
#include "core/tester.h"
#include "core/cleaner.h"

int main(int argc,char *argv[])
{
	parseOptions(argc,argv);
	std::vector<const char*> vec(argv+optind,argv+argc);
	switch(opt.run_type)
	{
	 case Options::VERSION:
		Info::displayVersion();
		break;
	 case Options::HELP:
		Info::displayHelp(vec);
		break;
	 case Options::TEST:
		Tester::main(vec);
		break;
	 case Options::CLEAN:
		Cleaner::main(vec);
		break;
	 default:
		quitError("Invalid usage. Use 'oit-stress --help' to get help.");
	}
	return 0;
}