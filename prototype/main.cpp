// TimeTableX Mini | M3 Step 2: laboratory room suitability (C++17).
// Fixed teachers, credit-based CSV input, and independent full-duration validation.
#include "timetablex/core/Scheduler.hpp"
#include "timetablex/core/SchedulingRules.hpp"
#include "timetablex/core/SessionGenerator.hpp"
#include "timetablex/core/Validator.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
using namespace std;
using namespace timetablex;

// M2 Step 2: deliberately limited CSV format (no quoted/comma-containing fields).
// Columns: course,teacher_id,batch_id,theory_half_credits,lab_half_credits,
//          theory_pattern_minutes,lab_pattern_minutes
// Durations within a pattern are separated by semicolons, e.g. 60;60;60.
// This parser does not silently guess missing or invalid values.
string trim(const string &value)
{
    size_t first = 0;
    while (first < value.size() && isspace(static_cast<unsigned char>(value[first])))
        ++first;
    size_t last = value.size();
    while (last > first && isspace(static_cast<unsigned char>(value[last - 1])))
        --last;
    return value.substr(first, last - first);
}

vector<string> splitFields(const string &line, char separator)
{
    vector<string> fields;
    string field;
    istringstream in(line);
    while (getline(in, field, separator))
        fields.push_back(trim(field));
    // Preserve an empty final CSV field, e.g. a theory-only course ending with ','.
    if (!line.empty() && line.back() == separator)
        fields.push_back("");
    return fields;
}

int parseNonnegativeInt(const string &text, const string &column)
{
    if (text.empty())
        throw invalid_argument("Missing integer in " + column);
    for (char c : text)
        if (c < '0' || c > '9')
            throw invalid_argument("Invalid nonnegative integer in " + column + ": " + text);
    try
    {
        size_t used = 0;
        int n = stoi(text, &used);
        if (used != text.size())
            throw invalid_argument("Invalid integer in " + column);
        return n;
    }
    catch (const out_of_range &)
    {
        throw invalid_argument("Integer out of range in " + column);
    }
}

vector<int> parseMinutesPattern(const string &value, const string &column)
{
    if (value.empty())
        return {};
    vector<int> minutes;
    for (const string &part : splitFields(value, ';'))
        minutes.push_back(parseNonnegativeInt(part, column));
    return minutes;
}

vector<CourseRequirement> loadCoursesCSV(istream &input)
{
    const string expectedHeader =
        "course,teacher_id,batch_id,theory_half_credits,lab_half_credits,"
        "theory_pattern_minutes,lab_pattern_minutes";
    string line;
    if (!getline(input, line))
        throw invalid_argument("Course CSV is empty");
    // Accept files with Windows CRLF line endings and standard whitespace.
    if (trim(line) != expectedHeader)
        throw invalid_argument("Incorrect course CSV header");

    vector<CourseRequirement> courses;
    int lineNumber = 1;
    while (getline(input, line))
    {
        ++lineNumber;
        if (trim(line).empty())
            continue;
        vector<string> fields = splitFields(line, ',');
        if (fields.size() != 7)
            throw invalid_argument("CSV line " + to_string(lineNumber) +
                                   " must contain exactly 7 columns");
        try
        {
            CourseRequirement c;
            c.course = fields[0];
            if (c.course.empty() || c.course.find('"') != string::npos)
                throw invalid_argument("Invalid course name");
            c.teacher = parseNonnegativeInt(fields[1], "teacher_id");
            c.batch = parseNonnegativeInt(fields[2], "batch_id");
            c.theoryHalfCredits = parseNonnegativeInt(fields[3], "theory_half_credits");
            c.labHalfCredits = parseNonnegativeInt(fields[4], "lab_half_credits");
            c.theoryPatternMinutes = parseMinutesPattern(fields[5], "theory_pattern_minutes");
            c.labPatternMinutes = parseMinutesPattern(fields[6], "lab_pattern_minutes");
            courses.push_back(c);
        }
        catch (const invalid_argument &e)
        {
            throw invalid_argument("CSV line " + to_string(lineNumber) + ": " + e.what());
        }
    }
    if (courses.empty())
        throw invalid_argument("Course CSV contains no course rows");
    return courses;
}

