# TimeTableX — M3: Room Capacity and Laboratory Suitability

**Project:** TimeTableX — Automated University Timetable Generator and Optimizer  
**Language:** C++17  
**Development checkpoints:** M3 Step 1 (`3de150f`) and M3 Step 2 (`4f6d987`)  
**Verified on the development machine:** 72/72 automated tests passed, followed by a successful nine-session CSV demonstration with zero reported hard-constraint violations.

## 1. The problem M3 solves

A timetable can be free of teacher, student-group, and room-time clashes but still be unusable. For example, a 32-student group cannot fit into a 30-seat classroom; a three-hour C++ computer-lab class cannot take place in an ordinary classroom without the required computing facilities.

M3 introduces **two hard constraints**. A hard constraint must always hold; an invalid assignment is rejected rather than accepted with a penalty.

1. **Capacity:** `room.capacity >= group.size`, with both values positive.
2. **Room suitability:** a session marked `isLab = true` requires `RoomType::ComputerLab`. Theory may use a regular classroom **or** a computer laboratory, subject to capacity and other rules.

The implementation does **not** yet optimize room utilization, teacher preferences, or timetable spread. Those are separate later milestones.

## 2. New data models

```cpp
enum class RoomType { Classroom, ComputerLab };

struct Room {
    string name;
    int capacity;
    RoomType type;
};

struct StudentGroup {
    string name;
    int size;
};
```

### Demonstration inventory

| Resource | Type | Seats / students |
|---|---|---:|
| Room-101 | Computer lab | 40 seats |
| Room-102 | Classroom | 30 seats |
| BSSE-17 | Student group | 32 students |
| BSSE-18 | Student group | 26 students |

**Worked example:** BSSE-17 has 32 students. Room-102 has only 30 seats, so BSSE-17 may not use that room. Room-101 has 40 seats, so it passes the capacity rule. For a C++ lab, Room-101 also passes the room-type rule.

The inventory is currently **hard-coded fictional demonstration data** in `defaultRooms` and `defaultGroups` within `prototype/main.cpp`; it is not yet loaded from a rooms CSV file.

## 3. Source-code rule for room type

The helper `roomSupportsSession()` implements the type requirement:

```cpp
bool roomSupportsSession(const Room &room, const Session &session)
{
    if (session.isLab)
        return room.type == RoomType::ComputerLab;
    return room.type == RoomType::Classroom ||
           room.type == RoomType::ComputerLab;
}
```

This helper checks **type only**. Seat capacity is deliberately verified separately in the scheduler and validator.

| Session | Classroom | Computer lab |
|---|---|---|
| Theory | Allowed if enough seats | Allowed if enough seats |
| Lab (`isLab = true`) | Rejected | Allowed if enough seats |

The current `isLab` flag means the current project's **computer laboratory** requirement. Different kinds of laboratories (electronics, chemistry, physics, etc.) would need an expanded room-capability model in a future version.

## 4. How backtracking enforces the hard constraints

`Scheduler::search(i)` tries each permitted weekly start slot and room for session `i`. Before selecting a candidate, it calls `Scheduler::canPlace(i, slot, room)`.

In `canPlace()`, after checking valid indices, M3 rejects the candidate when:

```cpp
if (!roomSupportsSession(p.rooms[room], s) ||
    p.groups[s.batch].size <= 0 ||
    p.rooms[room].capacity <= 0 ||
    p.groups[s.batch].size > p.rooms[room].capacity)
    return false;
```

The same function also enforces the earlier hard constraints: correct academic window, teacher availability for every occupied 30-minute unit, and overlapping teacher/batch/room bookings.

### Backtracking sequence

1. Choose the next required session in its existing input order.
2. Try possible start slots and rooms.
3. Reject any candidate violating hard constraints (including M3 rules).
4. Tentatively assign a valid candidate and recursively schedule the next session.
5. If deeper scheduling fails, undo the choice and try another placement.
6. Stop at the **first fully valid timetable** or report that no complete assignment was found.

**Important distinction:** the current solver uses straightforward recursive backtracking. MRV, forward checking, and weighted soft-constraint optimization are **not yet implemented**.

## 5. Why use an independent validator?

Even if the scheduler believes a timetable is correct, a separate check helps catch defects, corrupted output, or manually altered placements.

`validate(const Problem&, const vector<Placement>&)` independently checks every assigned session. It **does not call** `Scheduler::canPlace()`; instead, it repeats the relevant room-type and capacity requirements using its own checks, together with the time, availability, and conflict checks.

