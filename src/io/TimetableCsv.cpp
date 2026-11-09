#include "timetablex/io/TimetableCsv.hpp"

#include "timetablex/core/SchedulingRules.hpp"

#include <cctype>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace timetablex
{

using std::getline;
using std::invalid_argument;
using std::istringstream;
using std::isspace;
using std::istream;
using std::ofstream;
using std::out_of_range;
using std::size_t;
using std::stoi;
using std::string;
using std::to_string;
using std::vector;

namespace
{

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

} // namespace

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

bool exportCSV(const Problem &p, const vector<Placement> &a, const string &path)
{
    ofstream f(path);
    if (!f)
        return false;

    f << "Day,Start,End,Course,Type,Teacher,Batch,Room\n";
    for (int slot = 0; slot < SLOTS_PER_WEEK; ++slot)
        for (size_t i = 0; i < a.size(); ++i)
            if (a[i].slot == slot)
            {
                const Session &s = p.sessions[i];
                int start = slot % UNITS_PER_DAY;
                f << csvCell(DAY_NAMES[slot / UNITS_PER_DAY]) << ','
                  << csvCell(clockTime(start)) << ','
                  << csvCell(clockTime(start + s.durationUnits)) << ','
                  << csvCell(s.course) << ','
                  << csvCell(s.isLab ? "Lab" : "Theory") << ','
                  << csvCell(p.teacherNames[s.teacher]) << ','
                  << csvCell(p.groups[s.batch].name) << ','
                  << csvCell(p.rooms[a[i].room].name) << '\n';
            }
    f.close();
    return !f.fail();
}

} // namespace timetablex
