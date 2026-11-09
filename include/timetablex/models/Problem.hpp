#pragma once

#include <string>
#include <vector>

namespace timetablex
{

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
    std::vector<std::string> teacherNames;
    std::vector<Room> rooms;
    std::vector<StudentGroup> groups;
    // A teacher must be free for EVERY half-hour unit of a session.
    std::vector<std::vector<bool>> available;
};

} // namespace timetablex
