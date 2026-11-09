#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

Problem makeDemoResources();
Problem makeDemoProblem();
Problem loadDemoProblem(const std::string &path);

} // namespace timetablex
