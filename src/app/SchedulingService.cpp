#include "timetablex/app/SchedulingService.hpp"

#include "timetablex/core/Scheduler.hpp"
#include "timetablex/core/Validator.hpp"

#include <stdexcept>

namespace timetablex
{

ScheduleResult SchedulingService::generate(const Problem &problem) const
{
    Scheduler scheduler(problem);
    if (!scheduler.generate())
        return {false, {}, scheduler.attemptCount(), scheduler.backtrackCount()};

    if (!validate(problem, scheduler.placements()))
        throw std::runtime_error("Internal validation failed.");

    return {true, scheduler.placements(), scheduler.attemptCount(), scheduler.backtrackCount()};
}

} // namespace timetablex
