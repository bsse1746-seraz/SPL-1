// TimeTableX Mini | M2 Step 1: course credits to weekly session requirements (C++17).
// Fixed teachers; configurable academic credit policy; validated session patterns.
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;

const vector<string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday"};
const vector<string> teachers = {"Teacher A", "Teacher B", "Teacher C"};
const vector<string> batches = {"BSSE-17", "BSSE-18"};
const vector<string> rooms = {"Room-101", "Room-102"};

constexpr int MINUTES_PER_UNIT = 30;
constexpr int UNITS_PER_DAY = 16; // 09:00 through 17:00, in 30-minute units.
constexpr int DAYS_PER_WEEK = 5;
constexpr int slots = DAYS_PER_WEEK * UNITS_PER_DAY;
constexpr int THEORY_START_UNIT = 0; // 09:00
constexpr int THEORY_END_UNIT = 8;   // 13:00 (exclusive)
constexpr int LAB_START_UNIT = 10;   // 14:00
constexpr int DAY_END_UNIT = 16;     // 17:00 (exclusive)

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
        return startUnit == LAB_START_UNIT &&
               durationUnits == DAY_END_UNIT - LAB_START_UNIT;

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

struct Session
{
    string course;
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
    string course;
    int teacher, batch; // Teacher is ASSIGNED by the department, never selected by solver.
    int theoryHalfCredits = 0;
    int labHalfCredits = 0;
    vector<int> theoryPatternMinutes; // Coordinator-approved session durations.
    vector<int> labPatternMinutes;
};

// Defaults are a demonstration policy, NOT verified IIT-DU regulations.
// The coordinator can change either conversion rule in future data input.
struct CreditPolicy
{
    int theoryMinutesPerCredit = 60;
    int labMinutesPerCredit = 180;
};

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

vector<Session> generateRequiredSessions(const vector<CourseRequirement> &courses,
                                         const CreditPolicy &policy)
{
    vector<Session> sessions;
    for (const CourseRequirement &c : courses)
    {
        if (c.course.empty() || c.teacher < 0 ||
            c.teacher >= static_cast<int>(teachers.size()) ||
            c.batch < 0 || c.batch >= static_cast<int>(batches.size()) ||
            (c.theoryHalfCredits == 0 && c.labHalfCredits == 0))
            throw invalid_argument("Invalid course, assigned teacher, or student group");

        int theoryMinutes = requiredMinutes(c.theoryHalfCredits, policy.theoryMinutesPerCredit);
        int labMinutes = requiredMinutes(c.labHalfCredits, policy.labMinutesPerCredit);
        appendComponentSessions(sessions, c, c.theoryPatternMinutes, theoryMinutes, false);
        appendComponentSessions(sessions, c, c.labPatternMinutes, labMinutes, true);
    }
    return sessions;
}

string creditLabel(int halfCredits)
{
    return to_string(halfCredits / 2) + (halfCredits % 2 ? ".5" : ".0");
}

struct Problem
{
    vector<CourseRequirement> courses;
    CreditPolicy policy;
    vector<Session> sessions;
    // A teacher must be free for EVERY half-hour unit of a session.
    vector<vector<bool>> available =
        vector<vector<bool>>(3, vector<bool>(slots, true));
};

Problem sample()
{
    Problem p;
    // Same nine sessions as M1, now DERIVED from course requirements.
    // Each vector contains approved per-session lengths, in minutes.
    p.courses = {
        {"C++", 0, 0, 4, 2, {60, 60}, {180}},
        {"Math", 1, 0, 4, 0, {60, 60}, {}},
        {"Algorithms", 0, 1, 4, 0, {60, 60}, {}},
        {"English", 2, 1, 4, 0, {60, 60}, {}}
    };
    p.sessions = generateRequiredSessions(p.courses, p.policy);
    p.available[0][0] = false; // Teacher A unavailable Sunday 09:00-09:30.
    return p;
}

class Scheduler
{
    const Problem &p;

