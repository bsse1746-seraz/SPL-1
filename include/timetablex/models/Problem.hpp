#pragma once

#include <string>
#include <vector>

namespace timetablex
{

inline const std::vector<std::string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday"};
inline const std::vector<std::string> teachers = {"Teacher A", "Teacher B", "Teacher C"};
// A lab session requires a computer laboratory, not an ordinary classroom.
enum class RoomType { Classroom, ComputerLab };

struct Room
{
    std::string name;
    int capacity;
    RoomType type;
};

struct StudentGroup
{
    std::string name;
    int size;
};

// Fictional demonstration resources. Real departments will load these from input.
// Room-101 is a computer lab, Room-102 an ordinary classroom.
inline const std::vector<Room> defaultRooms = {
    {"Room-101", 40, RoomType::ComputerLab},
    {"Room-102", 30, RoomType::Classroom}
};
inline const std::vector<StudentGroup> defaultGroups = {
    {"BSSE-17", 32},
    {"BSSE-18", 26}
};

inline constexpr int MINUTES_PER_UNIT = 30;
inline constexpr int UNITS_PER_DAY = 16; // 09:00 through 17:00, in 30-minute units.
inline constexpr int DAYS_PER_WEEK = 5;
inline constexpr int SLOTS_PER_WEEK = DAYS_PER_WEEK * UNITS_PER_DAY;
inline constexpr int THEORY_START_UNIT = 0; // 09:00
inline constexpr int THEORY_END_UNIT = 8;   // 13:00 (exclusive)
inline constexpr int LAB_START_UNIT = 10;   // 14:00
inline constexpr int DAY_END_UNIT = 16;     // 17:00 (exclusive)

struct Session
{
    std::string course;
    int teacher, batch;
    int durationUnits = 2; // Two 30-minute units = one hour by default.
    bool isLab = false;
};

struct Placement
{
    int slot = -1; // Weekly index: day * 16 + starting half-hour unit.
    int room = -1;
};

// We store credits as HALF-CREDIT units, avoiding floating-point comparisons.
// Example: 3.0 credits => 6, 1.5 credits => 3, 1.0 credit => 2.
struct CourseRequirement
{
    std::string course;
    int teacher, batch; // Teacher is ASSIGNED by the department, never selected by solver.
    int theoryHalfCredits = 0;
    int labHalfCredits = 0;
    std::vector<int> theoryPatternMinutes; // Coordinator-approved session durations.
    std::vector<int> labPatternMinutes;
};

// Defaults are a demonstration policy, NOT verified IIT-DU regulations.
// The coordinator can change either conversion rule in future data input.
struct CreditPolicy
{
    int theoryMinutesPerCredit = 60;
    int labMinutesPerCredit = 120;
};

struct Problem
{
    std::vector<CourseRequirement> courses;
    CreditPolicy policy;
    std::vector<Session> sessions;
    std::vector<Room> rooms = defaultRooms;
    std::vector<StudentGroup> groups = defaultGroups;
    // A teacher must be free for EVERY half-hour unit of a session.
    std::vector<std::vector<bool>> available =
        std::vector<std::vector<bool>>(3, std::vector<bool>(SLOTS_PER_WEEK, true));
};

} // namespace timetablex
