# Sprint 115 Plan: Requirements Derivation from AST

## Context

Environment setup is currently manual and implicit. A developer knows their
project needs Python 3.11, numpy, and a running PostgreSQL instance because
they remember building it — not because the system can read that from the code.

Sprint 115 makes requirements derivation deterministic: given an AST, walk all
import/include/dependency nodes and produce a structured `RequirementsManifest`
— a language-agnostic list of what the project needs to run. From that manifest,
generate an idempotent setup script in the appropriate package manager format
(apt, pip, npm, cargo, cmake/vcpkg). Cross-reference the manifest against an
`EnvironmentSnapshot` (Sprint 112) to produce a gap report: what's missing,
what's the wrong version, what's ready.

This is the "environment as code" step: the project's needs become a
machine-readable artifact that any agent can check, compare, and act on.

---

## Goals

1. Define `RequirementsManifest` schema derived from the AST
2. Walk all dependency nodes across supported languages
3. Emit setup scripts per package manager (apt, pip, npm, cargo, vcpkg)
4. Cross-reference manifest against an EnvironmentSnapshot to get a gap report
5. Expose derivation and gap report as MCP tools

---

## Steps

### Step 1379: `RequirementsManifest` schema (12 tests)

Create `editor/src/RequirementsManifest.h`.

Fields: `manifest_id`, `source_file` (optional — if derived from single file),
`workspace` (optional — if derived from full project), `language_runtimes[]`
(name, min_version, source), `packages[]` (name, version_constraint,
package_manager: apt/pip/npm/cargo/vcpkg/brew/unknown),
`system_services[]` (name, port — e.g. postgres:5432, redis:6379),
`env_vars_required[]` (key, description, has_default: bool),
`derived_at`, `schema_version`.

Tests (12): constructable with defaults, manifest_id non-empty, packages
list starts empty, runtimes list starts empty, services list starts empty,
env_vars list starts empty, JSON serialization, JSON deserialization,
package_manager enum round-trips, unknown manager passthrough, schema_version
present, deterministic output.

### Step 1380: Import/include dependency walker (10 tests)

Create `editor/src/DependencyWalker.h`.

Walks an open AST, finds all import/include/require nodes, and extracts
package names. Language-specific strategies:
- Python: `import X`, `from X import Y` → package name is top-level module
- C/C++: `#include <X>` → system header; `#include "X"` → local (skip)
- JavaScript/TypeScript: `require('X')`, `import ... from 'X'` → npm package
- Rust: `extern crate X`, `use X::` → cargo crate
- Java: `import X.Y.Z` → Maven group

Stdlib modules filtered via a known-stdlib list per language (extensible).

Tests (10): Python import extracted, Python stdlib filtered, C++ system header
extracted, C++ local include skipped, JS require extracted, TS import extracted,
Rust extern crate extracted, Java import extracted, batch walk across multiple
files, deterministic output order.

### Step 1381: Runtime version inferrer (10 tests)

Create `editor/src/RuntimeVersionInferrer.h`.

Infers language runtime requirements from project configuration files:
- Python: `pyproject.toml` `requires-python`, `.python-version`, `setup.py`
- Node.js: `package.json` `engines.node`, `.nvmrc`
- Rust: `rust-toolchain.toml`
- C++: CMake `CMAKE_CXX_STANDARD`
- Java: `pom.xml` `maven.compiler.source`

Falls back to "any recent version" if no constraint found (min_version = "").

Tests (10): pyproject.toml version read, .python-version read, package.json
engines read, .nvmrc read, rust-toolchain.toml read, cmake standard read,
pom.xml source read, missing file returns no-constraint, multiple constraints
take strictest, deterministic output.

### Step 1382: Service dependency detector (10 tests)

Create `editor/src/ServiceDependencyDetector.h`.

Detects service dependencies from config files and connection string patterns
in source. Scans `.env`, `config.yaml`, `appsettings.json` for patterns like
`POSTGRES_HOST`, `REDIS_URL`, `MONGO_URI`, `DATABASE_URL`. Maps to
`SystemServiceRecord` (name, port).

