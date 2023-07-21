#pragma once

#include "options.h"
#include <filesystem>

namespace Cleaner
{

void tryDelete(const std::string &name);
void main(const std::vector<const char*> &args);

} // namespace Cleaner
