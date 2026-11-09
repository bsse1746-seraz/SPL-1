#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

std::vector<Session> generateRequiredSessions(const std::vector<CourseRequirement> &courses,
                                              const CreditPolicy &policy);

} // namespace timetablex
