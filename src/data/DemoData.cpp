#include "timetablex/data/DemoData.hpp"

#include "timetablex/core/SchedulingRules.hpp"
#include "timetablex/core/SessionGenerator.hpp"
#include "timetablex/io/TimetableCsv.hpp"

#include <fstream>
#include <stdexcept>

namespace timetablex
{

Problem makeDemoResources()
{
    Problem problem;
    problem.teacherNames = {"Teacher A", "Teacher B", "Teacher C"};
    problem.rooms = {
        {"Room-101", 40, RoomType::ComputerLab},
        {"Room-102", 30, RoomType::Classroom}
    };
    problem.groups = {
        {"BSSE-17", 32},
        {"BSSE-18", 26}
    };
    problem.available.assign(problem.teacherNames.size(), std::vector<bool>(SLOTS_PER_WEEK, true));
    return problem;
}

Problem makeDemoProblem()
{
    Problem p = makeDemoResources();
    // Same nine sessions as M1, now DERIVED from course requirements.
    // Each vector contains approved per-session lengths, in minutes.
    p.courses = {
        {"C++", 0, 0, 4, 2, {60, 60}, {120}},
        {"Math", 1, 0, 4, 0, {60, 60}, {}},
        {"Algorithms", 0, 1, 4, 0, {60, 60}, {}},
        {"English", 2, 1, 4, 0, {60, 60}, {}}
    };
    p.sessions = generateRequiredSessions(p);
    p.available[0][0] = false; // Teacher A unavailable Sunday 09:00-09:30.
    return p;
}

// External courses use the SAME fixed-teacher CSP solver as built-in courses.
// Teacher availability is all true for now; an availability input file is a later milestone.
Problem loadDemoProblem(const std::string &path)
{
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("Cannot open course CSV: " + path);
    Problem p = makeDemoResources();
    p.courses = loadCoursesCSV(input);
    p.sessions = generateRequiredSessions(p);
    return p;
}

} // namespace timetablex
