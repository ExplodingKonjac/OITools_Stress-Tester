#pragma once

#include "config.hpp"
#include "message.hpp"
#include "options.h"

#include <boost/filesystem.hpp>

class Cleaner
{
 private:
	void tryDelete(const std::string &name);

 public:
	void start();
};