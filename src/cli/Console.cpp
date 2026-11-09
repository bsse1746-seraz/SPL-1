#include "Console.hpp"

#include "SelfTests.hpp"

#include "timetablex/app/SchedulingService.hpp"
#include "timetablex/core/SchedulingRules.hpp"
#include "timetablex/data/DemoData.hpp"
#include "timetablex/io/TimetableCsv.hpp"

#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace timetablex
{

using std::cerr;
using std::cin;
using std::cout;
using std::exception;
using std::getline;
using std::left;
using std::setw;
using std::size_t;
using std::string;
using std::to_string;
using std::vector;

namespace
{

string creditLabel(int halfCredits)
{
    return to_string(halfCredits / 2) + (halfCredits % 2 ? ".5" : ".0");
}

void showInput(const Problem &p, bool externalData = false)
{
    cout << "\nINPUT: " << p.sessions.size() << " sessions, "
         << p.teacherNames.size() << " teachers, " << p.groups.size()
         << " groups, " << p.rooms.size() << " rooms.\n";
    cout << "Room inventory (capacity and lab suitability enforced):\n";
    for (const Room &r : p.rooms)
        cout << "  " << r.name << " | " << roomTypeName(r.type)
             << " | capacity " << r.capacity << "\n";
    cout << "Student group sizes:\n";
    for (const StudentGroup &g : p.groups)
        cout << "  " << g.name << " | " << g.size << " students\n";
    cout << "Theory 09:00-13:00 | Break 13:00-14:00 | Lab 14:00-17:00.\n";
    if (!p.courses.empty())
    {
        cout << "Credit policy (DEMO): theory " << p.policy.theoryMinutesPerCredit
             << " min/credit; lab " << p.policy.labMinutesPerCredit << " min/credit.\n";
        cout << "Course requirements (teacher assignments are fixed):\n";
        for (const CourseRequirement &c : p.courses)
            cout << "  " << c.course << ": theory " << creditLabel(c.theoryHalfCredits)
                 << " + lab " << creditLabel(c.labHalfCredits)
                 << " credits | " << p.teacherNames[c.teacher] << " | " << p.groups[c.batch].name << '\n';
        cout << "Generated weekly sessions:\n";
    }
    for (size_t i = 0; i < p.sessions.size(); ++i)
    {
        const Session &s = p.sessions[i];
        cout << i + 1 << ". " << s.course
             << " | " << p.teacherNames[s.teacher]
             << " | " << p.groups[s.batch].name
             << " | " << (s.isLab ? "Lab" : "Theory")
             << " | " << s.durationUnits * MINUTES_PER_UNIT << " min\n";
    }
    if (externalData)
        cout << "File demo: teacher availability is unrestricted (preferences not loaded yet).\n";
    else
        cout << "Teacher A unavailable Sunday 09:00-09:30.\n";
}

void show(const Problem &p, const vector<Placement> &a, int batch = -1)
{
    cout << '\n' << left << setw(12) << "Day"
         << setw(15) << "Time" << setw(15) << "Course"
         << setw(10) << "Type" << setw(13) << "Teacher"
         << setw(12) << "Batch" << "Room\n";

    for (int slot = 0; slot < SLOTS_PER_WEEK; ++slot)
        for (size_t i = 0; i < a.size(); ++i)
        {
            const Session &s = p.sessions[i];
            if (a[i].slot != slot || (batch != -1 && s.batch != batch))
                continue;

            int start = slot % UNITS_PER_DAY;
            string timeRange = clockTime(start) + "-" + clockTime(start + s.durationUnits);
            cout << setw(12) << DAY_NAMES[slot / UNITS_PER_DAY]
                 << setw(15) << timeRange
                 << setw(15) << s.course
                 << setw(10) << (s.isLab ? "Lab" : "Theory")
                 << setw(13) << p.teacherNames[s.teacher]
                 << setw(12) << p.groups[s.batch].name
                 << p.rooms[a[i].room].name << '\n';
        }
}

void demo(bool impossible, const string &csvPath = "")
{
    Problem p = csvPath.empty() ? makeDemoProblem() : loadDemoProblem(csvPath);
    showInput(p, !csvPath.empty());
    if (impossible)
    {
        p.available[0].assign(SLOTS_PER_WEEK, false);
        cout << "\nIMPOSSIBLE CASE: Teacher A is unavailable all week.\n";
    }

    const ScheduleResult result = SchedulingService{}.generate(p);
    if (!result.found)
    {
        cout << "No valid timetable exists for these demo rules (search exhausted).\n";
        return;
    }

    cout << "\nSUCCESS: all sessions assigned. Hard-constraint violations: 0.\n";
    show(p, result.placements);
    cout << "\nBSSE-17 view:\n";
    show(p, result.placements, 0);
    cout << "\nBSSE-18 view:\n";
    show(p, result.placements, 1);
    cout << "\nCandidate attempts: " << result.attempts
         << "; backtracks: " << result.backtracks << '\n';
    cout << "This is the first valid solution; preferences are not optimized yet.\n";
    cout << "Room capacity and computer-lab suitability are enforced.\n";
    cout << (exportCSV(p, result.placements, "timetable.csv")
                 ? "Saved timetable.csv in the working folder.\n"
                 : "Could not save timetable.csv.\n");
}

} // namespace

int runConsole(int argc, char *argv[])
{
    try
    {
        cout << "TIMETABLEX MINI | C++17 | SPL-1 prototype\n";
        if (argc > 1)
        {
            string arg = argv[1];
            if (arg == "--demo")
                demo(false);
            else if (arg == "--impossible")
                demo(true);
            else if (arg == "--test")
                runSelfTests();
            else if (arg == "--demo-file" && argc == 3)
                demo(false, argv[2]);
            else
            {
                cerr << "Use --demo, --impossible, --test, or --demo-file <courses.csv>.\n";
                return 1;
            }
            return 0;
        }
        string input;
        while (true)
        {
            cout << "\n1. Generate sample timetable\n2. Try impossible case\n3. Run checks\n0. Exit\nChoice: ";
            if (!getline(cin, input) || input == "0")
                break;
            if (input == "1")
                demo(false);
            else if (input == "2")
                demo(true);
            else if (input == "3")
                runSelfTests();
            else
                cout << "Enter 0, 1, 2 or 3.\n";
        }
    }
    catch (const exception &e)
    {
        cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}

} // namespace timetablex