string creditLabel(int halfCredits)
{
    return to_string(halfCredits / 2) + (halfCredits % 2 ? ".5" : ".0");
}

Problem sample()
{
    Problem p;
    // Same nine sessions as M1, now DERIVED from course requirements.
    // Each vector contains approved per-session lengths, in minutes.
    p.courses = {
        {"C++", 0, 0, 4, 2, {60, 60}, {120}},
        {"Math", 1, 0, 4, 0, {60, 60}, {}},
        {"Algorithms", 0, 1, 4, 0, {60, 60}, {}},
        {"English", 2, 1, 4, 0, {60, 60}, {}}
    };
    p.sessions = generateRequiredSessions(p.courses, p.policy);
    p.available[0][0] = false; // Teacher A unavailable Sunday 09:00-09:30.
    return p;
}

// External courses use the SAME fixed-teacher CSP solver as built-in courses.
// Teacher availability is all true for now; an availability input file is a later milestone.
Problem fromCSVFile(const string &path)
{
    ifstream input(path);
    if (!input)
        throw runtime_error("Cannot open course CSV: " + path);
    Problem p;
    p.courses = loadCoursesCSV(input);
    p.sessions = generateRequiredSessions(p.courses, p.policy);
    return p;
}

void showInput(const Problem &p, bool externalData = false)
{
    cout << "\nINPUT: " << p.sessions.size() << " sessions, "
         << teachers.size() << " teachers, " << p.groups.size()
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
                 << " credits | " << teachers[c.teacher] << " | " << p.groups[c.batch].name << '\n';
        cout << "Generated weekly sessions:\n";
    }
    for (size_t i = 0; i < p.sessions.size(); ++i)
    {
        const Session &s = p.sessions[i];
        cout << i + 1 << ". " << s.course
             << " | " << teachers[s.teacher]
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
            cout << setw(12) << days[slot / UNITS_PER_DAY]
                 << setw(15) << timeRange
                 << setw(15) << s.course
                 << setw(10) << (s.isLab ? "Lab" : "Theory")
                 << setw(13) << teachers[s.teacher]
                 << setw(12) << p.groups[s.batch].name
                 << p.rooms[a[i].room].name << '\n';
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
    for (int slot = 0; slot < SLOTS_PER_WEEK; ++slot)
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
                  << csvCell(p.groups[s.batch].name) << ','
                  << csvCell(p.rooms[a[i].room].name) << '\n';
            }
    f.close();
    return !f.fail();
}

