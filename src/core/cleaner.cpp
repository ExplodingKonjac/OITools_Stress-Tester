#include "cleaner.h"

namespace Cleaner
{

void tryDelete(const std::string &name)
{
	namespace fs=std::filesystem;

	std::error_code ec;
	bool ret=fs::remove(name,ec);
	if(ret) printMessage("Successfully deleted file %s.",name.c_str());
	else printMessage("Failed to delete file %s (%d).",name.c_str(),ec.value());
}
void main(const std::vector<const char*> &args)
{
	if(args.size()>1)
		printMessage("Redundant arguments ignored.");
	tryDelete("compile.log");
	tryDelete(opt.file+".in");
	tryDelete(opt.file+".out");
	tryDelete(opt.file+".ans");
	tryDelete(opt.file+".log");
}

} // namespace Cleaner
