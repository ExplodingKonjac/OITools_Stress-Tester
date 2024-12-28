#pragma once

#include "config.hpp"

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

#include <string>

class Info
{
 private:
	int argc;
	char **argv;
	
	std::string filter(const std::string &s);

 public:
	Info(int _argc,char *_argv[]);
	std::string usageText();
	std::string helpGenericText();
	std::string helpTestText();
	std::string helpCleanText();
	std::string helpCheckersText();
	std::string shortHintText();
	std::string versionInfo();
};
