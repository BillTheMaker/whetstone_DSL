# Sprint 113 Plan: Build Output Feedback Loop

## Context

Build failures are the most common reason a development iteration stalls.
They are also highly structured: compilers emit errors in machine-parseable
formats (file, line, col, error code, message). That structure is currently
discarded — the developer reads the raw output and decides what to fix.

Sprint 113 makes build output a first-class Whetstone input. A build failure
becomes a structured `BuildErrorRecord`, which is located in the AST to
identify the responsible node, classified by error type, and used to generate
a ranked list of fix candidates. Each candidate is a concrete AST mutation
with a confidence score. The result integrates with the IterationSession from
Sprint 111 — each build attempt becomes a recorded attempt with a
structured observation.

---

## Goals

1. Parse compiler output (GCC, Clang, CMake) into structured `BuildErrorRecord`s
2. Locate each error in the AST by file + line + col
3. Classify error types (missing dep, type error, undefined symbol, etc.)
4. Generate ranked fix candidates as AST mutations
5. Integrate with IterationSession (build attempt → AttemptRecord)

---

## Steps

### Step 1359: `BuildErrorRecord` schema (12 tests)

Create `editor/src/BuildErrorRecord.h`.

Fields: `error_id`, `tool` (enum: gcc, clang, clang_tidy, cmake, linker,
unknown), `severity` (enum: error, warning, note), `file_path`, `line`,
`col`, `error_code` (e.g. "E0137", "-Wunused"), `message`, `context_lines[]`
(surrounding source lines captured from file), `ast_node_id` (populated by
locator, initially empty).

Tests (12): constructable with defaults, tool enum round-trips, severity
defaults to error, file_path stored, line/col stored, error_code stored,
message stored, context_lines starts empty, ast_node_id starts empty,
JSON serialization, JSON deserialization, deterministic output.

### Step 1360: Build output parser — GCC/Clang format (10 tests)

Create `editor/src/BuildOutputParser.h`.

Parses the stdout/stderr of a build command into `BuildErrorRecord` list.
GCC/Clang format: `<file>:<line>:<col>: <severity>: <message>`.
CMake format: `CMake Error at <file>:<line>`.
Linker format: `undefined reference to '<symbol>'`.
Unknown lines captured as `tool=unknown` with raw message.

Tests (10): GCC error parsed correctly, Clang error parsed, CMake error
parsed, linker error parsed, warning parsed with severity=warning, note
parsed with severity=note, multi-line error grouped, empty input returns
empty list, malformed line returns unknown record, deterministic parse order.

### Step 1361: AST node locator from build error (10 tests)

Create `editor/src/BuildErrorLocator.h`.

Given a `BuildErrorRecord` (file_path, line, col) and an open AST, finds
the AST node responsible. Strategy: find the node whose span contains
(line, col). If exact match fails, find the nearest enclosing function/class.
Populates `ast_node_id` on the record.

Tests (10): exact line/col match returns node, enclosing function found when
exact fails, file mismatch returns nullopt, line 0 returns nullopt, col 0
accepted (start of line), multiple errors located in one pass, deterministic
node selection, linker error (no file) returns nullopt gracefully, node_id
written to record, works across all supported languages.

### Step 1362: Build error classifier (10 tests)

Create `editor/src/BuildErrorClassifier.h`.

Classifies `BuildErrorRecord` into semantic error types:
- `missing_include` — #include not found
- `undefined_symbol` — undeclared identifier or linker undefined ref
- `type_mismatch` — incompatible types in assignment/call
- `missing_dependency` — cmake target/package not found
- `syntax_error` — parse-level failure
- `resource_limit` — OOM, stack overflow
- `unknown` — catch-all

Classification uses error_code patterns + message substring matching.

Tests (10): missing_include classified, undefined_symbol classified,
type_mismatch classified, cmake dependency failure classified, syntax_error
classified, linker undefined ref → undefined_symbol, warning not
misclassified as error type, unknown catch-all, deterministic classification,
JSON round-trip of classified record.

### Step 1363: Fix candidate generator (10 tests)