Also detects from source: `psycopg2.connect(...)`, `mongoose.connect(...)`,
`redis.Redis(...)` call patterns in the AST.

Tests (10): postgres from .env POSTGRES_HOST, redis from REDIS_URL, mongo
from MONGO_URI, mysql from DATABASE_URL pattern, psycopg2 call in AST,
mongoose call in AST, redis.Redis call in AST, no false positives on
unrelated config, deterministic output, unknown service captured as unknown.

### Step 1383: Setup script generator (10 tests)

Create `editor/src/SetupScriptGenerator.h`.

Given a `RequirementsManifest`, generates an idempotent shell script per
requested package manager. Scripts use guard patterns (apt: check dpkg,
pip: check importlib, npm: check node_modules, cargo: check cargo metadata).

Output: `SetupScript` with fields `package_manager`, `script_text`,
`estimated_packages` (count), `requires_sudo` (bool).

One `whetstone_generate_setup_script` call may return multiple scripts
(one per relevant package manager detected from the manifest).

Tests (10): apt script generated for apt packages, pip script generated,
npm script generated, cargo script generated, idempotent guard present in
all scripts, sudo flag set for apt, not set for pip/npm/cargo, empty package
list produces no-op script, deterministic output, script text is valid shell.

### Step 1384: Requirements gap report (8 tests)

Create `editor/src/RequirementsGapReport.h`.

Cross-references `RequirementsManifest` against `EnvironmentSnapshot`.

Fields: `ready` (bool), `satisfied[]` (package name),
`missing[]` (name + required_version + package_manager),
`version_mismatch[]` (name + required + installed),
`unverifiable[]` (packages where installed version can't be confirmed),
`service_gaps[]` (services in manifest not found in snapshot processes).

Tests (8): ready true when all satisfied, missing package detected, version
mismatch detected, correct version passes, service gap detected, unverifiable
when probe can't confirm, JSON round-trip, deterministic output.

### Step 1385: `whetstone_derive_requirements` MCP tool (8 tests)

Input: `{ "workspace": string?, "files": string[]? }`
Output: `RequirementsManifest` JSON.

Walks all open buffers (or specified files), runs DependencyWalker +
RuntimeVersionInferrer + ServiceDependencyDetector, merges results.

Tests (8): tool registered, packages field present, runtimes field present,
services field present, empty workspace returns empty manifest, single file
mode works, manifest_id returned, deterministic output.

### Step 1386: `whetstone_generate_setup_script` MCP tool (8 tests)

Input: `{ "manifest_id": string?, "manifest": RequirementsManifest? }`
Output: `{ "scripts": SetupScript[] }`

Accepts either a previously derived manifest by ID or an inline manifest.

Tests (8): tool registered, scripts returned, apt script present for C++
project, pip script present for Python project, idempotent guards in output,
empty manifest returns empty scripts, manifest_id lookup works, deterministic.

### Step 1387: `whetstone_verify_requirements` MCP tool (8 tests)

Input: `{ "manifest_id": string, "snapshot_id": string }`
Output: `RequirementsGapReport` JSON.

Loads both from store, computes gap report.

Tests (8): tool registered, ready true when all installed, missing reported,
mismatch reported, service gap reported, missing manifest_id error, missing
snapshot_id error, deterministic output.

### Step 1388: Sprint 115 integration summary + regression (8 tests)

Create `editor/src/Sprint115IntegrationSummary.h`.

End-to-end: open Python project → derive_requirements → snapshot_environment →
verify_requirements → generate_setup_script → verify gap report matches missing
packages in snapshot.

Tests (8): headers constructable, tools registered, end-to-end flow works,
gap report accurate, setup script produced, no header exceeds 600 lines,
deterministic snapshots, full prior regression passes.

---

## Architecture Gate

- Dependency walker uses AST node traversal only — no regex on raw source text
- Service detection from config files uses ConfigFileProbe (Sprint 112) —
  no duplicated parsing logic
- Setup scripts are never executed by Whetstone — generation only, execution
  is the caller's responsibility
- Max 600 lines per header
