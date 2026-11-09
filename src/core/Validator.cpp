#include "timetablex/core/Validator.hpp"

#include "timetablex/core/SchedulingRules.hpp"

#include <cstddef>

namespace timetablex
{

using std::size_t;
using std::vector;

// Independent completed-schedule check: never calls Scheduler::canPlace().
bool validate(const Problem &p, const vector<Placement> &a)
{
    if (a.size() != p.sessions.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        const Session &s = p.sessions[i];
        if (s.teacher < 0 || s.teacher >= static_cast<int>(p.available.size()) ||
            s.batch < 0 || s.batch >= static_cast<int>(p.groups.size()) ||
            a[i].slot < 0 || a[i].slot >= SLOTS_PER_WEEK ||
            a[i].room < 0 || a[i].room >= static_cast<int>(p.rooms.size()))
            return false;

        // Independently recheck room suitability and capacity.
        // Do not rely on Scheduler::canPlace().
        if (!roomSupportsSession(p.rooms[a[i].room], s) ||
            p.groups[s.batch].size <= 0 || p.rooms[a[i].room].capacity <= 0 ||
            p.groups[s.batch].size > p.rooms[a[i].room].capacity)
            return false;

        int day = a[i].slot / UNITS_PER_DAY;
        int start = a[i].slot % UNITS_PER_DAY;
        if (day >= DAYS_PER_WEEK ||
            !fitsAcademicWindow(start, s.durationUnits, s.isLab))
            return false;

        if (p.available[s.teacher].size() < static_cast<size_t>(SLOTS_PER_WEEK))
            return false;
        for (int u = 0; u < s.durationUnits; ++u)
            if (!p.available[s.teacher][a[i].slot + u])
                return false;

        for (size_t j = 0; j < i; ++j)
        {
            if (a[j].slot < 0 || a[j].slot / UNITS_PER_DAY != day)
                continue;

            const Session &other = p.sessions[j];
            int otherStart = a[j].slot % UNITS_PER_DAY;
            if (intervalsOverlap(start, s.durationUnits,
                                 otherStart, other.durationUnits) &&
                (a[i].room == a[j].room ||
                 s.teacher == other.teacher ||
                 s.batch == other.batch))
                return false;
        }
    }
    return true;
}

} // namespace timetablex
