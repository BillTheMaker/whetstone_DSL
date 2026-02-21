# Sprint 112 Plan: Environment Snapshot and Diff

## Context

Before iterating on a failing environment, you must read it deterministically.
"Environment" means the intersection of: installed packages, active config files,
running processes, and file system state that your project depends on. Without a
structured snapshot, the iteration session has no baseline to compare against and
no way to tell whether a change actually helped.

Sprint 112 adds `EnvironmentSnapshot` — a structured, serializable record of the
environment at a point in time — and `EnvironmentDiff` — the delta between two
snapshots. These are the observability primitives that the build, test, and setup
feedback loops in later sprints will use.

---

## Goals

1. Define `EnvironmentSnapshot` schema covering deps, configs, processes, and paths
2. Implement probes that read each domain from the actual system
3. Define `EnvironmentDiff` as a typed delta between two snapshots
4. Expose snapshot and diff as MCP tools
5. Produce a setup verification report (what is present vs what is needed)

---

## Steps

### Step 1349: `EnvironmentSnapshot` core schema (12 tests)

Create `editor/src/EnvironmentSnapshot.h`.

Fields: `snapshot_id`, `captured_at`, `workspace_root`, `platform`
(os, arch, shell), `packages[]` (name, version, source: apt/pip/npm/cargo/etc.),
`config_files[]` (path, hash, key_values[] for known formats),
`processes[]` (name, pid, port, status), `env_vars[]` (key, value — filtered
to relevant set, never secrets).

Tests (12): constructable with defaults, snapshot_id non-empty, platform fields
populated, packages list starts empty, config list starts empty, process list
starts empty, env_vars list starts empty, JSON serialization, JSON deserialization,
unknown package source passthrough, schema version present, deterministic output.

### Step 1350: Package state probe (10 tests)

Create `editor/src/PackageProbe.h`.

Probes installed packages from available managers on the current platform.
Strategy: try each manager (dpkg-query, pip list, npm list -g, cargo --list)
via subprocess, parse stdout into PackageRecord rows, tag with source.
Failures are non-fatal — missing manager produces empty list for that source.

Tests (10): probe runs without crashing, result is parseable, known-installed
package appears in results, unknown package absent, source tag correct,
failed probe returns empty not error, deterministic sort order, version field
populated, JSON round-trip, timeout respected (default 5s).

### Step 1351: Config file probe (10 tests)

Create `editor/src/ConfigFileProbe.h`.

Scans workspace for known config files: `.env`, `*.yaml`, `*.toml`, `*.json`
(at workspace root depth only — no deep recursion). Reads content, computes
SHA-256 hash, extracts key_values for .env and simple YAML/TOML maps.
Sensitive keys (PASSWORD, SECRET, KEY, TOKEN) are redacted to `"[REDACTED]"`.

Tests (10): .env file parsed into key_values, sensitive keys redacted,
hash computed and stable, yaml keys extracted, toml keys extracted,
json keys extracted (top-level only), missing file produces absent entry,
hash changes on content change, deterministic key ordering, depth limit
respected.

### Step 1352: Process state probe (10 tests)

Create `editor/src/ProcessProbe.h`.

Lists running processes filtered to a known-relevant set (configurable list of
process name substrings). Uses `/proc` on Linux, `ps` subprocess on macOS.
Captures name, pid, and listening ports (from `ss -tlnp` or `netstat`).

Tests (10): probe runs without crashing, result parseable, known process
appears when running, absent process not in list, port captured when listening,
no port when not listening, deterministic sort, name filter respected,
JSON round-trip, timeout respected.

### Step 1353: `EnvironmentDiff` schema and computation (10 tests)

Create `editor/src/EnvironmentDiff.h`.

Compares two `EnvironmentSnapshot` instances. Fields: `added_packages[]`,
`removed_packages[]`, `changed_packages[]` (name + old_version + new_version),
`added_configs[]`, `removed_configs[]`, `changed_configs[]` (path + old_hash
+ new_hash), `added_processes[]`, `removed_processes[]`, `unchanged` (bool —
true when all diff lists are empty).

Tests (10): empty diff on identical snapshots, unchanged true on identical,
added package detected, removed package detected, changed package detected,
added config detected, changed config hash detected, process added detected,
JSON round-trip, deterministic diff ordering.

### Step 1354: Snapshot persistence (sidecar integration) (8 tests)

Extend `SessionStore.h` (or add `SnapshotStore.h`) to persist
`EnvironmentSnapshot` alongside iteration sessions.

Convention: `.whetstone/snapshots/<snapshot_id>.json`

Tests (8): save creates file, load round-trips, list returns ids, overwrite
preserves new data, missing snapshot returns nullopt, atomic write, path
derivation correct, sidecar dir created on first save.

### Step 1355: Setup verification report (8 tests)

Create `editor/src/SetupVerificationReport.h`.

Given a `RequiredEnvironment` spec (a list of expected packages, config files,
and processes) and a current `EnvironmentSnapshot`, produces a structured report:
`satisfied[]`, `missing[]`, `version_mismatch[]`, `ready` (bool — true when
missing and version_mismatch are both empty).

Tests (8): ready true when all present, missing package detected, version
mismatch detected, correct version passes, empty required produces ready=true,
JSON round-trip, deterministic output, report serializable.

### Step 1356: `whetstone_snapshot_environment` MCP tool (8 tests)

Input: `{ "workspace": string, "process_filter": string[]? }`
Output: full `EnvironmentSnapshot` JSON + `snapshot_id`.

Runs all three probes (packages, configs, processes) and persists result.

Tests (8): tool registered, snapshot_id returned, packages field present,
configs field present, processes field present, snapshot persisted to disk,
invalid workspace returns structured error, deterministic output shape.

### Step 1357: `whetstone_diff_environments` MCP tool (8 tests)

Input: `{ "before_id": string, "after_id": string }`
Output: `EnvironmentDiff` JSON + `unchanged` bool.

Loads both snapshots from store, computes diff, returns result.

Tests (8): tool registered, unchanged true on same snapshot, added package
appears in diff, removed package appears, changed package appears, missing
snapshot_id returns error, diff persisted to session if session_id provided,
deterministic output.

### Step 1358: Sprint 112 integration summary + regression (8 tests)

Create `editor/src/Sprint112IntegrationSummary.h`.

End-to-end test: capture snapshot → mutate environment (add a dummy config
file) → capture second snapshot → diff → verify change detected.

Tests (8): headers constructable, tools registered, end-to-end diff detects
change, verification report correct, sidecar files created, no header exceeds
600 lines, deterministic snapshots, full prior regression passes.

---

## Architecture Gate

- Probes are non-fatal: subprocess failure returns empty list, not panic
- No new dependencies — subprocess via std::system or popen, parsing in C++
- Sensitive data never stored in snapshots (redaction in ConfigFileProbe)
- Max 600 lines per header
- All MCP outputs stable and machine-readable
