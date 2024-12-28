#include "info.h"

Info::Info(int _argc,char *_argv[]):
	argc(_argc),argv(_argv)
{}

std::string Info::filter(const std::string &s)
{
	std::string res=s;
	// program name
	auto name=boost::filesystem::path(argv[0]).filename().string();
	res=boost::regex_replace(res,boost::regex("\\$\\{program\\}"),name);
	// variables
	res=boost::regex_replace(res,boost::regex("<([A-Z^<>]+)>"),"\e[4m$1\e[24m");
	// bold
	res=boost::regex_replace(res,boost::regex("\\*\\*([^\\*]+)\\*\\*"),"\e[1m$1\e[22m");
	// italic
	res=boost::regex_replace(res,boost::regex("\\*([^\\*]+)\\*"),"\e[3m$1\e[23m");
  // strings
  res=boost::regex_replace(res,boost::regex("'([^`']+)'"),"'\e[38;5;7m$1\e[39m'");
	// command or code
	res=boost::regex_replace(res,boost::regex("`([^`]+)`"),"`$1'");

	return res;
}

std::string Info::usageText()
{
	constexpr char text[]=
R"(Usage: ${program} {test|clean} [<OPTION>...] [<ARGS>...]
  or:  ${program} --help[={test|clean|checkers}]
)";
	return filter(text);
}

std::string Info::helpGenericText()
{
	constexpr char text[]=
R"(
Usage: ${program} {test|clean} [<OPTION>...] [<ARGS>...]
  or:  ${program} --help[={test|clean|checkers}]

 Generic options:
      **--usage**                Give a short usage message.

  **-h**, **--help**[={test|clean|checkers}]
                             Display help message of a specified section, or
                             this information if no arguments are provided.
  **-V**, **--version**              Print version information.

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
)";
	return filter(text);
}

std::string Info::helpTestText()
{
	constexpr char text[]=
R"(Usage: ${program} test [<OPTION>...] <CODE>
Perform stress test on *<CODE>.cpp*.

 **Options for subcommand `test`**:
      **--compile-chk**[=<BOOLEAN>]   Whether to compile *<CHK>.cpp*. Default value is
                             'false'. It is set to 'true' if no argument
                             provided.
      **--compile-gen**[=<BOOLEAN>]   Whether to compile *<GEN>.cpp*. Default value is
                             'true'. It is set to 'true' if no argument
                             provided.
  **-c**, **--chk**=<CHK>              Set the checker to <CHK>, and checker code to
                             *<CHK>.cpp*. Default value is 'chk'.
  **-f**, **--file**=<FILE>            Set the filename of the files produced to
                             *<FILE>.in/out/ans/log*. Default value is 'data'.
  **-g**, **--gen**=<GEN>              Set the generator to <GEN>, and generator code to
                             *<GEN>.cpp*. Default value is 'gen'.
  **-j**, **--jobs**=<NUM>             Set the number of threads used in testing. Default
                             value is '1'.
      **--ml-chk**=<ML>            Set the memory limit for checker to <ML> MB. Default
                             value is '2048'.
      **--ml-gen**=<ML>            Set the memory limit for generator to <ML> MB.
                             Default value is '2048'.
  **-M**, **--ml**=<ML>                Set the memory limit for testee and standard code
                             to <ML> MB. Default value is '512'.
  **-n**, **--tests**={<NUM>|infinity} Set the number of testcases to <NUM> or 'infinity'.
                             Default value is 'infinity'.
  **-s**, **--std**=<STD>              Set the standard code to *<STD>.cpp*. Default value is
                             'std'.
      **--tl-chk**=<TL>            Set the time limit for checker to <TL> ms. Default
                             value is '5000'.
      **--tl-gen**=<TL>            Set the time limit for generator to <TL> ms. Default
                             value is '5000'.
  **-T**, **--tl**=<TL>                Set the time limit for testee and standard code to
                             <TL> ms. Default value is '1000'.
  **-Wc**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to compiler when compiling checker
                             code. Default value is '-std=c++20,-O2'.
  **-We**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to compiler when compiling
                             testee/standard code. Default value is
                             '-std=c++20,-O2'.
  **-Wg**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to compiler when compiling
                             generator code. Default value is '-std=c++20,-O2'.
  **-Xc**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to checker. By default it's empty.
  **-Xe**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to testee/standard program. By
                             default it's empty.
  **-Xg**,<ARGS>                   Comma seperated extra arguments or options that
                             will be passed to generator. By default it's
                             empty.

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

**About testee/standard**:
  Testee/Standard program should read from <STDIN> and write into <STDOUT>. Their
  <STDIN> and <STDOUT> will be automatically redirected to some files, of which
  filenames are not guaranteed, so DON'T read from or write into any files.

**About generator**:
  Generator should write into STDOUT, which will be redirected to the input
  file. You can pass extra arguments or options to the generator through **-Xg**.

**About checker**:
  Checker code is recommended to be written with testlib. See
  <https://github.com/MikeMirzayanov/testlib/> for details. Otherwise you should
  make your checker able to be run in the format:
    `chk <INPUT> <OUTPUT> <ANSWER> [<OPTION>...]`
  and return 0 when the output is accepted and non-zero otherwise. <STDERR> of
  checker will be redirected to the log file. You can pass extra arguments or
  options to the checker through **-Xc**.
)";
	return filter(text);
}

