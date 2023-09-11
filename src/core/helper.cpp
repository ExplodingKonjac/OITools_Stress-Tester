#include "helper.h"

namespace Helper
{

void printHelp(std::string_view text)
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

static constexpr char help_general[]=R"(
Usage: stress $MODULE_NAME$ [$MODULE_ARGUMENTS$]... [$OPTIONS$]...

$MODULE_NAME$ can be @help@|@test@|@clean@.

run the following command to get further help:
    stress help [@general@|@test@|@clean@|@checkers@]
)",
help_test[]=R"(
Usage: stress test $NAME$ [$OPTIONS$]...
Do testing for $NAME$.cpp.

The following options are available:
    -f $FILE$, --file=$FILE$
        Set the name of file produced as $FILE$.in/out/ans/log;
    -s $STD$, --std=$STD$
        Set the standard code as $STD$.cpp;
    -g $GEN$, --gen=$GEN$
        Set the generator code as $GEN$.cpp;
    -c $CHK$, --chk=$CHK$
        Set the checker code as $CHK$.cpp;
    -T $TL$, --time-limit=$TL$
        Set the time limit for testee code as $TL$ ms;
    -M $ML$, --memory-limit=$ML$
        Set the memory limit for testee code as $ML$Mb;
    --tl-gen=$TL$
        Set the memory limit for generator as $TL$ms;
    --ml-gen=$ML$
        Set the memory limit for generator as $ML$Mb;
    --tl-chk=$TL$
        Set the memory limit for checker as $TL$ms;
    --ml-chk=$ML$
        Set the memory limit for checker as $ML$Mb;
    --compile-opt=$OPT$
        Set the compile options for testee code and standard code;
    --count=$NUMBER$|@infinite@
        Set the count of tests as $NUMBER$ (or infinite);
    --compile-gen[=@true@|@false@]
        Whether to compile generator, true if no argument provided;
    --compile-chk[=@true@|@false@]
        Whether to compile checker, true if no argument provided;

About testee/standard:
    STDIN of testee/standard will be redirected from $FILE$.in.
    STDOUT of testee will be redirected into $FILE$.out and STDOUT
    of standard will be redirected into $FILE$.ans. Reading/Writing
    data directly from/into $FILE$.in/out/ans is OK too.

About generator:
    STDOUT of the generator will be redirect to $FILE$.in. Writing
    the data directly into $FILE$.in is OK too.

About checker:
    Checker code is recommended to be written with testlib.h. See
    <https://github.com/MikeMirzayanov/testlib/> for details of
    testlib.h. Otherwise you should make your checker be able to
    run in the format './chk $INPUT_FILE$ $OUTPUT_FILE$ $ANSWER_FILE$',
    and return 0 when the answer is accepted and non-zero otherwise.
    STDERR of the checker will be redirected into $FILE$.log.
)",
help_clean[]=R"(
Usage: stress clean [$OPTIONS$]...
Clean the file produced during testing.

The following options are available:
    -f $FILE$, --file=$FILE$
        $FILE$.in/out/ans/log will be cleaned.
)",
help_checkers[]=R"(
Here are all built-in checkers and you should add them into PATH:

    chk-caseicmp
        Compare single `int64` with testcases.
    chk-casencmp
        Compare `int64`s with testcases.
    chk-casewcmp
        Compare tokens with testcases.
    chk-dcmp
        Compare `double`s (1e-6 relative).
    chk-fcmp
        Compare lines.
    chk-hcmp    
        Compare signed huge integers.
    chk-icmp    
        Compare single `int`.
    chk-lcmp
        Compare single `long int`.
    chk-ncmp
        Compare `long long`s.
    chk-nyesno
        Compare YES/NOs (case insensitive).
    chk-rcmp
        Compare single `double` (1.5e-6 absolute).
    chk-rcmp4
        Compare `double`s (1e-4 relative).
    chk-rcmp6
        Compare `double`s (1e-6 relative).
    chk-rcmp9
        Compare `double`s (1e-9 relative).
    chk-rncmp
        Compare `double`s (1.5e-5 absolute).
    chk-uncmp
        Compare unordered `long long`s.
    chk-wcmp
        Compare tokens.
    chk-yesno
        Compare single YES/NO (case insensitive).

See <https://github.com/MikeMirzayanov/testlib/tree/master/checkers>
for detail of these checkers.
)";

void main(const std::vector<const char*> &args)
{
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
	printHelp(text);
}

} // namespace Helper
