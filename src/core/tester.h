#pragma once

#include "options.h"
#include "runner.h"
#include <boost/process.hpp>
#include <boost/asio.hpp>

namespace Tester
{

void compileFiles();
int checkResult(Runner *run,bool ignore_re);
void main(const std::vector<const char*> &vec);

} // namespace Tester
