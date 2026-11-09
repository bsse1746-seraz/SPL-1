# Midterm demonstration

**TimeTableX · Seraz Alam Raihan (BSSE1746) · 12 November 2026**

## Run from the repository root

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1
.\build\timetablex.exe --test
.\build\timetablex.exe --demo-file data/courses.csv
Get-Content .\timetable.csv
.\build\timetablex.exe --demo
.\build\timetablex.exe --impossible
```

1. Show the test summary `74 tests passed.` Point out the forced backtracking, interior availability, and undersized computer-lab checks.
2. Open `data/courses.csv` and explain the C++ row. Two theory credits create two one-hour sessions. One lab credit creates one two-hour session.
3. Show the CSV demo's nine sessions and zero hard-constraint violations. The lab uses Room-101 on Sunday 14:00–16:00. The Math session takes 90 minutes. The department and batch tables show the same underlying placements.
4. Run the built-in demo to show Teacher A's Sunday 09:00–09:30 restriction. Run the impossible case to show what happens when Teacher A has no availability all week.

Keep the verified M3 checkpoint available for the demo. It builds from its own worktree with `g++ -std=c++17 -Wall -Wextra -Wpedantic prototype/main.cpp -o timetablex.exe`.

## Likely viva questions

| Question | Short answer |
| --- | --- |
| How did you split the old source? | I separated data structs, rules, session creation, search, validation, CSV, the scheduling service, CLI, and tests. `src/main.cpp` delegates to the console. |
| Why is a session different from a course? | One course can need several weekly meetings. A `Session` is one meeting with one duration and a fixed teacher and batch. |
| What is a placement? | A weekly start slot and a room index for one session. Placements and sessions share the same vector index. |
| What does the search undo? | If deeper recursion fails, it clears its tentative slot and room for the current session, then tries the next candidate. |
| Why check availability at every half-hour? | A teacher could be free at the start and unavailable halfway through. The whole session has to fit. |
| Why keep the validator separate? | It reads the finished placements and checks coverage and constraints directly. It can reject a corrupt result without running the solver. |
| Where is encapsulation used? | `Scheduler` keeps tentative state and recursion private. The service exposes the application operation as one call. |
| Is this the best possible timetable? | It is the first valid one in the current search order. Preference scoring and optimization are future work. |
| How could a GUI use it? | Build the problem, call `SchedulingService::generate`, then draw the validated placements in a grid. |
