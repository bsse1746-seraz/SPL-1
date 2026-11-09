# Refactor and verification notes

## Baseline audit

The public `main` and `feature/lab-scheduling` branches both resolved to `8b929d8e52ccd8e0b56882885bd2c7bbc4634573`. Their Git diff was empty. The refactor branch starts from that commit, which corrected lab contact time to 120 minutes per credit.

The old `prototype/main.cpp` was 1,033 lines. It already demonstrated encapsulation through the `Scheduler` class and modeling through `Room`, `StudentGroup`, `CourseRequirement`, `CreditPolicy`, `Session`, `Placement`, and `Problem` structs. `Scheduler` owned recursive search and kept the search functions private, while its result and counters were public. The same file also contained the demo fixtures, window and interval rules, credit expansion, CSV parser and writer, validator, console menu, and 74 self-checks. Its external runtime input was the course CSV.

## Change inventory

- Added headers and implementations for models, scheduling rules, credit expansion, search, validation, the scheduling service, CSV I/O, demo data, and the CLI. The scheduler now exposes read-only search state. Demo resources populate the model explicitly.
- Moved the original checks into `tests/` and retained `--test`. The CLI preserves the menu, argument handling, presentation, and exit behavior.
- Added CMake and Windows builds and a GitHub Actions workflow. Replaced the launcher, retired the old source entry point, and archived the starter guide.
- Updated the README and learning index and added focused guides. The sample input, earlier learning notes, proposals, slides, and historical C source retain their original bytes.

## Verification

| Check                           | Observed result                                                                                                                                |
| ------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Original GNU C++17 build        | GCC 14.2 on Linux with `-Wall -Wextra -Wpedantic`. 74 of 74 self-checks passed.                                                                |
| Refactored GNU C++17 build      | CMake 3.16.8 and 4.4.4 configure, build, and CTest passed. The direct GCC build also passed with `-Werror`. The CLI printed `74 tests passed.` |
| Comparison against the original | 15 of 15 comparisons matched in exit status, stdout, stderr, and CSV bytes where present.                                                      |
| CSV sample                      | Nine sessions, zero hard-constraint violations, 101 candidate attempts and zero backtracks. The C++ lab uses Room-101 on Sunday 14:00â€“16:00.   |
| Impossible case                 | Search reports exhaustion with the original exit behavior. An existing CSV is preserved.                                                       |
| Service smoke check             | A custom fourth teacher scheduled through the core library. Blocking that teacher's week returned failure and an empty placement list.         |
| Windows verification | MSYS2 GCC 15.2.0 compiled successfully using the PowerShell build script after correcting compiler selection. All 74 self-tests passed. The CSV demo scheduled nine sessions with zero hard-constraint violations. |
| Platform coverage | Linux and Windows builds verified. Hosted GitHub Actions execution is pending verification after pushing the branch. |

The 15 comparisons covered self-tests, menu EOF, all interactive choices, the built-in demo, sample CSV, CRLF CSV, impossible mode with and without an existing export, unknown switch, missing CSV argument, missing CSV path, invalid header, invalid credit pattern, unschedulable file input, and export failure.

## Development stages

| Stage | Commit subject                                                  |
| ----- | --------------------------------------------------------------- |
| A     | `build: add CMake for the verified prototype`                   |
| B     | `refactor: extract timetable domain models`                     |
| C     | `refactor: separate scheduling rules search and validation`     |
| D     | `refactor: separate CSV console and scheduling service`         |
| E     | `test: extract self-tests and add reproducible build workflows` |
| F     | `docs: explain the modular scheduler and midterm workflow`      |

## Final source tree

```text
TimeTableX/
â”œâ”€â”€ .github/
â”‚   â””â”€â”€ workflows/
â”‚       â””â”€â”€ cpp.yml
â”œâ”€â”€ .gitignore
â”œâ”€â”€ CMakeLists.txt
â”œâ”€â”€ README.md
â”œâ”€â”€ SPL1 Project Proposal Form.docx
â”œâ”€â”€ TimeTableX_SPL1_Project_Proposal_Form_Filled.docx
â”œâ”€â”€ TimeTableX_SPL1_Project_Proposal_Updated_Slide4.pptx
â”œâ”€â”€ code.c
â”œâ”€â”€ data/
â”‚   â””â”€â”€ courses.csv
â”œâ”€â”€ docs/
â”‚   â”œâ”€â”€ ARCHITECTURE.md
â”‚   â”œâ”€â”€ GITHUB_INTEGRATION.md
â”‚   â”œâ”€â”€ MIDTERM_DEMO.md
â”‚   â”œâ”€â”€ REFACTOR_NOTES.md
â”‚   â”œâ”€â”€ ROADMAP.md
â”‚   â””â”€â”€ archive/
â”‚       â”œâ”€â”€ README.md
â”‚       â””â”€â”€ START_HERE.txt
â”œâ”€â”€ include/
â”‚   â””â”€â”€ timetablex/
â”‚       â”œâ”€â”€ app/
â”‚       â”‚   â””â”€â”€ SchedulingService.hpp
â”‚       â”œâ”€â”€ core/
â”‚       â”‚   â”œâ”€â”€ Scheduler.hpp
â”‚       â”‚   â”œâ”€â”€ SchedulingRules.hpp
â”‚       â”‚   â”œâ”€â”€ SessionGenerator.hpp
â”‚       â”‚   â””â”€â”€ Validator.hpp
â”‚       â”œâ”€â”€ data/
â”‚       â”‚   â””â”€â”€ DemoData.hpp
â”‚       â”œâ”€â”€ io/
â”‚       â”‚   â””â”€â”€ TimetableCsv.hpp
â”‚       â””â”€â”€ models/
â”‚           â””â”€â”€ Problem.hpp
â”œâ”€â”€ learnings/
â”‚   â”œâ”€â”€ 02-credit-based-session-generation.md
â”‚   â”œâ”€â”€ 03-csv-course-input.md
â”‚   â”œâ”€â”€ 04-room-management.md
â”‚   â”œâ”€â”€ 05-modular-architecture.md
â”‚   â””â”€â”€ README.md
â”œâ”€â”€ scripts/
â”‚   â”œâ”€â”€ build.ps1
â”‚   â””â”€â”€ run_windows.bat
â”œâ”€â”€ src/
â”‚   â”œâ”€â”€ app/
â”‚   â”‚   â””â”€â”€ SchedulingService.cpp
â”‚   â”œâ”€â”€ cli/
â”‚   â”‚   â”œâ”€â”€ Console.cpp
â”‚   â”‚   â””â”€â”€ Console.hpp
â”‚   â”œâ”€â”€ core/
â”‚   â”‚   â”œâ”€â”€ Scheduler.cpp
â”‚   â”‚   â”œâ”€â”€ SchedulingRules.cpp
â”‚   â”‚   â”œâ”€â”€ SessionGenerator.cpp
â”‚   â”‚   â””â”€â”€ Validator.cpp
â”‚   â”œâ”€â”€ data/
â”‚   â”‚   â””â”€â”€ DemoData.cpp
â”‚   â”œâ”€â”€ io/
â”‚   â”‚   â””â”€â”€ TimetableCsv.cpp
â”‚   â””â”€â”€ main.cpp
â””â”€â”€ tests/
    â”œâ”€â”€ SelfTests.cpp
    â””â”€â”€ SelfTests.hpp
```
