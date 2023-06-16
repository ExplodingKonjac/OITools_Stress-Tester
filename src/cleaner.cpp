#include "cleaner.h"

namespace Cleaner
{

void tryDelete(const std::string &name)
{
	bool ret=DeleteFile(name.c_str());
	if(ret) printMessage("Successfully deleted file %s.",name.c_str());
	else printMessage("Failed to delete file %s (%lu).",name.c_str(),GetLastError());
}
void main()
{
	tryDelete("compile.log");
	tryDelete(opt.file+".in");
	tryDelete(opt.file+".out");
	tryDelete(opt.file+".ans");
	tryDelete(opt.file+".log");
}

} // namespace Cleaner
