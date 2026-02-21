# Sprint 117 Plan: HiveMind Integration — Feedback Loop as Distributed Jobs

## Context

Sprints 111-116 implement the feedback loop as a local, in-process operation:
the orchestrator runs build/test commands on the same machine as Whetstone.
That works for single-node development but misses HiveMind's core value: the
right work runs on the right hardware node.

Sprint 117 integrates the feedback loop with HiveMind's job dispatch model.
Instead of running `cmake --build` locally, the orchestrator can emit a
`build_job` to HiveMind and wait for the result. Instead of running `pytest`
locally, it emits a `test_job`. The orchestrator becomes a HiveMind
client — it submits iteration steps as jobs, polls for completion, and feeds
the results back into the session as `ObservationRecord`s.

This is the natural boundary between the two systems: Whetstone owns the
*reasoning* (what to do next, what does the result mean, what to try next);
HiveMind owns the *execution* (run this on the GPU node, run that on the
NAS, parallelize across the fleet).

---

## Goals

1. Define a HiveMind job adapter for build, test, and probe iteration steps
2. Submit iteration steps as HiveMind jobs via the nexus HTTP API
3. Poll for job completion and map the result to ObservationRecord
4. Integrate the distributed path with the FeedbackLoopOrchestrator policy
5. Allow the orchestrator to run locally or distributed (config per session)

---

## Steps

### Step 1399: `HiveMindJobAdapter` schema and config (12 tests)

Create `editor/src/HiveMindJobAdapter.h`.

Config fields: `nexus_url` (e.g. http://localhost:8765), `capabilities[]`
(required capabilities for the job — forwarded to HiveMind requirements),
`bounty` (low/normal/high_priority), `poll_interval_s` (default 3),
`timeout_s` (default 300).

Maps orchestrator action types to HiveMind job types:
- `build` → `"build_job"` with payload `{ "command": string, "workspace": string }`
- `test`  → `"test_job"` with payload `{ "command": string, "workspace": string }`
- `probe` → `"probe_job"` with payload `{ "workspace": string }`

Tests (12): config constructable, nexus_url stored, capabilities stored,
bounty defaults to normal, poll_interval defaults to 3, timeout defaults
to 300, build action maps to build_job, test action maps to test_job,
probe action maps to probe_job, payload fields populated, JSON round-trip,
deterministic output.

### Step 1400: Job submission via nexus HTTP API (10 tests)

Create `editor/src/NexusJobSubmitter.h`.

Submits a HiveMind job via `POST {nexus_url}/jobs`. Uses libcurl (available
from Sprint 113 build subprocess pattern, or via HTTP client in stdlibs).
Returns `job_id` string on success. Handles: 200 OK (parse id), 4xx (error),
timeout (fail fast), network error (fail fast).

Tests (10): submit produces job_id, invalid nexus_url returns error, 4xx
response returns structured error, timeout returns error, payload correctly
serialized, capabilities forwarded, bounty forwarded, job_type forwarded,
non-blocking submit (returns immediately after HTTP response),
deterministic request format.

### Step 1401: Job result poller (10 tests)

Create `editor/src/NexusJobPoller.h`.

Polls `GET {nexus_url}/jobs/{job_id}` until status reaches `done` or `failed`.
Returns `JobPollResult`: `job_id`, `status` (done/failed/pending/claimed),
`output` (parsed from result payload — `output_file` content or inline),
`exit_code`, `duration_ms`.

Respects poll_interval_s and timeout_s from HiveMindJobAdapter config.

Tests (10): poll returns done on success, poll returns failed on failure,
poll respects timeout, poll respects interval, output field populated,
exit_code populated, pending status keeps polling, claimed status keeps
polling, network error returns error, deterministic result shape.

### Step 1402: Result-to-ObservationRecord mapper (10 tests)

Create `editor/src/JobResultMapper.h`.

Maps a `JobPollResult` to an `ObservationRecord`:
- `source` = build_output / test_output / env_probe (from action type)
- `raw_text` = output field (truncated at 16KB)
- `exit_code` = from JobPollResult
- `duration_ms` = from JobPollResult
- `structured_data` = parsed by BuildOutputParser or TestOutputParser
  depending on action type

Tests (10): build result maps to ObservationRecord with source=build_output,
test result maps with source=test_output, probe result maps with
source=env_probe, raw_text truncated at limit, exit_code mapped, duration_ms
mapped, structured_data populated by parser, failed job maps to exit_code!=0,
JSON round-trip of mapped record, deterministic output.

### Step 1403: Distributed path in FeedbackLoopOrchestrator (10 tests)

Extend `FeedbackLoopOrchestrator.h` to support a `distributed` mode
(set per session via config).

When distributed=true:
- `step()` submits a HiveMind job instead of running subprocess locally
- Returns immediately with `session_status=active, action=submitted`
  (the loop is async — caller must step again to poll)
- On next step: if a pending job exists for the session, poll it;
  if done, map result and continue; if timeout, escalate

When distributed=false (default): existing local subprocess behavior unchanged.

Tests (10): distributed mode submits job not subprocess, immediate return
on submit, subsequent step polls existing job, done result processed correctly,
failed result processed, timeout escalates, local mode still works, config
persisted in session, mode switch between sessions works, deterministic policy.

### Step 1404: `whetstone_configure_hivemind` MCP tool (8 tests)

Input: `{ "nexus_url": string, "capabilities": string[]?,
          "bounty": string?, "timeout_s": int? }`
Output: `{ "configured": true, "nexus_url": string }`

Persists HiveMind config to `.whetstone/hivemind_config.json` in workspace.
All sessions created after this call use distributed mode by default.

Tests (8): tool registered, config persisted, nexus_url stored, capabilities
stored, bounty defaulted to normal, timeout defaulted to 300, overwrite works,
deterministic output.

### Step 1405: `whetstone_submit_iteration_job` MCP tool (8 tests)

Input: `{ "session_id": string, "action_type": string,
          "command": string, "workspace": string }`
Output: `{ "job_id": string, "attempt_id": string, "status": "submitted" }`

Submits one iteration step as a HiveMind job and records the pending attempt.

Tests (8): tool registered, job_id returned, attempt_id returned, status
is submitted, attempt recorded in session, nexus_url from config used,
missing config falls back to local, deterministic output.

### Step 1406: `whetstone_poll_iteration_job` MCP tool (8 tests)

Input: `{ "session_id": string, "attempt_id": string }`
Output: `{ "job_status": string, "session_status": string,
           "gap": GapModel?, "candidates": FixCandidate[]? }`

Polls the HiveMind job associated with the attempt, processes result if done.

Tests (8): tool registered, pending returns job_status=pending, done processes
result, failed processes failure, session_status updated on done, gap present
after processing, candidates present on failure, deterministic output.

### Step 1407: Sprint 117 integration summary + regression (8 tests)

Create `editor/src/Sprint117IntegrationSummary.h`.

End-to-end distributed test (against mock nexus): start session with
distributed=true → submit_iteration_job → poll_iteration_job (mock returns
done) → verify attempt recorded + candidates returned.

Tests (8): headers constructable, tools registered, distributed end-to-end
mock test passes, local mode still works, config round-trip, no header exceeds
600 lines, deterministic snapshots, full prior regression passes.

---

## Architecture Gate

- HiveMind nexus URL is never hardcoded — always from config or MCP input
- Distributed mode does not block the MCP server — jobs are async by design
- Local mode (distributed=false) remains the default until explicitly configured
- No HiveMind-specific types leak into the core orchestrator headers —
  the adapter pattern keeps the boundary clean
- Max 600 lines per header
