# Sprint 114 Plan: Test Failure Feedback Loop

## Context

Build success is only the first gate. Tests are the second gate, and they carry
richer semantic signal than compiler errors: a failing assertion tells you not
just *where* something went wrong but *what* the code was supposed to do versus
what it actually did. That gap between expected and actual is exactly the kind of
structured signal Whetstone can act on deterministically.

Sprint 114 adds test output parsing, failure classification, test-to-source
location mapping, and fix candidate generation for test failures. It mirrors
the architecture of Sprint 113 (Build Feedback Loop) but operates on test
output formats (Google Test, pytest, Jest, TAP). The result integrates with
IterationSession: each test run becomes an AttemptRecord with structured
observations and a GapModel keyed on failing test count.

---

## Goals

1. Parse test runner output (gtest, pytest, Jest, TAP) into `TestFailureRecord`s
2. Classify failures by type: assertion, exception, timeout, infra, wrong output
3. Map each failure to a source location and AST node
4. Generate ranked fix candidates from test failures
5. Integrate with IterationSession (test run → AttemptRecord)

---

## Steps

### Step 1369: `TestFailureRecord` schema (12 tests)

Create `editor/src/TestFailureRecord.h`.

Fields: `failure_id`, `runner` (enum: gtest, pytest, jest, tap, unknown),
`test_suite`, `test_name`, `failure_type` (enum: assertion, exception,
timeout, missing_output, wrong_output, infra, unknown),
`expected_value` (string, may be empty), `actual_value` (string, may be empty),
`file_path`, `line`, `message`, `ast_node_id` (populated by locator).

Tests (12): constructable, runner enum round-trips, failure_type defaults to
unknown, expected/actual stored, file_path stored, line stored, message stored,
ast_node_id starts empty, JSON serialization, JSON deserialization, unknown
runner passthrough, deterministic output.

### Step 1370: Test output parser — gtest format (10 tests)

Create `editor/src/TestOutputParser.h`.

Parses Google Test stdout format:
- `[ FAILED ]  SuiteName.TestName (Nms)` → failure header
- `Expected: <value>` / `  Actual: <value>` → assertion fields
- `<file>:<line>: Failure` → source location

Also parses pytest (`FAILED test_file.py::test_name - AssertionError: ...`)
and basic TAP (`not ok N - description`). Jest support for `✕ test name`
format. Unknown lines attached to current test as raw message.

Tests (10): gtest failure parsed, gtest assertion expected/actual extracted,
gtest file/line extracted, pytest failure parsed, jest failure parsed,
TAP not-ok parsed, pass lines ignored, multi-failure batch parsed,
empty input returns empty list, deterministic parse order.

### Step 1371: Test failure classifier (10 tests)

Create `editor/src/TestFailureClassifier.h`.

Classifies `TestFailureRecord` into semantic types using message patterns:
- `assertion` — Expect/Assert macro failure (expected != actual)
- `exception` — unhandled exception or signal in test
- `timeout` — test exceeded time limit
- `wrong_output` — stdout/output mismatch (snapshot tests)
- `missing_output` — expected output file not produced
- `infra` — test setup/teardown failure, missing fixture
- `unknown` — catch-all

Tests (10): assertion classified, unhandled exception classified, timeout
classified, snapshot mismatch → wrong_output, missing file → missing_output,
fixture failure → infra, unknown catch-all, deterministic classification,
classification applied to batch, JSON round-trip of classified record.

### Step 1372: Test-to-source locator (10 tests)

Create `editor/src/TestSourceLocator.h`.

Maps a `TestFailureRecord` to the implementation code under test, not just
the test file. Strategy:
1. If file_path is a test file (matches `*test*`, `*spec*`), find the
   corresponding implementation file by naming convention.
2. Locate the function-under-test by test_name substring match against
   AST function names.
3. Fall back to the test file's AST node if implementation not found.

Populates `ast_node_id` on the failure record.

Tests (10): test file maps to implementation file, function-under-test found
by name, fallback to test file node, no match returns nullopt gracefully,
gtest suite.test → function match, pytest test_name → function match,
deterministic selection, works across Python and C++, batch locating works,
ast_node_id written to record.

### Step 1373: Fix candidate generator for test failures (10 tests)

Create `editor/src/TestFixCandidateGenerator.h`.

Generates `FixCandidate` list from classified `TestFailureRecord`:

- `assertion` (expected != actual): suggest value correction or logic change
  at the implementation AST node. Confidence = 0.4–0.7 (semantic, heuristic).
- `exception`: suggest try/catch addition or precondition guard. Confidence 0.3.
- `timeout`: suggest async/await addition or algorithm complexity reduction hint.
  Confidence 0.2 (manual candidate prominent).
- `wrong_output` / `missing_output`: suggest regenerating snapshot or checking
  file write path. Manual candidate.
- `infra`: suggest fixture setup. Manual.
- `unknown`: manual only.

Tests (10): assertion produces value-correction candidate, confidence in range,
exception produces guard candidate, timeout produces manual prominent,
wrong_output produces snapshot candidate, batch processing works, candidates
ordered by confidence, manual fallback always present, deterministic output,
candidate mutations JSON round-trip.

### Step 1374: Test iteration session integration (8 tests)

Create `editor/src/TestIterationAdapter.h`.

Mirrors `BuildIterationAdapter`. Runs test command via subprocess, parses
output, computes GapModel (metric_type=test_failure_count), records attempt
in IterationSession, returns FixCandidates.

Tests (8): attempt appended to session, observation raw_text captured,
gap metric_type is test_failure_count, gap converged when all tests pass,
session converges on zero failures, fix candidates returned on failures,
attempt outcome=success on zero failures, attempt outcome=failure on any failure.

### Step 1375: `whetstone_parse_test_output` MCP tool (8 tests)

Input: `{ "output": string, "runner": string? }`
Output: `{ "failures": TestFailureRecord[], "pass_count": int, "fail_count": int }`

Tests (8): tool registered, fail_count correct, pass_count correct, runner
field respected, empty output returns empty failures, malformed input handled,
classification applied, deterministic output.

### Step 1376: `whetstone_suggest_test_fix` MCP tool (8 tests)

Input: `{ "session_id": string?, "test_output": string, "runner": string? }`
Output: `{ "candidates": FixCandidate[], "fail_count": int }`

Tests (8): tool registered, candidates returned, confidence present,
mutation field present, manual fallback always present, missing AST graceful,
candidates sorted by confidence, deterministic output.

### Step 1377: `whetstone_run_test_iteration` MCP tool (8 tests)

Input: `{ "session_id": string, "test_command": string, "workspace": string }`
Output: `{ "attempt_id": string, "session_status": string,
           "fail_count": int, "candidates": FixCandidate[] }`

Tests (8): tool registered, attempt_id returned, session_status updated,
fail_count correct, candidates on failure, session converged on all-pass,
timeout enforced (default 120s), deterministic output shape.

### Step 1378: Sprint 114 integration summary + regression (8 tests)

Create `editor/src/Sprint114IntegrationSummary.h`.

End-to-end: start session → run_test_iteration (failing tests) → verify
attempt recorded + candidates returned → apply top candidate → run again →
verify fail count reduced.

Tests (8): headers constructable, tools registered, end-to-end flow works,
gap model improves after fix, session transitions correctly, no header exceeds
600 lines, deterministic snapshots, full prior regression passes.

---

## Architecture Gate

- Assertion parsing is heuristic — expected/actual extraction may be empty
  for non-standard formats; that is acceptable
- Fix candidates are suggestions, not guaranteed correct fixes
- Manual fallback always generated so the iteration loop is never stuck
- Max 600 lines per header
