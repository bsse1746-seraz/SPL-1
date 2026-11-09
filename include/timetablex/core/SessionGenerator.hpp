#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

std::vector<Session> generateRequiredSessions(const Problem &problem);

} // namespace timetablex
