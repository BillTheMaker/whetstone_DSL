# Sprint 259 Plan: Adaptive Raw Top-Gap Retry Loop

## Goal
Automatically run a second intrinsic raw-generation attempt with expanded top-gap requirements when weighted raw search shows no uplift.

## Steps
- Step 2342: Add adaptive retry controls and retry signal budget.
- Step 2343: Trigger retry when weighted top-gap score does not improve baseline.
- Step 2344: Score retry result and apply if it improves fail count/top-gap score/task depth.
- Step 2345: Emit retry telemetry and artifacts in summary.
- Step 2346: Add `Sprint259IntegrationSummary.h` and execution tracker.
