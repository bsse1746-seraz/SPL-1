#pragma once

#include "timetablex/models/Problem.hpp"

namespace timetablex
{

bool intervalsOverlap(int startA, int durationA, int startB, int durationB);
bool fitsAcademicWindow(int startUnit, int durationUnits, bool isLab);
bool roomSupportsSession(const Room &room, const Session &session);
std::string clockTime(int unitWithinDay);
std::string roomTypeName(RoomType type);

} // namespace timetablex
