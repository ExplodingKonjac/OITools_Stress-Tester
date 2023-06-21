#pragma once
#include "core.h"
#include "runner.hpp"

namespace Tester
{

void compileFiles();
int checkResult(Runner *run,bool ignore_re);
void main();

} // namespace Tester
