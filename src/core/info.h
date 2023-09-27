#pragma once

#include "lib.hpp"

namespace Info
{

void printColored(std::string_view text);
void displayVersion();
void displayHelp(const std::vector<const char*> &args);

} // namespace Info
