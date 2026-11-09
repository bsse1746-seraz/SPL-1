#include "timetablex/core/SchedulingRules.hpp"

#include <iomanip>
#include <sstream>

namespace timetablex
{

using std::ostringstream;
using std::setfill;
using std::setw;
using std::string;

// Intervals are [start, end): a class ending at 10:00 does not clash
// with another beginning at 10:00.
bool intervalsOverlap(int startA, int durationA,
                      int startB, int durationB)
{
    int endA = startA + durationA;
    int endB = startB + durationB;
    return startA < endB && startB < endA;
}

// These unit numbers are relative to ONE day, beginning at 09:00.
bool fitsAcademicWindow(int startUnit, int durationUnits, bool isLab)
{
    if (startUnit < 0 || durationUnits <= 0)
        return false;

    if (isLab)
        return startUnit >= LAB_START_UNIT &&
               startUnit < DAY_END_UNIT &&
               durationUnits <= DAY_END_UNIT - startUnit;

    return startUnit >= THEORY_START_UNIT &&
           startUnit < THEORY_END_UNIT &&
           durationUnits <= THEORY_END_UNIT - startUnit;
}

string clockTime(int unitWithinDay)
{
    int minutes = 9 * 60 + unitWithinDay * MINUTES_PER_UNIT;
    ostringstream out;
    out << setfill('0') << setw(2) << minutes / 60
        << ':' << setw(2) << minutes % 60;
    return out.str();
}

// HARD room suitability rule: theory may use either room type,
// but laboratory sessions require a computer lab. Capacity is checked separately.
bool roomSupportsSession(const Room &room, const Session &session)
{
    if (session.isLab)
        return room.type == RoomType::ComputerLab;
    return room.type == RoomType::Classroom ||
           room.type == RoomType::ComputerLab;
}

string roomTypeName(RoomType type)
{
    return type == RoomType::ComputerLab ? "Computer lab" : "Classroom";
}

} // namespace timetablex
