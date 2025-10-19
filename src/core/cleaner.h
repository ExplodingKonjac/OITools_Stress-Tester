#pragma once

#include "base/config.hpp"
#include "base/message.hpp"
#include "options.h"

#include <boost/filesystem.hpp>

class Cleaner
{
 private:
	void tryDelete(const std::string &name);

 public:
	void start();
};