# Sprint 116 Plan: Feedback Loop Orchestrator

## Context

Sprints 111-115 built the primitives: iteration sessions, environment snapshots,
build feedback, test feedback, and requirements derivation. Each works
independently. Sprint 116 wires them into a single orchestrated loop.

The `FeedbackLoopOrchestrator` is a state machine that drives an
`IterationSession` from start to convergence (or escalation). It decides the
next action based on the current session state: if the environment is not
verified, run `verify_requirements` first; if the build is failing, run
`run_build_iteration`; if tests are failing, run `run_test_iteration`. After
each action it selects the highest-confidence fix candidate, applies it, and
loops. When max_attempts is reached without convergence, it escalates to human
with a structured summary of what was tried and what the current gap is.

This is the "inner loop" of deterministic development: goal → sense → act →
observe → repeat.

---

## Goals

1. Define the orchestrator's decision policy (what to do given current state)
2. Implement automatic candidate selection and application
3. Handle escalation with a structured "what was tried" summary
4. Expose the orchestrator as a single MCP entry point
5. Ensure the loop is inspectable at every step (full audit trail)

---

## Steps

### Step 1389: `OrchestratorPolicy` decision model (12 tests)

Create `editor/src/OrchestratorPolicy.h`.

The policy maps (session_status, gap_model, last_attempt_outcome) →
next action type:
- If environment not verified: probe environment
- If requirements gap present: generate setup script → escalate (can't
  auto-install; human must run the script)
- If build failing: run build iteration, apply top candidate
- If tests failing: run test iteration, apply top candidate
- If gap = 0: mark converged
- If max_attempts reached: escalate
- If last attempt made gap worse (worsening trend, 2+ consecutive): escalate

Each decision includes: `action_type`, `rationale` string, `confidence` float.

Tests (12): environment probe first when no snapshot, requirements gap → escalate,
build failure → build iteration, test failure → test iteration, zero gap →
converge, max_attempts → escalate, worsening trend → escalate, consecutive
no-progress → escalate, decision includes rationale, confidence in range,
JSON round-trip of decision, deterministic decision for same inputs.

### Step 1390: Candidate auto-selector (10 tests)

Create `editor/src/CandidateAutoSelector.h`.

Given a list of `FixCandidate`s and a policy, selects the candidate to apply.
Rules:
- Select highest confidence candidate
- Never re-apply a candidate that was already attempted in this session
  (tracked by candidate description hash)
- If all non-manual candidates were already tried: escalate
- If top candidate is manual (mutation_type=manual): escalate immediately
  (cannot apply manually)

Tests (10): highest confidence selected, already-tried skipped, all tried →
escalate decision, manual type → escalate, empty list → escalate, confidence
tie broken deterministically, attempt history consulted, selection logged
to session, JSON round-trip of selection record, deterministic output.

### Step 1391: Escalation report composer (10 tests)

Create `editor/src/EscalationReport.h`.

Produces a human-readable + machine-readable summary when the loop escalates:
- `session_id`, `goal`, `goal_type`
- `attempts_made` (count)
- `final_gap` (GapModel)
- `attempts_summary[]`: each attempt's action, outcome, gap_before, gap_after
- `top_remaining_candidates[]`: the best manual candidates not yet tried
- `escalation_reason` (enum: max_attempts, worsening_trend, all_auto_tried,
  manual_required, infra_gap)
- `recommended_next_step` (human-readable string)

Tests (10): report constructable, session_id present, attempts_summary accurate,
top candidates included, escalation_reason populated, recommended_next_step
human-readable, JSON round-trip, empty attempts handled, gap accurate,
deterministic output.

### Step 1392: `FeedbackLoopOrchestrator` core loop (10 tests)

Create `editor/src/FeedbackLoopOrchestrator.h`.

The orchestrator drives one step of the loop at a time (not a blocking loop —
caller steps it). Interface:
- `step(session_id)` → `OrchestratorStepResult` (action taken, new session
  status, gap after, escalation_report if escalated)

