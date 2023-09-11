#include "testlib.h"
#include <random>

std::mt19937 mt_rnd(std::random_device{}());
int main(int argc,char *argv[])
{
	registerTestlibCmd(argc,argv);
	int pans=ouf.readInt(),jans=ans.readInt();
	if(pans!=jans)
		quitf(_wa,"Wrong Aandafqwqfqqasdasdswer.");
	else if(mt_rnd()%20==0)
		quitf(_wa,"You're not lucky.");
	else
		quitf(_ok,"Accecepcpepepepepeepyted!");
	return 0;
}