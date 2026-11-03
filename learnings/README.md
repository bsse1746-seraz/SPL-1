# TimeTableX — Learning Notes

This directory records the design decisions, implementation logic, tests, and viva explanations for the TimeTableX SPL-1 project.

## Reading order

1. [M2 Step 1 — Credit-Based Session Generation](02-credit-based-session-generation.md)

## Workflow

For each feature: define expected behavior, implement in C++17, compile, run automated tests and the demonstration, review source changes, then commit and push. Test results are recorded as checkpoints, not assumed to remain true after future edits.

## Project scope reminder

The department fixes teacher-course assignments. Theory runs within 09:00–13:00; 13:00–14:00 is a mandatory break; the current lab rule requires 14:00–17:00 sessions. The credit policy currently uses *illustrative, configurable values*, not a verified IIT-DU regulation.
