# Modular architecture

A header tells another file what it can call or use. A `.cpp` file usually contains the implementation. Start with `include/timetablex/models/Problem.hpp` to see what the program stores, then open `src/core/Scheduler.cpp` to see how it changes tentative placements while searching.

Follow one run through these files.

1. `src/main.cpp` hands the command-line arguments to the console.
2. `src/data/DemoData.cpp` assembles the resources. The file demo uses `src/io/TimetableCsv.cpp` to parse courses.
3. `src/core/SessionGenerator.cpp` converts approved meeting lengths to 30-minute units.
4. `src/app/SchedulingService.cpp` runs the scheduler, then passes the completed assignments to `src/core/Validator.cpp`.
5. `src/cli/Console.cpp` shows the result. The CSV module writes the export.

For a practice trace, use the **dead end undone by backtracking** check in `tests/SelfTests.cpp`. The Flexible class can fit at 09:00 or 10:00. The Restricted class only fits at 09:00. Follow `search` as it first tries 09:00 for Flexible, sees the dead end, clears that placement, and moves Flexible to 10:00.

See the [architecture guide](../docs/ARCHITECTURE.md) for a module map and the [midterm guide](../docs/MIDTERM_DEMO.md) for runnable commands and viva questions.
