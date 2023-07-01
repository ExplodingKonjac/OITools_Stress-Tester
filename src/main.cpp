#include "core/options.h"
#include "core/helper.h"
#include "core/tester.h"
#include "core/cleaner.h"

int main(int argc,char *argv[])
{
	parseOptions(argc,argv);
	if(optind>=argc)
		quitError("Invalid usage. Use 'stress help' to get help.");
	std::vector<const char*> vec(argv+optind+1,argv+argc);
	switch(strhash(argv[optind]))
	{
	 case strhash("help"):
		Helper::main(vec);
		break;
	 case strhash("test"):
	 	Tester::main(vec);
		break;
	 case strhash("clean"):
	 	Cleaner::main(vec);
		break;
	 default:
		quitError("Invalid usage. Use 'stress help' to get help.");
	}
	return 0;
}