For example, a schedule that manually assigns the 180-minute C++ lab to Room-102 is rejected even if the timetable contains no overlapping classes.

## 6. M3 automated test coverage

### Step 1 — capacity checks (10 tests)

- The room types and capacities are represented.
- Student-group sizes and room capacities are represented.
- Validator rejects a room that is too small.
- Validator accepts a sufficiently large room.
- Solver skips an undersized room and finds another.
- A room with capacity exactly equal to group size is permitted.
- The solver reports failure when no room fits.
- Zero-capacity rooms are rejected.
- Invalid (zero) group size is rejected.
- A group larger than every room cannot be scheduled.

### Step 2 — laboratory room suitability (10 tests)

- A classroom supports a theory session.
- A computer laboratory supports a theory session.
- A computer laboratory supports a lab session.
- An ordinary classroom does not support a lab session.
- The validator accepts a correctly timed lab in a computer lab.
- The validator rejects a lab in an ordinary classroom.
- The solver skips a classroom and selects a computer lab.
- The solver rejects labs when every room is a classroom.
- A computer lab with insufficient seats is still rejected.
- The combined theory-and-lab sample remains valid.

**Regression outcome:** 52 pre-existing tests + 10 capacity tests + 10 suitability tests = **72 tests passed** at the M3 Step 2 checkpoint.

### Reproduce the verification in VS Code (PowerShell)

Run from the repository root:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic prototype/main.cpp -o prototype/timetablex.exe
if ($LASTEXITCODE -eq 0) { .\prototype\timetablex.exe --test }
```

Demonstrate course CSV scheduling:

```powershell
.\prototype\timetablex.exe --demo-file data/courses.csv
```

The tested CSV demonstration scheduled **nine sessions**, reported **zero hard-constraint violations**, and placed the C++ laboratory on **Sunday 14:00–17:00 in Room-101**. The generated schedule is a feasibility demonstration, not an optimized timetable.

## 7. What did not change?

- Teacher-to-course assignments remain **fixed** by the department.
- University timetable days are Sunday–Thursday.
- Theory sessions fit inside 09:00–13:00; 13:00–14:00 remains a break.
- The current lab pattern is one uninterrupted 180-minute block from 14:00 to 17:00.
- Time resolution remains 30 minutes.
- Course requirements and session patterns can still be read from `data/courses.csv`.
- Conflict detection still checks **entire overlapping durations**, not just start times.

## 8. Viva questions and brief answers

**Q1. Why is capacity a hard constraint?**  
Because there must be enough seats for every student. A smaller room is physically unsuitable regardless of preference score.

**Q2. Can a theory class use a computer lab?**  
Yes, if the computer lab has enough capacity and is free. This is the current project's defined policy.

**Q3. Can a C++ lab use a large ordinary classroom?**  
No. Room suitability is different from capacity. A large classroom still does not satisfy the computer-lab requirement.

**Q4. Where do you check those rules?**  
`Scheduler::canPlace()` prevents invalid assignments during search. `validate()` checks a completed timetable independently.

**Q5. Does the program assign teachers automatically?**  
No. The teacher is fixed in each course requirement. The solver chooses only a permissible time and room for each session.

**Q6. What is the difference between `roomSupportsSession()` and the capacity check?**  
The helper checks the room *type*; the capacity comparison checks whether the room has enough seats. Both must pass.

**Q7. Why are so many classes on Sunday?**  
The solver currently accepts the first valid combination. It has not yet implemented teacher preference scoring or timetable optimization.

**Q8. What happens if no computer lab is available?**  
The solver cannot place a required laboratory session and may report that no complete valid timetable exists.

**Q9. Does TimeTableX support all laboratory categories?**  
Not yet. The current model distinguishes only `Classroom` and `ComputerLab`.

**Q10. What comes next?**  
Teacher preferred days/times, hard unavailability as structured input, weighted penalties for soft preferences, and stronger search heuristics as later milestones.

## 9. Known limitations and next steps

The classroom and computer-lab inventory is built into the program; teacher preference CSV and room inventory CSV are not implemented. The room type model is simple, with no equipment list or specialist lab types. The solver checks feasibility but does not yet optimize placement quality, minimize day gaps, or apply MRV / forward checking. For larger departments, those additions and modular source files will become important.

**Commits:** `3de150f` (M3 capacity), `4f6d987` (M3 lab suitability).  
**Next milestone:** M4 — Teacher Preferences and Soft-Constraint Scoring.
