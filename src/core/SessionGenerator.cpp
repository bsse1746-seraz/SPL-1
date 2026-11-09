#include "timetablex/core/SessionGenerator.hpp"

#include "timetablex/core/SchedulingRules.hpp"

#include <stdexcept>

namespace timetablex
{

using std::invalid_argument;
using std::vector;

namespace
{

int requiredMinutes(int halfCredits, int minutesPerCredit)
{
    if (halfCredits < 0 || minutesPerCredit <= 0 ||
        halfCredits > 100 || minutesPerCredit > 1000 ||
        (halfCredits * minutesPerCredit) % 2 != 0)
        throw invalid_argument("Invalid credit value or credit conversion policy");
    return halfCredits * minutesPerCredit / 2;
}

void appendComponentSessions(vector<Session> &sessions,
                             const CourseRequirement &c,
                             const vector<int> &patternMinutes,
                             int expectedMinutes, bool isLab)
{
    int total = 0;
    for (int minutes : patternMinutes)
    {
        if (minutes <= 0 || minutes % MINUTES_PER_UNIT != 0 ||
            !fitsAcademicWindow(isLab ? LAB_START_UNIT : THEORY_START_UNIT,
                                minutes / MINUTES_PER_UNIT, isLab))
            throw invalid_argument("Invalid theory/lab session duration for " + c.course);
        total += minutes;
    }
    if (total != expectedMinutes)
        throw invalid_argument("Session pattern does not match credits for " + c.course);

    for (int minutes : patternMinutes)
        sessions.push_back({c.course + (isLab ? " Lab" : ""),
                            c.teacher, c.batch, minutes / MINUTES_PER_UNIT, isLab});
}

} // namespace

vector<Session> generateRequiredSessions(const Problem &problem)
{
    vector<Session> sessions;
    for (const CourseRequirement &c : problem.courses)
    {
        if (c.course.empty() || c.teacher < 0 ||
            c.teacher >= static_cast<int>(problem.teacherNames.size()) ||
            c.batch < 0 || c.batch >= static_cast<int>(problem.groups.size()) ||
            (c.theoryHalfCredits == 0 && c.labHalfCredits == 0))
            throw invalid_argument("Invalid course, assigned teacher, or student group");

        int theoryMinutes = requiredMinutes(c.theoryHalfCredits, problem.policy.theoryMinutesPerCredit);
        int labMinutes = requiredMinutes(c.labHalfCredits, problem.policy.labMinutesPerCredit);
        appendComponentSessions(sessions, c, c.theoryPatternMinutes, theoryMinutes, false);
        appendComponentSessions(sessions, c, c.labPatternMinutes, labMinutes, true);
    }
    return sessions;
}

} // namespace timetablex