Internally:
1. Load session from SessionStore
2. Run OrchestratorPolicy to get next action decision
3. Execute the action (build iteration / test iteration / env probe)
4. Run CandidateAutoSelector on result
5. Apply selected mutation to AST (if non-manual)
6. Record attempt, update session, persist
7. Return result

Tests (10): step runs without crashing, action matches policy, mutation applied
to AST, attempt recorded in session, session converges when gap=0, session
escalates at max_attempts, worsening trend triggers escalation, escalation
report present when escalated, step result includes gap, deterministic output.

### Step 1393: Full loop driver (end-to-end stepping) (10 tests)

Add `FeedbackLoopOrchestrator::run_to_completion(session_id, max_wall_s)`.

Calls `step()` in a loop until session reaches a terminal state or wall time
expires. Each step result appended to an audit log. Returns the final session
state + full audit log.

Tests (10): run_to_completion returns on convergence, returns on escalation,
respects wall time limit, audit log has one entry per step, step count ≤
max_attempts, gap trend visible in audit log, escalation report in last entry
if escalated, deterministic for deterministic inputs, session persisted at end,
no infinite loop possible.

### Step 1394: Audit trail and inspection (8 tests)

Create `editor/src/LoopAuditLog.h`.

A serializable list of `AuditEntry` records: one per orchestrator step.
Fields: `step_index`, `action_type`, `action_description`, `candidate_applied`
(optional), `gap_before`, `gap_after`, `session_status_after`, `duration_ms`.

Persist to `.whetstone/audits/<session_id>.jsonl` (newline-delimited JSON,
one entry per line, appendable).

Tests (8): audit entry constructable, appended per step, persisted to JSONL,
JSONL format valid, gap_before/after present, candidate_applied present when
applied, duration_ms populated, deterministic output.

### Step 1395: `whetstone_start_feedback_loop` MCP tool (8 tests)

Input: `{ "goal": string, "goal_type": string, "build_command": string?,
          "test_command": string?, "workspace": string, "max_attempts": int? }`
Output: `{ "session_id": string, "status": "pending" }`

Creates the session and registers build/test commands for the orchestrator.
Does not start the loop — caller steps it.

Tests (8): tool registered, session_id returned, status is pending, session
persisted, build_command stored, test_command stored, missing workspace error,
deterministic output.

### Step 1396: `whetstone_step_feedback_loop` MCP tool (8 tests)

Input: `{ "session_id": string }`
Output: `{ "action_taken": string, "session_status": string, "gap": GapModel,
           "candidates": FixCandidate[]?, "escalation": EscalationReport? }`

Calls orchestrator.step() once and returns the result.

Tests (8): tool registered, action_taken populated, session_status updated,
gap present, candidates present on failure, escalation present when escalated,
terminal session returns error, deterministic output.

### Step 1397: `whetstone_run_feedback_loop` MCP tool (8 tests)

Input: `{ "session_id": string, "max_wall_seconds": int? }`
Output: `{ "final_status": string, "steps_taken": int, "final_gap": GapModel,
           "audit_log": AuditEntry[]?, "escalation": EscalationReport? }`

Calls run_to_completion and returns the full result.

Tests (8): tool registered, final_status correct, steps_taken accurate,
final_gap present, audit_log included when requested, escalation present when
escalated, wall time respected, deterministic output shape.

### Step 1398: Sprint 116 integration summary + regression (8 tests)

Create `editor/src/Sprint116IntegrationSummary.h`.

End-to-end: start_feedback_loop → run_feedback_loop on a project with a
known fixable error → verify converged within max_attempts.

Tests (8): headers constructable, tools registered, end-to-end convergence
demonstrated, audit log produced, no header exceeds 600 lines,
deterministic snapshots, escalation path tested, full prior regression passes.

---

## Architecture Gate

- Orchestrator steps are non-blocking (one step per call) — no background
  threads, no blocking I/O in the orchestrator itself
- The loop is always inspectable: every step is logged to the audit trail
  before returning
- Manual candidates never auto-applied — escalation is the only path for
  mutation_type=manual
- Max 600 lines per header
