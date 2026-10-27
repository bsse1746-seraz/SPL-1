// TimeTableX Mini: a deliberately small C++17 SPL-1 learning prototype.
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdexcept>
using namespace std;

const vector<string> days = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday"};
const vector<string> periods = {"09:00", "10:00", "11:00"};
const vector<string> teachers = {"Teacher A", "Teacher B", "Teacher C"};
const vector<string> batches = {"BSSE-17", "BSSE-18"};
const vector<string> rooms = {"Room-101", "Room-102"};
const int slots = 15; // slot = day index * 3 + period index

struct Session { string course; int teacher, batch; };
struct Placement { int slot = -1, room = -1; };
struct Problem {
    vector<Session> sessions;
    vector<vector<bool>> available = vector<vector<bool>>(3, vector<bool>(slots, true));
};

Problem sample() {
    Problem p;
    // Each row is ONE one-hour class; repeated rows mean multiple weekly classes.
    p.sessions = {{"C++",0,0}, {"C++",0,0}, {"Math",1,0}, {"Math",1,0},
                  {"Algorithms",0,1}, {"Algorithms",0,1}, {"English",2,1}, {"English",2,1}};
    p.available[0][0] = false; // Teacher A unavailable Sunday at 09:00.
    return p;
}

class Scheduler {
    const Problem& p;
    bool canPlace(int i, int slot, int room) const {
        const auto& s = p.sessions[i];
        if (!p.available[s.teacher][slot]) return false;
        for (int j = 0; j < i; ++j) {
            if (result[j].slot != slot) continue;
            const auto& other = p.sessions[j];
            if (s.teacher == other.teacher || s.batch == other.batch || result[j].room == room)
                return false;
        }
        return true;
    }
    bool search(int i) {
        if (i == static_cast<int>(p.sessions.size())) return true;
        for (int slot = 0; slot < slots; ++slot) {
            for (int room = 0; room < static_cast<int>(rooms.size()); ++room) {
                ++attempts;
                if (!canPlace(i, slot, room)) continue;
                result[i] = {slot, room};              // Make a choice.
                if (search(i + 1)) return true;         // Try the next class.
                result[i] = {-1, -1};                   // Undo a dead end.
                ++backtracks;
            }
        }
        return false;
    }
public:
    vector<Placement> result;
    long long attempts = 0, backtracks = 0;
    explicit Scheduler(const Problem& problem) : p(problem) {}
    bool generate() {
        result.assign(p.sessions.size(), Placement{});
        attempts = backtracks = 0;
        return search(0);
    }
};

// Independent final check: does not call the scheduler's canPlace().
bool validate(const Problem& p, const vector<Placement>& a) {
    if (a.size() != p.sessions.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i].slot < 0 || a[i].slot >= slots || a[i].room < 0 ||
            a[i].room >= static_cast<int>(rooms.size())) return false;
        if (!p.available[p.sessions[i].teacher][a[i].slot]) return false;
        for (size_t j = 0; j < i; ++j) {
            if (a[i].slot == a[j].slot &&
                (a[i].room == a[j].room || p.sessions[i].teacher == p.sessions[j].teacher ||
                 p.sessions[i].batch == p.sessions[j].batch)) return false;
        }
    }
    return true;
}

void showInput(const Problem& p) {
    cout << "\nINPUT: " << p.sessions.size() << " one-hour sessions, 3 teachers, 2 batches, 2 rooms.\n";
    for (size_t i = 0; i < p.sessions.size(); ++i) {
        auto s = p.sessions[i];
        cout << i+1 << ". " << s.course << " | " << teachers[s.teacher] << " | " << batches[s.batch] << '\n';
    }
    cout << "Teacher A unavailable: Sunday 09:00. All other slots available in normal demo.\n";
}

void show(const Problem& p, const vector<Placement>& a, int batch = -1) {
    cout << '\n' << left << setw(12) << "Day" << setw(8) << "Time" << setw(14) << "Course"
         << setw(13) << "Teacher" << setw(12) << "Batch" << "Room\n";
    for (int slot = 0; slot < slots; ++slot)
        for (size_t i = 0; i < a.size(); ++i) {
            auto s = p.sessions[i];
            if (a[i].slot != slot || (batch != -1 && s.batch != batch)) continue;
            cout << setw(12) << days[slot/3] << setw(8) << periods[slot%3] << setw(14) << s.course
                 << setw(13) << teachers[s.teacher] << setw(12) << batches[s.batch] << rooms[a[i].room] << '\n';
        }
}

