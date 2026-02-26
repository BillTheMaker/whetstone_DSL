# Sprint 233 Plan: Semantic-Aware Task Decomposition Bridge

## Goal
Ensure semantic planning packets materially affect task decomposition instead of only being logged.

## Steps
- Step 2205: Add semantic packet -> intake augmentation pass to improve fallback intake quality.
- Step 2206: Add semantic requirement injection into normalized requirement set before taskitem generation.
- Step 2207: Add semantic task expansion bridge to derive additional constrained tasks from semantic requirements.
- Step 2208: Emit semantic bridge telemetry in pipeline summary (`semantic_requirement_injection`, `semantic_task_expansion`).
- Step 2209: Run A/B pipeline comparison (semantic bridge off vs on) and record deltas.
- Step 2210: Add `Sprint233IntegrationSummary.h`.
