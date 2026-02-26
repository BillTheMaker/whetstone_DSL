# Sprint 240 Plan: Native Decomposition Retry Path

## Goal
Attempt a deterministic second-pass native decomposition before fallback expansion when initial native task depth is below target.

## Steps
- Step 2243: Add configurable native decomposition retry toggle and target minimum task count.
- Step 2244: Retry `whetstone_generate_taskitems` with explicit decomposition constraint.
- Step 2245: Apply retry result only when it improves native task count.
- Step 2246: Emit retry telemetry in pipeline summary.
- Step 2247: Add `Sprint240IntegrationSummary.h` and execution tracker.