void demo(bool impossible, const string &csvPath = "")
{
    Problem p = csvPath.empty() ? sample() : fromCSVFile(csvPath);
    showInput(p, !csvPath.empty());
    if (impossible)
    {
        p.available[0].assign(SLOTS_PER_WEEK, false);
        cout << "\nIMPOSSIBLE CASE: Teacher A is unavailable all week.\n";
    }

    Scheduler engine(p);
    if (!engine.generate())
    {
        cout << "No valid timetable exists for these demo rules (search exhausted).\n";
        return;
    }
    if (!validate(p, engine.placements()))
        throw runtime_error("Internal validation failed.");

    cout << "\nSUCCESS: all sessions assigned. Hard-constraint violations: 0.\n";
    show(p, engine.placements());
    cout << "\nBSSE-17 view:\n";
    show(p, engine.placements(), 0);
    cout << "\nBSSE-18 view:\n";
    show(p, engine.placements(), 1);
    cout << "\nCandidate attempts: " << engine.attemptCount()
         << "; backtracks: " << engine.backtrackCount() << '\n';
    cout << "This is the first valid solution; preferences are not optimized yet.\n";
    cout << "Room capacity and computer-lab suitability are enforced.\n";
    cout << (exportCSV(p, engine.placements())
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
    check(s.generate() && validate(p, s.placements()), "normal schedule covers all sessions");
    auto broken = s.placements();
    broken[0].slot = 0;
    check(!validate(p, broken), "unavailable teacher rejected");
    broken = s.placements();
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

    p.available[0].assign(SLOTS_PER_WEEK, false);
    Scheduler impossible(p);
    check(!impossible.generate(), "impossible timetable handled");

    // Flexible class initially takes 09:00, but Restricted needs 09:00.
    Problem q;
    q.sessions = {{"Flexible", 0, 0}, {"Restricted", 1, 0}};
    q.available[0].assign(SLOTS_PER_WEEK, false);
    q.available[1].assign(SLOTS_PER_WEEK, false);
    q.available[0][0] = q.available[0][1] = true;
    q.available[0][2] = q.available[0][3] = true;
    q.available[1][0] = q.available[1][1] = true;
    Scheduler retry(q);
    check(retry.generate() && retry.backtrackCount() > 0 && validate(q, retry.placements()),
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
            p.sessions[idx].durationUnits == 4 &&
            s.placements()[idx].slot % UNITS_PER_DAY == LAB_START_UNIT)
            sampleLabAt14 = true;
    check(sampleLabAt14, "solver places the two-hour sample lab at 14:00");
    check(clockTime(16) == "17:00" && clockTime(10) == "14:00",
          "clock formatting handles lab end time");

    Problem one;
    one.sessions = {{"90-minute Theory", 0, 0, 3, false}};
    Scheduler ninety(one);
    check(ninety.generate() && validate(one, ninety.placements()),
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
    check(avoidUnavailable.generate() && validate(unavailableMid, avoidUnavailable.placements()) &&
              avoidUnavailable.placements()[0].slot != 0,
          "solver avoids unavailable interior time unit");

    Problem boundary;
    boundary.sessions = {{"Theory", 0, 0, 2, false}};
    check(!validate(boundary, {{7, 0}}),
          "validator rejects theory crossing lunch break");

    Problem lab;
    lab.sessions = {{"Lab", 0, 0, 6, true}};
    check(!validate(lab, {{9, 0}}) && !validate(lab, {{11, 0}}),
          "validator rejects lab outside 14:00-17:00");
    lab.sessions[0].durationUnits = 7;
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
        if (session.isLab && session.durationUnits == 4)
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
        {"Programming", 2, 1, 6, 2, {60, 60, 60}, {120}}};
    vector<Session> combinedSessions = generateRequiredSessions(combined, CreditPolicy{});
    check(combinedSessions.size() == 4 && combinedSessions.back().isLab &&
              combinedSessions.back().durationUnits == 4 &&
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
    check(rejectsRequirements({{"Bad Lab", 0, 0, 0, 2, {}, {180}}}, CreditPolicy{}),
          "one lab credit rejects the old 180-minute pattern");
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

    // M2 Step 2: strict file-format parsing and integration with the existing solver.
    const string header =
        "course,teacher_id,batch_id,theory_half_credits,lab_half_credits,"
        "theory_pattern_minutes,lab_pattern_minutes\n";
    istringstream validCsv(header +
        "C++,0,0,4,2,60;60,120\n"
        "Math,1,0,3,0,90,\n"
        "Algorithms,0,1,6,0,60;60;60,\n"
        "English,2,1,4,0,60;60,\n");
    vector<CourseRequirement> loaded = loadCoursesCSV(validCsv);
    check(loaded.size() == 4 && loaded[0].course == "C++" &&
          loaded[1].theoryPatternMinutes[0] == 90,
          "CSV parser loads courses and session patterns");
    vector<Session> loadedSessions = generateRequiredSessions(loaded, CreditPolicy{});
    check(loadedSessions.size() == 9 && loadedSessions[2].isLab &&
              loadedSessions[2].durationUnits == 4 && loadedSessions[3].durationUnits == 3,
          "CSV generates nine sessions including a two-hour lab and 90-minute theory");
    Problem csvProblem;
    csvProblem.courses = loaded;
    csvProblem.sessions = loadedSessions;
    Scheduler csvSolver(csvProblem);
    check(csvSolver.generate() && validate(csvProblem, csvSolver.placements()),
          "CSV-derived course requirements produce a valid timetable");

    auto rejectsCsv = [&](const string &data)
    {
        try
        {
            istringstream in(data);
            (void)loadCoursesCSV(in);
        }
        catch (const invalid_argument &)
        {
            return true;
        }
        return false;
    };
    check(rejectsCsv(""), "empty CSV rejected");
    check(rejectsCsv(header), "header-only CSV rejected");
    check(rejectsCsv("course,teacher,batch\nA,0,0\n"), "incorrect CSV header rejected");
    check(rejectsCsv(header + "A,0,0,2,0,60\n"), "missing CSV column rejected");
    check(rejectsCsv(header + "A,zero,0,2,0,60,\n"), "nonnumeric teacher ID rejected");
    check(rejectsCsv(header + "A,0,0,2,0,60; ,\n"), "invalid session pattern token rejected");
    istringstream windowsCsv(header + "A,1,0,2,0,60,\r\n");
    check(loadCoursesCSV(windowsCsv).size() == 1,
          "Windows CRLF course CSV accepted");

    // M3 Step 1: room capacity is a HARD constraint for solver AND validator.
    check(defaultRooms.size() == 2 && defaultRooms[0].type == RoomType::ComputerLab &&
              defaultRooms[1].type == RoomType::Classroom &&
              roomTypeName(defaultRooms[0].type) == "Computer lab",
          "room types and capacities are represented");
    check(defaultGroups.size() == 2 && defaultGroups[0].size == 32 &&
              defaultRooms[0].capacity == 40,
          "student group sizes and room capacities are represented");

    Problem tooSmall;
    tooSmall.sessions = {{"Theory", 0, 0}};
    tooSmall.rooms[0].capacity = 20;
    tooSmall.rooms[1].capacity = 35; // Alternate room can fit all 32 students.
    check(!validate(tooSmall, {{0, 0}}),
          "validator rejects room below student group size");
    check(validate(tooSmall, {{0, 1}}),
          "validator allows room with enough seats");

    Scheduler findsRoom(tooSmall);
    check(findsRoom.generate() && validate(tooSmall, findsRoom.placements()) &&
              findsRoom.placements()[0].room == 1,
          "solver skips undersized room and selects fitting room");

    Problem exactlyFits;
    exactlyFits.sessions = {{"Theory", 0, 0}};
    exactlyFits.rooms[0].capacity = exactlyFits.groups[0].size;
    check(validate(exactlyFits, {{0, 0}}),
          "room capacity equal to group size is allowed");

    Problem noRoom;
    noRoom.sessions = {{"Theory", 0, 0}};
    noRoom.rooms[0].capacity = 20;
    noRoom.rooms[1].capacity = 25;
    Scheduler noRoomSolver(noRoom);
    check(!noRoomSolver.generate() && !validate(noRoom, {{0, 0}}) &&
              !validate(noRoom, {{0, 1}}),
          "solver rejects scenario with no sufficiently large room");

    Problem badCapacity;
    badCapacity.sessions = {{"Theory", 0, 0}};
    badCapacity.rooms[0].capacity = 0;
    check(!validate(badCapacity, {{0, 0}}),
          "validator rejects zero-capacity room");

    Problem badGroup;
    badGroup.sessions = {{"Theory", 0, 0}};
    badGroup.groups[0].size = 0;
    check(!validate(badGroup, {{0, 0}}),
          "validator rejects invalid student group size");

    Problem bigGroup;
    bigGroup.sessions = {{"Theory", 0, 0}};
    bigGroup.groups[0].size = 41;
    Scheduler bigGroupSolver(bigGroup);
    check(!bigGroupSolver.generate(),
          "solver refuses group larger than every room");

    // M3 Step 2: lab-room suitability is a HARD constraint in both paths.
    Session theorySession = {"Theory", 0, 0, 2, false};
    Session labSession = {"Lab", 0, 0, 6, true};
    check(roomSupportsSession(defaultRooms[1], theorySession),
          "classroom supports theory sessions");
    check(roomSupportsSession(defaultRooms[0], theorySession),
          "computer lab may host theory sessions");
    check(roomSupportsSession(defaultRooms[0], labSession),
          "computer lab supports laboratory sessions");
    check(!roomSupportsSession(defaultRooms[1], labSession),
          "ordinary classroom is unsuitable for laboratory sessions");

    Problem singleLab;
    singleLab.sessions = {labSession};
    check(validate(singleLab, {{LAB_START_UNIT, 0}}),
          "validator accepts afternoon lab in computer laboratory");
    check(!validate(singleLab, {{LAB_START_UNIT, 1}}),
          "validator rejects afternoon lab in ordinary classroom");

    Problem classroomFirst = singleLab;
    classroomFirst.rooms = {defaultRooms[1], defaultRooms[0]};
    Scheduler laboratoryFinder(classroomFirst);
    check(laboratoryFinder.generate() &&
              validate(classroomFirst, laboratoryFinder.placements()) &&
              laboratoryFinder.placements()[0].room == 1,
          "solver skips first classroom and selects computer laboratory");

    Problem noComputerLab = singleLab;
    noComputerLab.rooms = {defaultRooms[1], {"Room-103", 60, RoomType::Classroom}};
    Scheduler noLabRoom(noComputerLab);
    check(!noLabRoom.generate() && !validate(noComputerLab, {{LAB_START_UNIT, 0}}),
          "solver rejects lab when all rooms are classrooms");

    Problem undersizedLab = singleLab;
    undersizedLab.rooms[0].capacity = 30; // BSSE-17 has 32 students.
    Scheduler smallComputerLab(undersizedLab);
    check(!smallComputerLab.generate() &&
              !validate(undersizedLab, {{LAB_START_UNIT, 0}}),
          "computer lab must also satisfy student group capacity");

    // Verify that the original combined-course demo still uses a suitable lab.
    Problem demoWithLab = sample();
    Scheduler labDemo(demoWithLab);
    bool correctlyPlacedLab = labDemo.generate() &&
                               validate(demoWithLab, labDemo.placements());
    if (correctlyPlacedLab)
        for (size_t i = 0; i < demoWithLab.sessions.size(); ++i)
            if (demoWithLab.sessions[i].isLab &&
                !roomSupportsSession(demoWithLab.rooms[labDemo.placements()[i].room],
                                     demoWithLab.sessions[i]))
                correctlyPlacedLab = false;
    check(correctlyPlacedLab,
          "combined theory and laboratory demo obeys room suitability");

    Problem lateLab;
    lateLab.sessions = {{"Two-Hour Lab", 0, 0, 4, true}};
    lateLab.available[0][LAB_START_UNIT] = false;
    lateLab.available[0][LAB_START_UNIT + 1] = false;
    Scheduler lateLabSolver(lateLab);
    check(lateLabSolver.generate() && validate(lateLab, lateLabSolver.placements()) &&
              lateLabSolver.placements()[0].slot == LAB_START_UNIT + 2 &&
              !validate(lateLab, {{LAB_START_UNIT + 3, 0}}),
          "two-hour lab can start at 15:00 but cannot extend past 17:00");

    Problem splitLab;
    splitLab.sessions = generateRequiredSessions(
        {{"Two-Credit Lab", 0, 0, 0, 4, {}, {120, 120}}}, CreditPolicy{});
    Scheduler splitLabSolver(splitLab);
    check(splitLab.sessions.size() == 2 && splitLabSolver.generate() &&
              validate(splitLab, splitLabSolver.placements()) &&
              splitLabSolver.placements()[0].slot / UNITS_PER_DAY !=
                  splitLabSolver.placements()[1].slot / UNITS_PER_DAY,
          "two lab credits can be scheduled as two approved two-hour sessions");

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
