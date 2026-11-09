# TimeTableX roadmap

| Milestone | Scope | Evidence to collect |
| --- | --- | --- |
| M1–M3, implemented | Calendar and clashes, credit-based sessions and CSV, capacity and lab suitability. | Existing 74 checks and the nine-session sample demo. |
| Modular refactor, implemented | Separate core, application service, CSV, CLI, and tests. | Baseline output parity and reproducible builds. |
| M4, planned | Accept teacher availability and preferences. Separate mandatory blocked times from preferred days and times. Define weighted penalties. | Fixtures that distinguish hard failures from softer scheduling costs. |
| M5, planned | Evaluate MRV, forward checking, and preference optimization as separate algorithm changes. | Compare validity, attempts, run time, and preference cost against the current first-feasible solver. |
| GUI and larger datasets, planned | Add validated input forms, a weekly grid, and batch or room views. Call the service and reuse export. | Demonstrate a GUI-generated schedule with the same validation as the CLI. |

Keep teacher–course assignments fixed. Add any new hard constraint to both candidate checking and final validation, with a case that deliberately violates it.
