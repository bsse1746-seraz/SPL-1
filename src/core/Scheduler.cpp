#include "timetablex/core/Scheduler.hpp"

#include "timetablex/core/SchedulingRules.hpp"

#include <cstddef>

namespace timetablex
{

bool Scheduler::canPlace(int i, int slot, int room) const
{
    const Session &s = problem_.sessions[i];
    if (s.teacher < 0 || s.teacher >= static_cast<int>(problem_.available.size()) ||
        s.batch < 0 || s.batch >= static_cast<int>(problem_.groups.size()) ||
        room < 0 || room >= static_cast<int>(problem_.rooms.size()) ||
        slot < 0 || slot >= SLOTS_PER_WEEK)
        return false;

    // HARD: correct room type AND enough seats for the entire group.
    if (!roomSupportsSession(problem_.rooms[room], s) ||
        problem_.groups[s.batch].size <= 0 || problem_.rooms[room].capacity <= 0 ||
        problem_.groups[s.batch].size > problem_.rooms[room].capacity)
        return false;

    int day = slot / UNITS_PER_DAY;
    int start = slot % UNITS_PER_DAY;
    if (day < 0 || day >= DAYS_PER_WEEK ||
        !fitsAcademicWindow(start, s.durationUnits, s.isLab))
        return false;

    // Check the teacher's availability at every occupied half-hour unit.
    if (problem_.available[s.teacher].size() < static_cast<std::size_t>(SLOTS_PER_WEEK))
        return false;
    for (int u = 0; u < s.durationUnits; ++u)
        if (!problem_.available[s.teacher][slot + u])
            return false;

    // Compare intervals, not just their starting times.
    for (int j = 0; j < i; ++j)
    {
        if (placements_[j].slot < 0 || placements_[j].slot / UNITS_PER_DAY != day)
            continue;

        const Session &other = problem_.sessions[j];
        int otherStart = placements_[j].slot % UNITS_PER_DAY;
        if (intervalsOverlap(start, s.durationUnits,
                             otherStart, other.durationUnits) &&
            (s.teacher == other.teacher ||
             s.batch == other.batch ||
             room == placements_[j].room))
            return false;
    }
    return true;
}

bool Scheduler::search(int i)
{
    if (i == static_cast<int>(problem_.sessions.size()))
        return true;

    for (int slot = 0; slot < SLOTS_PER_WEEK; ++slot)
    {
        for (int room = 0; room < static_cast<int>(problem_.rooms.size()); ++room)
        {
            ++attempts_;
            if (!canPlace(i, slot, room))
                continue;

            placements_[i] = {slot, room}; // Choose a placement.
            if (search(i + 1))
                return true;
            placements_[i] = {-1, -1}; // Undo and try another placement.
            ++backtracks_;
        }
    }
    return false;
}

bool Scheduler::generate()
{
    placements_.assign(problem_.sessions.size(), Placement{});
    attempts_ = backtracks_ = 0;
    return search(0);
}

} // namespace timetablex
