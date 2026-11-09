#pragma once

#include "timetablex/models/Problem.hpp"

#include <iosfwd>

namespace timetablex
{

std::vector<CourseRequirement> loadCoursesCSV(std::istream &input);

// Exports placements from a successfully validated schedule.
bool exportCSV(const Problem &problem, const std::vector<Placement> &placements,
               const std::string &path);

} // namespace timetablex
