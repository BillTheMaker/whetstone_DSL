# Sprint 226 Plan: AB Runner Integration for Repairs

## Goal
Integrate deterministic repair pass into AB Path-B so parity checks evaluate repaired deterministic output.

## Steps
- Step 2169: Call repair tool after `whetstone_run_pipeline` output extraction.
- Step 2170: Preserve repaired output in path-B artifact file.
- Step 2171: Keep gate evaluation and token accounting stable.
- Step 2172: Run focused hard subset with parity gate enabled.
- Step 2173: Verify blocked-parity load reduction.
