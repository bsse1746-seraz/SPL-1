# M2 Step 2 — Structured Course Input (CSV)

## Goal

Allow a departmental coordinator to define courses in a text file instead of rebuilding C++ whenever course credits or approved session patterns change. **Teacher assignments remain fixed.** The loaded requirements are converted into sessions, then passed to the existing backtracking scheduler and independent validator.

## File and demonstration command

- Input: `data/courses.csv`
- Source: `prototype/main.cpp`
- Run from the repository root: `./prototype/timetablex.exe --demo-file data/courses.csv` (on Windows PowerShell, use `.\prototype\timetablex.exe --demo-file data/courses.csv`).
- Test: `.\prototype\timetablex.exe --test` after rebuilding the executable.

The CSV must begin with this exact header:

```csv
course,teacher_id,batch_id,theory_half_credits,lab_half_credits,theory_pattern_minutes,lab_pattern_minutes
```

The current illustrative dataset:

```csv
course,teacher_id,batch_id,theory_half_credits,lab_half_credits,theory_pattern_minutes,lab_pattern_minutes
C++,0,0,4,2,60;60,180
Math,1,0,3,0,90,
Algorithms,0,1,6,0,60;60;60,
English,2,1,4,0,60;60,
```

**Reading one row:** `C++,0,0,4,2,60;60,180` fixes Teacher A (ID 0) to the BSSE-17 group (ID 0); 4 half-credit units mean 2.0 theory credits and 2 half-credit units mean 1.0 lab credit. The approved pattern produces two 60-minute theory sessions and one 180-minute lab. Credit conversions are examples, not verified official IIT-DU rules.

## How the C++ data flows

1. `fromCSVFile(path)` opens the input file and calls `loadCoursesCSV(input)`.
2. `loadCoursesCSV()` reads and validates the header, splits each nonblank data row into seven columns, and builds `CourseRequirement` records. It includes line information in certain validation errors.
3. `parseNonnegativeInt()` validates integer fields; `parseMinutesPattern()` converts `60;60` into two 60-minute durations.
4. `generateRequiredSessions(courses, policy)` checks assigned teacher/group references and compares the total approved session minutes against the contact hours derived from credit units.
5. Existing `Scheduler` assigns each `Session` to an allowed time and room; independent `validate()` checks its final placement.
6. The demonstration prints department data, the generated sessions, the timetable, and a CSV export (`timetable.csv`).

**Key design principle:** The parser reads *requirements*, not a finished timetable. Scheduling remains the responsibility of the CSP/backtracking engine.

## Validation and limitations

The input parser rejects an empty file, header-only input, incorrect header, wrong column counts, nonnumeric IDs, and invalid session-pattern tokens. It accepts Windows CRLF line endings. Session generation also verifies credits and component durations. The parser is intentionally simple: **it does not support quoted CSV fields or commas inside course names**. Teacher IDs are currently 0–2, batch IDs 0–1, and room data are still built into the prototype.

In the `--demo-file` path, **teacher availability defaults to unrestricted**. Teacher preference files, room suitability and capacity, section combinations, and optimization are **not implemented** at this checkpoint.

## Verification checkpoint — November 4, 2026

- User ran the C++17 program on Windows and reported **52/52 tests passed** (42 earlier tests + 10 CSV-related tests).
- User ran `--demo-file data/courses.csv`: four courses generated **nine sessions**; independent validation reported **zero hard-constraint violations**.
- Demonstration included a 90-minute Mathematics theory session (11:00–12:30) and a three-hour C++ lab (14:00–17:00).
- Search printed **101 candidate attempts and 0 backtracks** on this particular dataset. Zero backtracks means the first search path was sufficient, not that backtracking is absent.
- Code and `data/courses.csv` were pushed to `feature/lab-scheduling` in commit `25c06fc`.

These are historical test results from the user's run. Future edits require fresh test execution.

## Questions to prepare for viva

**Q: Why use CSV instead of editing `main.cpp` for every course?**  
A: A coordinator can change structured input data without recompiling the scheduling logic. The program separates academic requirements from algorithm implementation.

**Q: Why represent credits as half-credit integers?**  
A: An integer such as 3 represents 1.5 credits. It avoids floating-point equality problems when comparing credit requirements against approved minute totals.

**Q: Does the scheduler automatically choose course teachers?**  
A: No. The CSV stores the teacher ID assigned by the department. The scheduler chooses only when and where each required session runs.

**Q: What if a 1.5-credit theory course requests only a 60-minute class under the example policy?**  
A: The session-generation validation rejects it because the approved session pattern does not match the required 90 minutes.

**Q: Does a passing CSV test mean the system is production-ready?**  
A: No. The parser has a deliberately restricted format, and availability, room suitability, and preference optimization still require development.
