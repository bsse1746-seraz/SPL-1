#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

bool validate(const Problem &p, const std::vector<Placement> &placements);

} // namespace timetablex
