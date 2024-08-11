#pragma once

#include "options.h"
#include "runner.h"
#include <boost/process.hpp>
#include <boost/asio.hpp>

namespace Tester
{

void main(const std::vector<const char*> &vec);

} // namespace Tester
