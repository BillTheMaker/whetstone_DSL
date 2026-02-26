# Sprint 246 Plan: Intrinsic Multishot Generator Decomposition

## Goal
Improve first-pass native decomposition without profile-autofill stubs by using generator-driven multishot decomposition passes per active impact profile.

## Steps
- Step 2274: Add optional multishot decomposition controls in pipeline.
- Step 2275: Run `whetstone_generate_taskitems` per active intrinsic profile requirement.
- Step 2276: Merge successful multishot tasks into effective task set with deterministic profile overlays.
- Step 2277: Surface multishot telemetry and effective task count in summary.
- Step 2278: Verify enforced impact coverage pass/fail with multishot on/off.
- Step 2279: Add `Sprint246IntegrationSummary.h` and execution tracker.
