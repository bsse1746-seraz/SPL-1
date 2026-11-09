#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

struct ScheduleResult
{
    bool found = false;
    std::vector<Placement> placements;
    long long attempts = 0;
    long long backtracks = 0;
};

class SchedulingService
{
public:
    // Requires a prepared problem. A successful result has passed independent validation.
    ScheduleResult generate(const Problem &problem) const;
};

} // namespace timetablex
