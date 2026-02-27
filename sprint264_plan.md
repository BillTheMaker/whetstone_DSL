# Sprint 264 Plan: Raw Score/Gate Parity Alignment

## Goal
Eliminate scorer-vs-gate semantic drift by ensuring downstream coverage checks evaluate the same selected raw candidate task set used by raw scoring.

## Steps
- Step 2368: Persist selected raw candidate tasks back into generation artifacts.
- Step 2369: Re-run hard sample with structural projector to verify score/gate parity.
- Step 2370: Confirm `native_raw_candidate_search` and `native_impact_coverage` agree.
- Step 2371: Add `Sprint264IntegrationSummary.h` and execution tracker.