std::string Info::helpCleanText()
{
	constexpr char text[]=
R"(Usage: ${program} clean [<OPTION>...]
Clean the file produced during testing.

 Options for subcommand `clean`:
  **-f**, **--file**=<FILE>            *<FILE>.in/out/ans/log* and *compile.log* will be
                             cleaned. Default value is 'data'.

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
)";
	return filter(text);
}

std::string Info::helpCheckersText()
{
	constexpr char text[]=
R"(A number of built-in checkers are provided. Note that the directory where
built-in checkers are installed should be added to PATH, so that they could be
recognized.

 Built-in checkers:
  **chk-caseicmp**               Compare **int64** numbers with testcases.
  **chk-casencmp**               Compare **int64** sequences with testcases.
  **chk-casewcmp**               Compare sequences of tokens with testcases.
  **chk-dcmp**                   Compare **double**s (1e-6 absolute or relative error
                             allowed).
  **chk-fcmp**                   Compare as sequences of lines.
  **chk-hcmp**                   Compare signed huge integers.
  **chk-icmp**                   Compare **int** numbers.
  **chk-lcmp**                   Compare **long** int numbers.
  **chk-ncmp**                   Compare sequences of **long long** numbers.
  **chk-nyesno**                 Compare two sequences of 'YES'/'NO's (case
                             insensitive).
  **chk-rcmp**                   Compare **double**s (1.5e-6 absolute error allowed).
  **chk-rcmp4**                  Compare sequences of **double**s (1e-4 absolute or
                             relative error allowed).
  **chk-rcmp6**                  Compare sequences of **double**s (1e-6 absolute or
                             relative error allowed).
  **chk-rcmp9**                  Compare sequences of **double**s (1e-9 absolute or
                             relative error allowed).
  **chk-rncmp**                  Compare sequences of **double**s (1.5e-5 absolute
                             error allowed).
  **chk-uncmp**                  Compare unordered sequences of **long long** numbers.
  **chk-wcmp**                   Compare sequences of tokens (it can handle most
                             situations).
  **chk-yesno**                  Compare 'YES'/'NO's (case insensitive).

See <https://github.com/MikeMirzayanov/testlib/tree/master/checkers> for
details.
)";
	return filter(text);
}

std::string Info::shortHintText()
{
	constexpr char text[]=
R"(Try `${program} --help` or `${program} --usage` for more information.
)";
	return filter(text);
}

std::string Info::versionInfo()
{
	constexpr char text[]=
R"(${program} )" OIT_STRESS_VERSION "\n"
COPYRIGHT R"(

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
)";
	return filter(text);
}