string csvCell(const string& value) {
    string out = "\"";
    for (char c : value) { if (c == '"') out += '"'; out += c; }
    return out + "\"";
}
bool exportCSV(const Problem& p, const vector<Placement>& a) {
    ofstream f("timetable.csv");
    f << "Day,Time,Course,Teacher,Batch,Room\n";
    for (int slot = 0; slot < slots; ++slot)
        for (size_t i = 0; i < a.size(); ++i) if (a[i].slot == slot) {
            auto s = p.sessions[i];
            f << csvCell(days[slot/3]) << ',' << csvCell(periods[slot%3]) << ',' << csvCell(s.course)
              << ',' << csvCell(teachers[s.teacher]) << ',' << csvCell(batches[s.batch])
              << ',' << csvCell(rooms[a[i].room]) << '\n';
        }
    f.close();
    return !f.fail();
}

void demo(bool impossible) {
    Problem p = sample();
    showInput(p);
    if (impossible) {
        p.available[0].assign(slots, false);
        cout << "\nIMPOSSIBLE CASE: Teacher A is now unavailable for the entire week.\n";
    }
    Scheduler engine(p);
    if (!engine.generate()) {
        cout << "No valid timetable exists for these demo rules (search exhausted).\n";
        return;
    }
    if (!validate(p, engine.result)) throw runtime_error("Internal validation failed.");
    cout << "\nSUCCESS: all sessions assigned. Hard-constraint violations: 0.\n";
    show(p, engine.result);
    cout << "\nBSSE-17 view:\n"; show(p, engine.result, 0);
    cout << "\nBSSE-18 view:\n"; show(p, engine.result, 1);
    cout << "\nCandidate attempts: " << engine.attempts << "; backtracks: " << engine.backtracks << '\n';
    cout << "This is the first valid solution; weekly balance is not optimized yet.\n";
    cout << (exportCSV(p, engine.result) ? "Saved timetable.csv in the working folder.\n" : "Could not save timetable.csv.\n");
}

void selfTest() {
    int count = 0;
    auto check = [&](bool ok, const string& name) {
        if (!ok) throw runtime_error("FAILED: " + name);
        cout << "PASS: " << name << '\n'; ++count;
    };
    Problem p = sample(); Scheduler s(p);
    check(s.generate() && validate(p,s.result), "normal schedule covers all sessions");
    auto broken = s.result; broken[0].slot = 0;
    check(!validate(p,broken), "unavailable teacher rejected");
    broken = s.result; broken.pop_back();
    check(!validate(p,broken), "missing session rejected");
    auto pairCase = [&](Session a, Session b, int roomA, int roomB) {
        Problem q; q.sessions = {a,b};
        return validate(q, {{1,roomA},{1,roomB}});
    };
    check(!pairCase({"A",0,0},{"B",0,1},0,1), "teacher clash rejected");
    check(!pairCase({"A",0,0},{"B",1,0},0,1), "batch clash rejected");
    check(!pairCase({"A",0,0},{"B",1,1},0,0), "room clash rejected");
    check(pairCase({"A",0,0},{"B",1,1},0,1), "independent simultaneous classes allowed");
    p.available[0].assign(slots,false); Scheduler impossible(p);
    check(!impossible.generate(), "impossible timetable handled");
    // First class initially takes slot 0; second class requires slot 0.
    // A correct backtracker must undo that first choice and move it to slot 1.
    Problem q; q.sessions = {{"Flexible",0,0},{"Restricted",1,0}};
    q.available[0].assign(slots,false); q.available[1].assign(slots,false);
    q.available[0][0] = q.available[0][1] = q.available[1][0] = true;
    Scheduler retry(q);
    check(retry.generate() && retry.backtracks > 0 && validate(q,retry.result), "dead end undone by backtracking");
    cout << count << " tests passed.\n";
}

int main(int argc, char* argv[]) {
    try {
        cout << "TIMETABLEX MINI | C++17 | SPL-1 prototype\n";
        if (argc > 1) {
            string arg = argv[1];
            if (arg == "--demo") demo(false);
            else if (arg == "--impossible") demo(true);
            else if (arg == "--test") selfTest();
            else { cerr << "Use --demo, --impossible or --test.\n"; return 1; }
            return 0;
        }
        string input;
        while (true) {
            cout << "\n1. Generate sample timetable\n2. Try impossible case\n3. Run checks\n0. Exit\nChoice: ";
            if (!getline(cin,input) || input == "0") break;
            if (input == "1") demo(false);
            else if (input == "2") demo(true);
            else if (input == "3") selfTest();
            else cout << "Enter 0, 1, 2 or 3.\n";
        }
    } catch (const exception& e) { cerr << e.what() << '\n'; return 1; }
    return 0;
}
