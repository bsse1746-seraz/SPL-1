#pragma once

#include "timetablex/models/Problem.hpp"

#include <array>

namespace timetablex
{

inline constexpr int MINUTES_PER_UNIT = 30;
inline constexpr int UNITS_PER_DAY = 16; // 09:00 through 17:00, in 30-minute units.
inline constexpr int DAYS_PER_WEEK = 5;
inline constexpr int SLOTS_PER_WEEK = DAYS_PER_WEEK * UNITS_PER_DAY;
inline constexpr int THEORY_START_UNIT = 0; // 09:00
inline constexpr int THEORY_END_UNIT = 8;   // 13:00 (exclusive)
inline constexpr int LAB_START_UNIT = 10;   // 14:00
inline constexpr int DAY_END_UNIT = 16;     // 17:00 (exclusive)
inline constexpr std::array<const char *, DAYS_PER_WEEK> DAY_NAMES = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday"
};

bool intervalsOverlap(int startA, int durationA, int startB, int durationB);
bool fitsAcademicWindow(int startUnit, int durationUnits, bool isLab);
bool roomSupportsSession(const Room &room, const Session &session);
std::string clockTime(int unitWithinDay);
std::string roomTypeName(RoomType type);

} // namespace timetablex
