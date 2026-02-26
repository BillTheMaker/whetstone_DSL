# Sprint 261 Plan: Intrinsic Prompt-Pack Intervention

## Goal
Add a stronger model-facing intrinsic prompt pack derived from top-gap signals (schema-style required fields + anti-pattern bans) and inject it before raw generation.

## Steps
- Step 2353: Add prompt-pack synthesis tool from top-gap backlog signals.
- Step 2354: Inject prompt-pack normalized requirements in pipeline pre-generation stage.
- Step 2355: Emit prompt-pack telemetry in summary packet.
- Step 2356: Validate OFF/ON behavior on hard sample.
- Step 2357: Add `Sprint261IntegrationSummary.h` and execution tracker.
