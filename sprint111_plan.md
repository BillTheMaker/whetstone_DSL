# Sprint 111 Plan: Iteration Session Context

## Context

Sprints 1-110 established Whetstone as a deterministic *open-loop* generator:
schema → struct, spec → taskitems, source → transpiled output. The output is
fully determined by the input, with no environmental state involved.

Sprint 111 begins a new capability tier: **closed-loop iteration**. Real
development is not open-loop. It involves running code, observing what breaks,
forming a hypothesis, patching, and repeating until a goal is met. This sprint
introduces the foundational primitive for that loop: the `IterationSession`.

An `IterationSession` is a structured record of one attempt to achieve a
convergence goal (all tests pass, build succeeds, environment is ready). It
tracks: the goal, each attempt taken, each observation made, and the current
distance from the goal. Every subsequent sprint in this epoch builds on it.

---

## Goals

1. Define the `IterationSession` schema with goal, attempt history, and state
2. Define `AttemptRecord` and `ObservationRecord` as structured sub-types
3. Define `GapModel` to measure distance from the convergence goal
4. Implement a state machine: pending → active → converged / escalated / failed
5. Persist sessions to sidecar files alongside annotated ASTs

---

## Steps

### Step 1339: `IterationSession` core schema (12 tests)

Create `editor/src/IterationSession.h`.

Fields: `session_id`, `goal` (string), `goal_type` (enum: build_success,
tests_pass, environment_ready, custom), `status` (enum: pending, active,
converged, escalated, failed), `created_at`, `updated_at`, `attempts[]`,
`max_attempts` (default 10).

Tests (12): constructable with defaults, goal stored, goal_type round-trips,
status defaults to pending, session_id is non-empty on construction,
attempt list starts empty, max_attempts enforced, JSON serialization,
JSON deserialization, unknown status survives passthrough,
deterministic serialization order, schema version field present.

### Step 1340: `AttemptRecord` schema (10 tests)

Create `editor/src/AttemptRecord.h`.

Fields: `attempt_id`, `session_id`, `action_type` (enum: build, test, patch,
probe, escalate), `action_description`, `started_at`, `completed_at`,
`outcome` (enum: success, failure, partial, error), `observation_id`.

Tests (10): constructable, action_type round-trips, outcome defaults to error,
duration computable from timestamps, JSON round-trip, unknown action_type
passthrough, attempt_id non-empty, links to session_id, links to observation_id,
deterministic serialization.

### Step 1341: `ObservationRecord` schema (10 tests)

Create `editor/src/ObservationRecord.h`.

Fields: `observation_id`, `source` (enum: build_output, test_output,
process_stdout, process_stderr, file_read, env_probe), `raw_text`,
`exit_code`, `duration_ms`, `structured_data` (JSON blob for parsed output),
`truncated` (bool — raw_text may be capped at 16KB).

Tests (10): constructable, source enum round-trips, exit_code defaults to -1,
truncated flag set when raw_text exceeds limit, structured_data round-trips,
JSON serialization, deserialization, unknown source passthrough,
observation_id non-empty, deterministic output.

### Step 1342: `GapModel` — distance from convergence goal (10 tests)

Create `editor/src/GapModel.h`.

Fields: `metric_type` (enum: error_count, test_failure_count, missing_deps,
custom), `current_value`, `target_value`, `delta`, `converged` (bool —
true when current_value == target_value), `trend` (enum: improving,
stagnant, worsening — derived from last two values).

Tests (10): converged true when delta is zero, trend improving when delta
decreasing, trend worsening when delta increasing, stagnant after two equal
values, JSON round-trip, negative delta flagged as worsening, custom metric
type passthrough, deterministic output, delta auto-computed, schema constructable.

### Step 1343: Session state machine transitions (10 tests)

Add `SessionStateMachine` to `IterationSession.h` (or small companion header).

Valid transitions:
- pending → active (on first attempt recorded)
- active → converged (on gap.converged == true)
- active → escalated (on max_attempts reached without convergence)
- active → failed (on attempt with outcome=error and no recovery path)
- Any terminal state is immutable.

Tests (10): pending→active on first attempt, active→converged when gap closes,
active→escalated at max_attempts, active→failed on terminal error,
illegal transition rejected, terminal states immutable, state serialized,
state deserialized, history preserved through transitions, deterministic
transition log.

### Step 1344: Session persistence to sidecar file (10 tests)

Add `SessionStore` to a new `editor/src/SessionStore.h`.

Sidecar path convention: `.whetstone/sessions/<session_id>.json` relative
to workspace root. `SessionStore::save(session)` writes atomically (write
to `.tmp` then rename). `SessionStore::load(session_id)` reads and
deserializes. `SessionStore::list()` returns all session IDs in the store.

Tests (10): save creates file, load round-trips session, list returns saved ids,
overwrite preserves new state, missing session returns nullopt, atomic write
(partial write does not corrupt), path derivation from workspace root,
concurrent save does not corrupt (file lock), deterministic file layout,
sidecar dir created on first save.

### Step 1345: `whetstone_start_iteration_session` MCP tool (8 tests)

Create `editor/src/mcp/RegisterIterationSessionTools.h`.

Input: `{ "goal": string, "goal_type": string, "max_attempts": int? }`
Output: `{ "session_id": string, "status": "pending" }`

Persists session to SessionStore immediately on creation.

Tests (8): tool registered, schema validates, session_id returned,
status is pending, session persisted to disk, missing goal_type defaults
to "custom", max_attempts defaults to 10, deterministic output shape.

### Step 1346: `whetstone_record_attempt` MCP tool (8 tests)

Input: `{ "session_id": string, "action_type": string,
          "action_description": string, "observation": { ... } }`
Output: `{ "attempt_id": string, "session_status": string, "gap": { ... }? }`

Appends AttemptRecord + ObservationRecord to session. Returns updated
session status after state machine step.

Tests (8): tool registered, attempt appended, session_status updated,
unknown session_id returns error, illegal transition returns error,
gap field present when gap model updated, deterministic output,
session persisted after record.

### Step 1347: `whetstone_get_session_state` MCP tool (8 tests)

Input: `{ "session_id": string }`
Output: full session JSON with attempts[], gap, and current status.

Tests (8): tool registered, full session returned, attempts included,
gap included when present, terminal sessions readable, missing session
returns structured error, deterministic serialization, attempt count correct.

### Step 1348: Sprint 111 integration summary + regression (8 tests)

Create `editor/src/Sprint111IntegrationSummary.h`.

Tests (8): all new headers constructable, tools registered in initialize
response, session round-trip end-to-end (start → record × 2 → converge →
read), state machine covers all transitions, sidecar created on disk,
no header exceeds 600 lines, deterministic snapshots, full prior regression
passes.

---

## Architecture Gate

- Max 600 lines per header — split `IterationSessionExtended.h` if needed
- No runtime dependencies beyond nlohmann-json and std::filesystem
- Session IDs use UUIDv4 (reuse existing UUID utility if present, else add one)
- All MCP tool outputs are stable and machine-readable
- Sidecar format is versioned (`"schema_version": 1`)
