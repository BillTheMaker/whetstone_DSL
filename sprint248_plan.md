# Sprint 248 Plan: Raw Generator Candidate Search (No Overlays)

## Goal
Probe whether raw generator outputs can be improved without shaping/autofill/multishot by searching multiple profile-guided requirement variants and selecting the best raw candidate.

## Steps
- Step 2285: Add raw task-set profile coverage scoring tool.
- Step 2286: Add optional raw candidate search path in pipeline.
- Step 2287: Generate and score multiple raw generation variants.
- Step 2288: Select best raw variant by failing-profile score and task count.
- Step 2289: Add `Sprint248IntegrationSummary.h` and execution tracker.
