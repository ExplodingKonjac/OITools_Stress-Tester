#include "cleaner.h"

namespace fs=boost::filesystem;

void Cleaner::tryDelete(const std::string &name)
{
	boost::system::error_code ec;
	fs::remove(name,ec);
	if(ec)
		msg.error("failed to remove file {0} ({1}): {2}",name,ec.value(),ec.message());
	else
		msg.print("successfully removed file {0}\n",name);
}

void Cleaner::start()
{
	tryDelete("compile.log");
	tryDelete(opt.file+".in");
	tryDelete(opt.file+".out");
	tryDelete(opt.file+".ans");
	tryDelete(opt.file+".log");
}