    bool canPlace(int i, int slot, int room) const
    {
        const Session &s = p.sessions[i];
        if (s.teacher < 0 || s.teacher >= static_cast<int>(p.available.size()) ||
            s.batch < 0 || s.batch >= static_cast<int>(batches.size()) ||
            room < 0 || room >= static_cast<int>(rooms.size()))
            return false;

        int day = slot / UNITS_PER_DAY;
        int start = slot % UNITS_PER_DAY;
        if (day < 0 || day >= DAYS_PER_WEEK ||
            !fitsAcademicWindow(start, s.durationUnits, s.isLab))
            return false;

        // Check the teacher's availability at every occupied half-hour unit.
        if (p.available[s.teacher].size() < static_cast<size_t>(slots))
            return false;
        for (int u = 0; u < s.durationUnits; ++u)
            if (!p.available[s.teacher][slot + u])
                return false;

        // Compare intervals, not just their starting times.
        for (int j = 0; j < i; ++j)
        {
            if (result[j].slot < 0 || result[j].slot / UNITS_PER_DAY != day)
                continue;

            const Session &other = p.sessions[j];
            int otherStart = result[j].slot % UNITS_PER_DAY;
            if (intervalsOverlap(start, s.durationUnits,
                                 otherStart, other.durationUnits) &&
                (s.teacher == other.teacher ||
                 s.batch == other.batch ||
                 room == result[j].room))
                return false;
        }
        return true;
    }

    bool search(int i)
    {
        if (i == static_cast<int>(p.sessions.size()))
            return true;

        for (int slot = 0; slot < slots; ++slot)
        {
            for (int room = 0; room < static_cast<int>(rooms.size()); ++room)
            {
                ++attempts;
                if (!canPlace(i, slot, room))
                    continue;

                result[i] = {slot, room}; // Choose a placement.
                if (search(i + 1))
                    return true;
                result[i] = {-1, -1}; // Undo and try another placement.
                ++backtracks;
            }
        }
        return false;
    }

public:
    vector<Placement> result;
    long long attempts = 0, backtracks = 0;

    explicit Scheduler(const Problem &problem) : p(problem) {}

    bool generate()
    {
        result.assign(p.sessions.size(), Placement{});
        attempts = backtracks = 0;
        return search(0);
    }
};

