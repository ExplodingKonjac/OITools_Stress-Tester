#pragma once
#include "core.h"

namespace Helper
{

inline constexpr char help_text[]=R"(
Usage: stress <module_name> [options...]

Modules and their options:
NAME                             DEFAULT VALUE   DESCRIPTION
* stress help                    /               Get help.
* stress test name [options...]  /               Do stress test for <name>.cpp.
  * -file=<name>                 data            Set the name of files as <name>.in/out/ans/log.
  * -std=<name>                  std             Set the standard code as <name>.cpp.
  * -gen=<name>                  gen             Set the generator code as <name>.cpp.
  * -chk=<name>                  chk             Set the checker code as <name>.cpp.
  * -tl=<time>                   1000            Set the time limit as <time>ms.
  * -ml=<memory>                 1024            Set the memory limit as <memoty>MB.
  * -tl-gen=<time>               5000            Set the time limit for Generator as <time>ms.
  * -ml-gen=<memory>             2048            Set the memory limit for Generator as <memoty>MB.
  * -tl-chk=<time>               5000            Set the time limit for Checker as <time>ms.
  * -ml-chk=<memory>             2048            Set the memory limit for Checker as <memoty>MB.
  * -opt="<options>"             -std=c++17 -O2  Set the compiling options as <options>.
  * -cnt=<number/unlimited>      unlimited       Set the number of tests as <number>.
  * -cgen=<boolean>              true            Controls whether Generator will be compiled.
  * -cchk=<boolean>              false           Controls whether Checker will be compiled.
* stress clean [options...]      /               Clean the file produced during testin.
  * -file=<name>                 data            Set the name of files to clean as <name>.in/out/ans/log.

Built-in Checkers:
NAME                             DESCRIPTION
* chk-caseicmp                   Compare single `int64` with testcases.
* chk-casencmp                   Compare `int64`s with testcases.
* chk-casewcmp                   Compare tokens with testcases.
* chk-dcmp                       Compare `double`s (1e-6 relative).
* chk-fcmp                       Compare lines.
* chk-hcmp                       Compare signed huge integers.
* chk-icmp                       Compare single `int`.
* chk-lcmp                       Compare single `long int`.
* chk-ncmp                       Compare `long long`s.
* chk-nyesno                     Compare YES/NOs (case insensitive).
* chk-rcmp                       Compare single `double` (1.5e-6 absolute).
* chk-rcmp4                      Compare `double`s (1e-4 relative).
* chk-rcmp6                      Compare `double`s (1e-6 relative).
* chk-rcmp9                      Compare `double`s (1e-9 relative).
* chk-rncmp                      Compare `double`s (1.5e-5 absolute).
* chk-uncmp                      Compare unordered `long long`s.
* chk-wcmp                       Compare tokens.
* chk-yesno                      Compare single YES/NO (case insensitive).
See <https://github.com/MikeMirzayanov/testlib/tree/master/checkers> for detail.
)";

void main();

} // namespace Helper