Create `editor/src/BuildFixCandidateGenerator.h`.

Given a classified `BuildErrorRecord` and the open AST, generates a ranked
list of `FixCandidate` objects. Each candidate has: `candidate_id`,
`description`, `confidence` (0.0–1.0), `mutation_type` (addImport,
changeType, addDependency, deleteNode, insertNode, manual), `mutation`
(JSON blob suitable for passing to `whetstone_mutate`).

Classification → candidate strategy:
- `missing_include` → addImport candidate for the likely header
- `undefined_symbol` → suggest declaration or import fix
- `type_mismatch` → suggest cast or type change
- `missing_dependency` → suggest vcpkg/apt install command (manual mutation)
- `syntax_error` → low-confidence deleteNode or manual
- `unknown` → manual with description only

Tests (10): missing_include produces addImport candidate, confidence > 0.5
for known patterns, confidence < 0.3 for unknown, mutation JSON round-trips,
manual candidate always produced as fallback, candidates ordered by confidence
desc, candidate_id non-empty, description human-readable, no candidates on
empty error list, deterministic output.

### Step 1364: Build iteration session integration (8 tests)

Create `editor/src/BuildIterationAdapter.h`.

Wraps a build attempt as an `AttemptRecord` + `ObservationRecord`:
- Runs build command via subprocess, captures stdout+stderr+exit_code
- Parses output into `BuildErrorRecord` list
- Computes `GapModel` (metric_type=error_count, current=errors.size(),
  target=0)
- Records attempt in provided `IterationSession`
- Returns list of `FixCandidate` for all errors

Tests (8): attempt appended to session, observation raw_text captured,
gap metric_type is error_count, gap converged when build succeeds,
session status transitions to converged on success, fix candidates returned,
attempt outcome=success on exit_code 0, attempt outcome=failure on non-zero.

### Step 1365: `whetstone_parse_build_output` MCP tool (8 tests)

Input: `{ "output": string, "tool": string? }`
Output: `{ "errors": BuildErrorRecord[], "warning_count": int, "error_count": int }`

Parses raw build output string, classifies each record, returns structured list.

Tests (8): tool registered, error_count correct, warning_count correct,
tool field respected, empty output returns empty errors, malformed input
returns structured error, classification applied to all records,
deterministic output shape.

### Step 1366: `whetstone_suggest_build_fix` MCP tool (8 tests)

Input: `{ "session_id": string?, "build_output": string,
          "file": string?, "line": int?, "col": int? }`
Output: `{ "candidates": FixCandidate[], "error_count": int }`

Parses output, locates in active AST buffer, classifies, generates candidates.

Tests (8): tool registered, candidates returned, confidence present on all,
mutation field present, manual fallback always present, missing AST graceful,
candidates sorted by confidence, deterministic output.

### Step 1367: `whetstone_run_build_iteration` MCP tool (8 tests)

Input: `{ "session_id": string, "build_command": string,
          "workspace": string }`
Output: `{ "attempt_id": string, "session_status": string,
           "error_count": int, "candidates": FixCandidate[] }`

Runs the build, records the attempt in the session, returns fix candidates.

Tests (8): tool registered, attempt_id returned, session_status updated,
error_count correct, candidates present on failure, session converged on
success, build timeout enforced (default 120s), deterministic output shape.

### Step 1368: Sprint 113 integration summary + regression (8 tests)

Create `editor/src/Sprint113IntegrationSummary.h`.

End-to-end: start session → run_build_iteration (failing build) → verify
attempt recorded + candidates returned → apply top candidate → run again →
verify error count reduced.

Tests (8): headers constructable, tools registered, end-to-end flow works,
gap model improves after fix applied, session transitions correctly, no header
exceeds 600 lines, deterministic snapshots, full prior regression passes.

---

## Architecture Gate

- Build command execution via popen/std::system — not a shell injection vector
  (command is passed as-is from MCP caller; caller must sanitize inputs)
- Confidence scores are heuristic — never presented as guarantees
- Manual fallback candidate always generated so the loop is never stuck
- Max 600 lines per header
