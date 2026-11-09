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
| CSV sample                      | Nine sessions, zero hard-constraint violations, 101 candidate attempts and zero backtracks. The C++ lab uses Room-101 on Sunday 14:00–16:00.   |
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
├── .github/
│   └── workflows/
│       └── cpp.yml
├── .gitignore
├── CMakeLists.txt
├── README.md
├── SPL1 Project Proposal Form.docx
├── TimeTableX_SPL1_Project_Proposal_Form_Filled.docx
├── TimeTableX_SPL1_Project_Proposal_Updated_Slide4.pptx
├── code.c
├── data/
│   └── courses.csv
├── docs/
│   ├── ARCHITECTURE.md
│   ├── GITHUB_INTEGRATION.md
│   ├── MIDTERM_DEMO.md
│   ├── REFACTOR_NOTES.md
│   ├── ROADMAP.md
│   └── archive/
│       ├── README.md
│       └── START_HERE.txt
├── include/
│   └── timetablex/
│       ├── app/
│       │   └── SchedulingService.hpp
│       ├── core/
│       │   ├── Scheduler.hpp
│       │   ├── SchedulingRules.hpp
│       │   ├── SessionGenerator.hpp
│       │   └── Validator.hpp
│       ├── data/
│       │   └── DemoData.hpp
│       ├── io/
│       │   └── TimetableCsv.hpp
│       └── models/
│           └── Problem.hpp
├── learnings/
│   ├── 02-credit-based-session-generation.md
│   ├── 03-csv-course-input.md
│   ├── 04-room-management.md
│   ├── 05-modular-architecture.md
│   └── README.md
├── scripts/
│   ├── build.ps1
│   └── run_windows.bat
├── src/
│   ├── app/
│   │   └── SchedulingService.cpp
│   ├── cli/
│   │   ├── Console.cpp
│   │   └── Console.hpp
│   ├── core/
│   │   ├── Scheduler.cpp
│   │   ├── SchedulingRules.cpp
│   │   ├── SessionGenerator.cpp
│   │   └── Validator.cpp
│   ├── data/
│   │   └── DemoData.cpp
│   ├── io/
│   │   └── TimetableCsv.cpp
│   └── main.cpp
└── tests/
    ├── SelfTests.cpp
    └── SelfTests.hpp
```
