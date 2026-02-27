# Sprint 265 Plan: Raw Score/Gate Parity Packet and Enforcement

## Goal
Make raw-score vs coverage-gate agreement a first-class runtime invariant with explicit telemetry and optional hard-fail enforcement.

## Steps
- Step 2372: Add native raw score/gate parity packet in pipeline summary.
- Step 2373: Add enforcement control that fails on divergence.
- Step 2374: Validate parity enforcement on hard sample with projector path.
- Step 2375: Add `Sprint265IntegrationSummary.h` and execution tracker.
