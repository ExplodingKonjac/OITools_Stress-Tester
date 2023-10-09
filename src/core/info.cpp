#include "info.h"

namespace Info
{

void printColored(std::string_view text)
{
	bool is_var=false,is_str=false,is_type=false;
	std::size_t pos=0;
	while(pos<text.size())
	{
		auto nxt=text.find_first_of("$@`",pos);
        TextAttr attr=TextAttr::tg_stdout;
		if(is_var)
			attr|=TextAttr::fg_cyan;
		else if(is_str)
			attr|=TextAttr::fg_yellow;
		else if(is_type)
			attr|=TextAttr::fg_purple|TextAttr::intensity;
		else
			attr|=TextAttr::plain;
        setTextAttr(attr);
		std::cout<<text.substr(pos,nxt-pos);
		if(nxt==text.npos) break;
		pos=nxt+1;
		switch(text[nxt])
		{
		 case '$': is_var^=1;break;
		 case '@': is_str^=1;break;
		 case '`': is_type^=1;break;
		 default: break;
		}
	}
	setTextAttr(TextAttr::plain|TextAttr::tg_stdout);
}

void displayVersion()
{
	constexpr char version_info[]=
	"oit-stress 1.3.1\n"
	"Copyright (C) 2023 ExplodingKonjac\n"
	"\n"
	"This program is free software: you can redistribute it and/or modify\n"
	"it under the terms of the GNU General Public License as published by\n"
	"the Free Software Foundation, either version 3 of the License, or\n"
	"(at your option) any later version.\n"
	"\n"
	"This program is distributed in the hope that it will be useful,\n"
	"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	"GNU General Public License for more details.\n"
	"\n"
	"You should have received a copy of the GNU General Public License\n"
	"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n";
	std::cout<<version_info;
}

void displayHelp(const std::vector<const char*> &args)
{
	constexpr char help_general[]=
	"Usage: stress $RUN_TYPE$ [$MODULE_ARGUMENTS$]... [$OPTIONS$]...\n"
	"\n"
	"$RUN_TYPE$ can be @--version@|@--help@|@--test@|@--clean@.\n"
	"\n"
	"run the following command to get further help:\n"
	"  oit-stress --help [@general@|@test@|@clean@|@checkers@]\n"
	"\n",
	help_test[]=
	"Usage: stress --test $NAME$ [$OPTIONS$]...\n"
	"Do testing for $NAME$.cpp.\n"
	"\n"
	"The following options are available:\n"
	"  -f $FILE$, --file=$FILE$\n"
	"    Set the name of file produced as $FILE$.in/out/ans/log;\n"
	"  -s $STD$, --std=$STD$\n"
	"    Set the standard code as $STD$.cpp;\n"
	"  -g $GEN$, --gen=$GEN$\n"
	"    Set the generator code as $GEN$.cpp;\n"
	"  -c $CHK$, --chk=$CHK$\n"
	"    Set the checker code as $CHK$.cpp;\n"
	"  -T $TL$, --time-limit=$TL$\n"
	"    Set the time limit for testee code as $TL$ms;\n"
	"  -M $ML$, --memory-limit=$ML$\n"
	"    Set the memory limit for testee code as $ML$Mb;\n"
	"  --tl-gen=$TL$\n"
	"    Set the memory limit for generator as $TL$ms;\n"
	"  --ml-gen=$ML$\n"
	"    Set the memory limit for generator as $ML$Mb;\n"
	"  --tl-chk=$TL$\n"
	"    Set the memory limit for checker as $TL$ms;\n"
	"  --ml-chk=$ML$\n"
	"    Set the memory limit for checker as $ML$Mb;\n"
	"  --compile-opt=$OPT$\n"
	"    Set the compile options for testee code and standard code;\n"
	"  --count=$NUMBER$|@infinite@\n"
	"    Set the count of tests as $NUMBER$ (or infinite);\n"
	"  --compile-gen[=@true@|@false@]\n"
	"    Whether to compile generator, true if no argument provided;\n"
	"  --compile-chk[=@true@|@false@]\n"
	"    Whether to compile checker, true if no argument provided;\n"
	"\n"
	"About testee/standard:\n"
	"  STDIN of testee/standard will be redirected from $FILE$.in.\n"
	"  STDOUT of testee will be redirected into $FILE$.out and STDOUT\n"
	"  of standard will be redirected into $FILE$.ans. Reading/Writing\n"
	"  data directly from/into $FILE$.in/out/ans is OK too.\n"
	"\n"
	"About generator:\n"
	"  STDOUT of the generator will be redirect to $FILE$.in. Writing\n"
	"  the data directly into $FILE$.in is OK too.\n"
	"\n"
	"About checker:\n"
	"  Checker code is recommended to be written with testlib.h. See\n"
	"  <https://github.com/MikeMirzayanov/testlib/> for details of\n"
	"  testlib.h. Otherwise you should make your checker be able to\n"
	"  run in the format './chk $INPUT_FILE$ $OUTPUT_FILE$ $ANSWER_FILE$',\n"
	"  and return 0 when the answer is accepted and non-zero otherwise.\n"
	"  STDERR of the checker will be redirected into $FILE$.log.\n"
	"\n",
	help_clean[]=
	"Usage: stress --clean [$OPTIONS$]...\n"
	"Clean the file produced during testing.\n"
	"\n"
	"The following options are available:\n"
	"  -f $FILE$, --file=$FILE$\n"
	"    $FILE$.in/out/ans/log will be cleaned.\n"
	"\n",
	help_checkers[]=
	"Here are all built-in checkers and you should add them into PATH:\n"
	"\n"
	"  chk-caseicmp\n"
	"    Compare single `int64` with testcases.\n"
	"  chk-casencmp\n"
	"    Compare `int64`s with testcases.\n"
	"  chk-casewcmp\n"
	"    Compare tokens with testcases.\n"
	"  chk-dcmp\n"
	"    Compare `double`s (1e-6 relative).\n"
	"  chk-fcmp\n"
	"    Compare lines.\n"
	"  chk-hcmp  \n"
	"    Compare signed huge integers.\n"
	"  chk-icmp  \n"
	"    Compare single `int`.\n"
	"  chk-lcmp\n"
	"    Compare single `long int`.\n"
	"  chk-ncmp\n"
	"    Compare `long long`s.\n"
	"  chk-nyesno\n"
	"    Compare YES/NOs (case insensitive).\n"
	"  chk-rcmp\n"
	"    Compare single `double` (1.5e-6 absolute).\n"
	"  chk-rcmp4\n"
	"    Compare `double`s (1e-4 relative).\n"
	"  chk-rcmp6\n"
	"    Compare `double`s (1e-6 relative).\n"
	"  chk-rcmp9\n"
	"    Compare `double`s (1e-9 relative).\n"
	"  chk-rncmp\n"
	"    Compare `double`s (1.5e-5 absolute).\n"
	"  chk-uncmp\n"
	"    Compare unordered `long long`s.\n"
	"  chk-wcmp\n"
	"    Compare tokens.\n"
	"  chk-yesno\n"
	"    Compare single YES/NO (case insensitive).\n"
	"\n"
	"See <https://github.com/MikeMirzayanov/testlib/tree/master/checkers>\n"
	"for detail of these checkers.\n"
	"\n";

	const char *type="general";
	if(args.size()>1)
		printMessage("Redundant arguments ignored.");
	if(!args.empty())
		type=args[0];
	const char *text;
	switch(strhash(type))
	{
	 case strhash("general"):
	 	text=help_general;
		break;
	 case strhash("test"):
	 	text=help_test;
		break;
	 case strhash("checkers"):
		text=help_checkers;
		break;
     case strhash("clean"):
        text=help_clean;
	 default:
		quitError("Unknown help type '%s'.",type);
	}
	printColored(text);
}

} // namespace Info
