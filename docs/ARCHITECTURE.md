# TimeTableX architecture

## Modules

| Module | Responsibility |
| --- | --- |
| `models/Problem.hpp` | Plain data for courses, credits, sessions, rooms, groups, availability, and placements. |
| `core/SchedulingRules` | Shared calendar constants, interval checks, room suitability, and time labels. |
| `core/SessionGenerator` | Validates resource IDs and approved credit patterns, then creates weekly sessions. |
| `core/Scheduler` | Tries start slots and room IDs. Owns the working placements and search counters. |
| `core/Validator` | Rechecks a complete assignment independently of the solver's candidate checks. |
| `app/SchedulingService` | Runs the solver, invokes the validator on a complete assignment, and returns a structured result. |
| `io/TimetableCsv` | Parses course requirements from a stream and exports a validated timetable to a path. |
| `data/DemoData` | Supplies example rooms, groups, teacher names, and availability. Builds the demo problem from built-in data or CSV. |
| `src/main.cpp` and `src/cli/Console.cpp` | Start the CLI, handle menu and command-line options, and display or export the result. |
| `tests/SelfTests.cpp` | Runs the 74 original checks through the extracted modules. |

Public declarations live in `include/timetablex/`. The implementations live in `src/`. The CLI and its test entry point are private to the executable target.

## From a row to a placement

`C++,0,0,4,2,60;60,120` names C++, teacher 0, batch 0, four theory half-credits, and two lab half-credits. The parser stores that in a `CourseRequirement`. The generator checks that 60 + 60 matches two theory credits and that 120 matches one lab credit. It emits two theory sessions of two units each and one lab session of four units. Each carries the original teacher and batch IDs.

A `Problem` supplies the sessions, rooms, group sizes, and the teacher-by-slot availability matrix. Teacher and group IDs are zero-based indexes. Each teacher has a corresponding availability row of at least 80 entries. The search writes one `Placement` per session, using the same vector index. A placement records a weekly start slot and a room index. For example, Sunday at 14:00 is slot 10.

`Scheduler::search(i)` tries slots from earliest to latest and rooms in inventory order. `canPlace` rejects invalid candidates and compares the full time interval with earlier choices. A placement that leads to a dead end is reset to `{-1, -1}`. Reaching the end of the session list means every session has a placement. The separate validator checks the resulting list for valid indices, coverage, windows, availability, room constraints, and pairwise resource overlaps. It shares calendar and room-rule functions with the search.

## OOP choices

- **Encapsulation.** `Scheduler` keeps its tentative placements, counters, candidate checks, and recursion private. Its public methods expose generation and read-only access to results.
- **Abstraction.** A caller asks `SchedulingService` to generate a timetable and gets a `ScheduleResult` with success status, placements, and counters.
- **Composition.** The service constructs a scheduler for the given problem and calls the separate validator. The CLI assembles demo data, the service call, and output.
- **Single responsibility.** Rules, course expansion, search, validation, and console presentation each have a distinct reason to change. Plain records use structs and stateless operations use free functions.

A future GUI can construct the same `Problem`, call `generateRequiredSessions` if starting from course requirements, and pass the prepared problem to the service. It can then display the resulting session and placement pairs or offer CSV export. The service performs synchronous search, so a responsive GUI would run it off the UI thread.
