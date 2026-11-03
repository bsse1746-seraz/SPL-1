# M2 Step 1 — Credit-Based Session Generation

**Project:** TimeTableX (C++17, SPL-1)  
**Milestone:** M2, Step 1  
**Source:** `prototype/main.cpp`  
**Feature commit:** `fceca25` on `feature/lab-scheduling`

## 1. Problem and motivation

M1 already placed *predefined* class sessions using recursive backtracking and full-duration conflict checks. However, in a department the coordinator defines courses, credits, assigned teachers, student groups, and approved weekly class patterns. A scheduler should create all required sessions **before** trying to place them.

This milestone adds the stage:

`Course requirements + credit policy -> validated weekly Session objects -> existing scheduler`

Teacher allocation is not part of the search. Each `CourseRequirement` already contains the department-assigned teacher.

## 2. Demonstration credit policy (provisional)

The current `CreditPolicy` defaults to:

| Component | Contact minutes for one credit | Example |
| --- | ---: | --- |
| Theory | 60 | 3.0 credits = 180 minutes/week |
| Laboratory | 180 | 1.0 credit = 180 minutes/week |

These are **example project values**, not a claim about IIT-DU regulations. The conversion factors can be configured.

The academic windows remain: theory **09:00–13:00**, break **13:00–14:00**, and three-hour labs **14:00–17:00**. A scheduling unit is **30 minutes**.

## 3. C++ data structures

`CourseRequirement` describes one course offering's input:

- `course`: course title.
- `teacher`, `batch`: existing integer IDs (fixed assignment; no teacher selection).
- `theoryHalfCredits`, `labHalfCredits`: credits stored in **half-credit units**.
- `theoryPatternMinutes`, `labPatternMinutes`: coordinator-approved durations for each weekly session.

`CreditPolicy` holds `theoryMinutesPerCredit` and `labMinutesPerCredit`.

`Session` is the actual unit the backtracking scheduler places. It contains course name, assigned teacher and batch, duration in **30-minute units**, and `isLab`.

### Why half-credit integers?

Instead of storing 1.5 as a floating-point value, the code stores it as 3 half-credit units:

| Academic credits | Stored integer |
| ---: | ---: |
| 1.0 | 2 |
| 1.5 | 3 |
| 2.0 | 4 |
| 3.0 | 6 |

This avoids equality problems caused by floating-point arithmetic in credit calculations. For a valid value:

`required minutes = halfCredits * minutesPerCredit / 2`

For example, 3.0 theory credits become `6 * 60 / 2 = 180` minutes.

## 4. Function-level explanation

**`requiredMinutes(...)`** validates the credit and policy inputs, checks that the multiplication gives an integral number of minutes after dividing by two, then calculates total required contact minutes. It throws `invalid_argument` for invalid inputs.

**`appendComponentSessions(...)`** checks every proposed session length:

1. It must be positive and divisible by 30 minutes.
2. It must fit the appropriate theory or laboratory time window.
3. The sum of its session lengths must equal the minutes required by its credits.
4. If valid, each session length becomes a `Session` with the same fixed teacher and batch.

**`generateRequiredSessions(...)`** processes all course requirements. It validates each course's name, assigned teacher ID, student group ID, and nonempty credit requirement. It then builds theory sessions and laboratory sessions separately and returns a `vector<Session>`.

**`sample()`** now describes course requirements and calls `generateRequiredSessions(...)` rather than manually listing each class session.

## 5. Example: combined course

A Programming course with **3.0 theory + 1.0 laboratory credits** can have approved patterns:

- Theory: `{60, 60, 60}` minutes.
- Lab: `{180}` minutes.

The generator creates **four** sessions: three one-hour theory classes and one three-hour lab. All four retain the course's fixed teacher and student group.

A pattern such as `{60, 60}` for 3.0 theory credits is rejected because it supplies only 120 of the required 180 minutes. Splitting a 180-minute lab into two 90-minute labs is also rejected by the current fixed 14:00–17:00 laboratory rule.

## 6. Verification evidence

On the student's Windows VS Code/PowerShell setup, the C++17 program's `--test` command reported **42 tests passed** after M2 Step 1. That included the preceding **32 M1 tests** and **10 new M2 tests**.

The new checks covered:

- Demo data generating nine sessions from four course requirements.
- 3.0 theory credits creating three one-hour sessions.
- 1.5 theory credits creating a 90-minute session.
- Combined theory/lab credit requirements preserving the assigned teacher and group.
- Rejection of contact-hour mismatches, invalid split labs, durations not aligned to the 30-minute grid, invalid instructor references, and empty credit requirements.
- Alternative, configurable theory credit-to-contact-minute policy.

### Reproduce locally

From the repository root in PowerShell:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic prototype/main.cpp -o prototype/timetablex.exe
if ($LASTEXITCODE -ne 0) { throw "Compilation failed" }
.\prototype\timetablex.exe --test
```

Expected result for this checkpoint: `42 tests passed.` Re-run the tests after subsequent changes; the earlier result is not proof about modified code.

## 7. Viva questions and short answers

**Q: Why not specify every weekly class manually?**  
A: Course credits and approved patterns already describe how much teaching is required. Generating sessions prevents missing a required class or accidentally adding extra sessions.

**Q: Does the scheduler select teachers?**  
A: No. The department fixes the teacher in `CourseRequirement`. Generated sessions retain that teacher; backtracking selects only valid time and room placements.

**Q: Why do you use `vector<Session>`?**  
A: The number of weekly sessions varies by course. A vector stores a variable number of session records and can be passed to the existing scheduling engine.

**Q: What if the submitted pattern does not match the credits?**  
A: The generator throws `invalid_argument`. It does not silently create an incomplete timetable.

**Q: Is the credit conversion an IIT-DU regulation?**  
A: No. The current 60-minute theory and 180-minute lab values are configurable demonstration rules. Actual departmental policy must be confirmed before production use.

**Q: Is M2 complete?**  
A: No. This step generates sessions from in-code course requirements. Reading requirements from external structured files and more robust input validation come next.

## 8. Known limitations / next steps

- Course requirements are currently entered in source code, not loaded from CSV or another file.
- Fixed demo arrays currently limit the number of teachers and student groups.
- Room capacity and lab facility suitability are not implemented yet.
- Teacher preference scoring, MRV, and forward checking are later milestones.
- The generator validates approved session patterns; it does **not** infer every possible session distribution automatically.
- No global optimality is claimed: backtracking currently stops at the first valid timetable.