// Independent completed-schedule check: never calls Scheduler::canPlace().
bool validate(const Problem &p, const vector<Placement> &a)
{
    if (a.size() != p.sessions.size())
        return false;

    for (size_t i = 0; i < a.size(); ++i)
    {
        const Session &s = p.sessions[i];
        if (s.teacher < 0 || s.teacher >= static_cast<int>(p.available.size()) ||
            s.batch < 0 || s.batch >= static_cast<int>(batches.size()) ||
            a[i].slot < 0 || a[i].slot >= slots ||
            a[i].room < 0 || a[i].room >= static_cast<int>(rooms.size()))
            return false;

        int day = a[i].slot / UNITS_PER_DAY;
        int start = a[i].slot % UNITS_PER_DAY;
        if (day >= DAYS_PER_WEEK ||
            !fitsAcademicWindow(start, s.durationUnits, s.isLab))
            return false;

        if (p.available[s.teacher].size() < static_cast<size_t>(slots))
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

void showInput(const Problem &p)
{
    cout << "\nINPUT: " << p.sessions.size()
         << " sessions, 3 teachers, 2 batches, 2 rooms.\n";
    cout << "Theory 09:00-13:00 | Break 13:00-14:00 | Lab 14:00-17:00.\n";
    if (!p.courses.empty())
    {
        cout << "Credit policy (DEMO): theory " << p.policy.theoryMinutesPerCredit
             << " min/credit; lab " << p.policy.labMinutesPerCredit << " min/credit.\n";
        cout << "Course requirements (teacher assignments are fixed):\n";
        for (const CourseRequirement &c : p.courses)
            cout << "  " << c.course << ": theory " << creditLabel(c.theoryHalfCredits)
                 << " + lab " << creditLabel(c.labHalfCredits)
                 << " credits | " << teachers[c.teacher] << " | " << batches[c.batch] << '\n';
        cout << "Generated weekly sessions:\n";
    }
    for (size_t i = 0; i < p.sessions.size(); ++i)
    {
        const Session &s = p.sessions[i];
        cout << i + 1 << ". " << s.course
             << " | " << teachers[s.teacher]
             << " | " << batches[s.batch]
             << " | " << (s.isLab ? "Lab" : "Theory")
             << " | " << s.durationUnits * MINUTES_PER_UNIT << " min\n";
    }
    cout << "Teacher A unavailable Sunday 09:00-09:30.\n";
}

void show(const Problem &p, const vector<Placement> &a, int batch = -1)
{
    cout << '\n' << left << setw(12) << "Day"
         << setw(15) << "Time" << setw(15) << "Course"
         << setw(10) << "Type" << setw(13) << "Teacher"
         << setw(12) << "Batch" << "Room\n";

    for (int slot = 0; slot < slots; ++slot)
        for (size_t i = 0; i < a.size(); ++i)
        {
            const Session &s = p.sessions[i];
            if (a[i].slot != slot || (batch != -1 && s.batch != batch))
                continue;

            int start = slot % UNITS_PER_DAY;
            string timeRange = clockTime(start) + "-" + clockTime(start + s.durationUnits);
            cout << setw(12) << days[slot / UNITS_PER_DAY]
                 << setw(15) << timeRange
                 << setw(15) << s.course
                 << setw(10) << (s.isLab ? "Lab" : "Theory")
                 << setw(13) << teachers[s.teacher]
                 << setw(12) << batches[s.batch]
                 << rooms[a[i].room] << '\n';
        }
}

string csvCell(const string &value)
{
    string out = "\"";
    for (char c : value)
    {
        if (c == '"')
            out += '"';
        out += c;
    }
    return out + "\"";
}

bool exportCSV(const Problem &p, const vector<Placement> &a)
{
    ofstream f("timetable.csv");
    if (!f)
        return false;

    f << "Day,Start,End,Course,Type,Teacher,Batch,Room\n";
    for (int slot = 0; slot < slots; ++slot)
        for (size_t i = 0; i < a.size(); ++i)
            if (a[i].slot == slot)
            {
                const Session &s = p.sessions[i];
                int start = slot % UNITS_PER_DAY;
                f << csvCell(days[slot / UNITS_PER_DAY]) << ','
                  << csvCell(clockTime(start)) << ','
                  << csvCell(clockTime(start + s.durationUnits)) << ','
                  << csvCell(s.course) << ','
                  << csvCell(s.isLab ? "Lab" : "Theory") << ','
                  << csvCell(teachers[s.teacher]) << ','
                  << csvCell(batches[s.batch]) << ','
                  << csvCell(rooms[a[i].room]) << '\n';
            }
    f.close();
    return !f.fail();
}

void demo(bool impossible)
{
    Problem p = sample();
    showInput(p);
    if (impossible)
    {
        p.available[0].assign(slots, false);
        cout << "\nIMPOSSIBLE CASE: Teacher A is unavailable all week.\n";
    }

    Scheduler engine(p);
    if (!engine.generate())
    {
        cout << "No valid timetable exists for these demo rules (search exhausted).\n";
        return;
    }
    if (!validate(p, engine.result))
        throw runtime_error("Internal validation failed.");

    cout << "\nSUCCESS: all sessions assigned. Hard-constraint violations: 0.\n";
    show(p, engine.result);
    cout << "\nBSSE-17 view:\n";
    show(p, engine.result, 0);
    cout << "\nBSSE-18 view:\n";
    show(p, engine.result, 1);
    cout << "\nCandidate attempts: " << engine.attempts
         << "; backtracks: " << engine.backtracks << '\n';
    cout << "This is the first valid solution; preferences are not optimized yet.\n";
    cout << "Room facility/capacity rules will be added in M3.\n";
    cout << (exportCSV(p, engine.result)
                 ? "Saved timetable.csv in the working folder.\n"
                 : "Could not save timetable.csv.\n");
}

void selfTest()
{
    int count = 0;
    auto check = [&](bool ok, const string &name)
    {
        if (!ok)
            throw runtime_error("FAILED: " + name);
        cout << "PASS: " << name << '\n';
        ++count;
    };

    // The original nine checks, adapted for 30-minute slots.
    Problem p = sample();
    Scheduler s(p);
    check(s.generate() && validate(p, s.result), "normal schedule covers all sessions");
    auto broken = s.result;
    broken[0].slot = 0;
    check(!validate(p, broken), "unavailable teacher rejected");
    broken = s.result;
    broken.pop_back();
    check(!validate(p, broken), "missing session rejected");

    auto pairCase = [&](Session a, Session b, int roomA, int roomB)
    {
        Problem q;
        q.sessions = {a, b};
        return validate(q, {{2, roomA}, {2, roomB}});
    };
    check(!pairCase({"A", 0, 0}, {"B", 0, 1}, 0, 1), "teacher clash rejected");
    check(!pairCase({"A", 0, 0}, {"B", 1, 0}, 0, 1), "batch clash rejected");
    check(!pairCase({"A", 0, 0}, {"B", 1, 1}, 0, 0), "room clash rejected");
    check(pairCase({"A", 0, 0}, {"B", 1, 1}, 0, 1), "independent simultaneous classes allowed");

    p.available[0].assign(slots, false);
    Scheduler impossible(p);
    check(!impossible.generate(), "impossible timetable handled");

    // Flexible class initially takes 09:00, but Restricted needs 09:00.
    Problem q;
    q.sessions = {{"Flexible", 0, 0}, {"Restricted", 1, 0}};
    q.available[0].assign(slots, false);
    q.available[1].assign(slots, false);
    q.available[0][0] = q.available[0][1] = true;
    q.available[0][2] = q.available[0][3] = true;
    q.available[1][0] = q.available[1][1] = true;
    Scheduler retry(q);
    check(retry.generate() && retry.backtracks > 0 && validate(q, retry.result),
          "dead end undone by backtracking");

    // The five academic-window checks from M1 Step 1.
    check(fitsAcademicWindow(0, 2, false), "one-hour theory allowed");
    check(fitsAcademicWindow(5, 3, false), "90-minute theory allowed");
    check(!fitsAcademicWindow(7, 2, false), "theory crossing break rejected");
    check(fitsAcademicWindow(10, 6, true), "three-hour afternoon lab allowed");
    check(!fitsAcademicWindow(9, 6, true), "lab starting during break rejected");

    // The four interval-overlap checks from M1 Step 2.
    check(intervalsOverlap(0, 6, 4, 2), "overlapping theory sessions detected");
    check(!intervalsOverlap(0, 2, 2, 2), "back-to-back theory sessions allowed");
    check(!intervalsOverlap(0, 2, 6, 2), "separate theory sessions allowed");
    check(intervalsOverlap(10, 6, 12, 2), "overlapping laboratory period detected");

    // M1 Step 3: integration checks of the ACTUAL solver and validator.
    bool sampleLabAt14 = false;
    for (size_t idx = 0; idx < p.sessions.size(); ++idx)
        if (p.sessions[idx].isLab &&
            s.result[idx].slot % UNITS_PER_DAY == LAB_START_UNIT)
            sampleLabAt14 = true;
    check(sampleLabAt14, "solver places three-hour lab at 14:00");
    check(clockTime(16) == "17:00" && clockTime(10) == "14:00",
          "clock formatting handles lab end time");

    Problem one;
    one.sessions = {{"90-minute Theory", 0, 0, 3, false}};
    Scheduler ninety(one);
    check(ninety.generate() && validate(one, ninety.result),
          "solver schedules 90-minute theory");

    Problem pair;
    pair.sessions = {{"Long Theory", 0, 0, 3, false},
                     {"Other Theory", 0, 1, 2, false}};
    check(!validate(pair, {{0, 0}, {2, 1}}),
          "validator catches partially overlapping teacher sessions");
    pair.sessions[1].teacher = 1;
    pair.sessions[1].batch = 0;
    check(!validate(pair, {{0, 0}, {2, 1}}),
          "validator catches partially overlapping student group");
    pair.sessions[1].batch = 1;
    check(!validate(pair, {{0, 0}, {2, 0}}),
          "validator catches partially overlapping room bookings");
    check(validate(pair, {{0, 0}, {2, 1}}),
          "independent overlapping sessions use separate resources");

    pair.sessions[1].teacher = 0;
    pair.sessions[1].batch = 0;
    pair.sessions[0].durationUnits = 2;
    check(validate(pair, {{0, 0}, {2, 0}}),
          "back-to-back sessions may share teacher batch and room");

    Problem unavailableMid;
    unavailableMid.sessions = {{"Theory", 0, 0, 2, false}};
    unavailableMid.available[0][1] = false; // Teacher absent 09:30-10:00.
    check(!validate(unavailableMid, {{0, 0}}),
          "validator checks availability inside entire class");
    Scheduler avoidUnavailable(unavailableMid);
    check(avoidUnavailable.generate() && validate(unavailableMid, avoidUnavailable.result) &&
              avoidUnavailable.result[0].slot != 0,
          "solver avoids unavailable interior time unit");

    Problem boundary;
    boundary.sessions = {{"Theory", 0, 0, 2, false}};
    check(!validate(boundary, {{7, 0}}),
          "validator rejects theory crossing lunch break");

    Problem lab;
    lab.sessions = {{"Lab", 0, 0, 6, true}};
    check(!validate(lab, {{9, 0}}) && !validate(lab, {{11, 0}}),
          "validator rejects lab outside 14:00-17:00");
    lab.sessions[0].durationUnits = 4;
    Scheduler wrongLab(lab);
    check(!wrongLab.generate() && !validate(lab, {{10, 0}}),
          "invalid lab duration rejected by solver and validator");

    Problem nextDay;
    nextDay.sessions = {{"Theory A", 0, 0}, {"Theory B", 0, 0}};
    check(validate(nextDay, {{0, 0}, {UNITS_PER_DAY, 0}}),
          "same teacher may teach on different days at same time");

    // M2 Step 1: courses + credit policy -> validated weekly sessions.
    Problem derived = sample();
    int generatedLabCount = 0;
    for (const Session &session : derived.sessions)
        if (session.isLab && session.durationUnits == 6)
            ++generatedLabCount;
    check(derived.courses.size() == 4 && derived.sessions.size() == 9 &&
              derived.sessions[0].course == "C++" && generatedLabCount == 1,
          "sample sessions generated from credit requirements");

    vector<CourseRequirement> threeCredit = {
        {"Algorithms", 0, 0, 6, 0, {60, 60, 60}, {}}};
    vector<Session> three = generateRequiredSessions(threeCredit, CreditPolicy{});
    check(three.size() == 3 && three[0].durationUnits == 2 &&
              three[1].durationUnits == 2 && three[2].durationUnits == 2,
          "three theory credits create three one-hour sessions");

    vector<CourseRequirement> oneAndHalf = {
        {"Linear Algebra", 1, 0, 3, 0, {90}, {}}};
    vector<Session> ninetyCredit = generateRequiredSessions(oneAndHalf, CreditPolicy{});
    check(ninetyCredit.size() == 1 && ninetyCredit[0].durationUnits == 3,
          "one-and-half theory credits create a ninety-minute session");

    vector<CourseRequirement> combined = {
        {"Programming", 2, 1, 6, 2, {60, 60, 60}, {180}}};
    vector<Session> combinedSessions = generateRequiredSessions(combined, CreditPolicy{});
    check(combinedSessions.size() == 4 && combinedSessions.back().isLab &&
              combinedSessions.back().durationUnits == 6 &&
              combinedSessions.back().teacher == 2 && combinedSessions.back().batch == 1,
          "combined course generates fixed-teacher theory and lab sessions");

    auto rejectsRequirements = [&](const vector<CourseRequirement> &bad,
                                   const CreditPolicy &policy)
    {
        try
        {
            (void)generateRequiredSessions(bad, policy);
        }
        catch (const invalid_argument &)
        {
            return true;
        }
        return false;
    };
    check(rejectsRequirements({{"Bad Theory", 0, 0, 6, 0, {60, 60}, {}}}, CreditPolicy{}),
          "mismatched theory contact hours rejected");
    check(rejectsRequirements({{"Bad Lab", 0, 0, 0, 2, {}, {90, 90}}}, CreditPolicy{}),
          "split ninety-minute lab sessions rejected");
    check(rejectsRequirements({{"Bad Duration", 0, 0, 3, 0, {45, 45}, {}}}, CreditPolicy{}),
          "non-grid-aligned theory sessions rejected");
    check(rejectsRequirements({{"Bad Teacher", 3, 0, 2, 0, {60}, {}}}, CreditPolicy{}),
          "invalid fixed instructor reference rejected");
    check(rejectsRequirements({{"No Credits", 0, 0, 0, 0, {}, {}}}, CreditPolicy{}),
          "empty credit requirement rejected");

    CreditPolicy longerTheory;
    longerTheory.theoryMinutesPerCredit = 90;
    vector<CourseRequirement> alternativePattern = {
        {"Alternative Policy", 0, 0, 4, 0, {90, 90}, {}}};
    vector<Session> alternative = generateRequiredSessions(alternativePattern, longerTheory);
    check(alternative.size() == 2 && alternative[0].durationUnits == 3,
          "configurable theory contact-hour policy supported");

    cout << count << " tests passed.\n";
}

int main(int argc, char *argv[])
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
                selfTest();
            else
            {
                cerr << "Use --demo, --impossible or --test.\n";
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
                selfTest();
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
