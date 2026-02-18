# Sprint 39 Plan: Self-Hosting + Release Pipeline

## Context

Sprint 39 makes Whetstone build itself and ship as a product. Self-hosting
validates the entire constructive editing model on a real, complex C++ project.
The release pipeline (AppImage, HiveMind-push auto-update, plugin system) turns
the editor into something deployable across the drone fleet.

Primary goals:
1. Whetstone opens and navigates its own codebase
2. CMake build system generated from project AST
3. In-editor test runner replaces external build cycle
4. AppImage packaging + HiveMind auto-update mechanism
5. Plugin system for third-party language generators

---

## Phase 39a: Self-Hosting Pipeline (Steps 644–648)

### Step 644: `.whetstone.json` for the editor project itself (12 tests)
The Whetstone project describes itself using its own config format. This validates
that the config system works for real C++ projects, not just toy examples.
Also: the editor can open itself and navigate its own AST.

### Step 645: CMake generator from Whetstone project description (12 tests)
Given a project AST, generate CMakeLists.txt. Supports: FetchContent deps,
vcpkg integration, test targets, install rules. Lets Whetstone regenerate its own
build system from the AST — the ultimate self-hosting test.

### Step 646: In-editor test runner (12 tests)
Run the Whetstone test suite (stepNNN_test binaries) from within the chat panel:
[Run Tests] button → streams output → shows pass/fail per step → links failures
to the relevant source file. Replaces the external build+test cycle.

### Step 647: Self-modification safety guard (12 tests)
Detect when an agent-requested mutation targets a file that is part of the running
editor binary. Block with: "Cannot mutate live binary — use a build/restart cycle."
Prevents the editor from corrupting its own running code.

### Step 648: Phase 39a Integration (8 tests)
Self-hosting demo: open whetstone_DSL project in the editor → ask agent to add a
new MCP tool → agent generates the header, wires it in CMake → in-editor test
runner validates → self-modification guard active.

---

## Phase 39b: Distribution + Plugin System (Steps 649–653)

### Step 649: AppImage / .deb package generator (12 tests)
Generate packaging scripts from project metadata. Output: working AppImage build
script and debian/control. Makes `whetstone` installable as a system package.
Runs as a CI step in the project.

### Step 650: HiveMind-push auto-update mechanism (12 tests)
Drone worker type: `update_editor`. When HiveMind publishes a new editor version
to the apiary, drones on all nodes receive the update job, download the AppImage,
verify hash, replace binary, restart service. Zero-touch updates across the fleet.

### Step 651: Language generator plugin manifest (12 tests)
Define a plugin format: a .so / shared library that exports a `ProjectionGenerator`
subclass. Third parties can add new language generators without recompiling the
editor. Plugin discovery: scan workspace for whetstone-plugin-*.so.

### Step 652: Privacy-preserving local telemetry (12 tests)
Collect: session length, tool call counts, error rates, most-used generators.
Store locally in ~/.whetstone/telemetry.db. Aggregate weekly report. Never
transmits externally. Feeds the Active Inference engine's entropy scoring.

### Step 653: Sprint 39 Integration + Summary (8 tests)
Full release pipeline validated: build → package → AppImage → HiveMind deploy
→ all nodes updated. Plugin system tested with a new language generator.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 39a | 644–648 | 56 | Self-hosting + CMake generator + in-editor test runner |
| 39b | 649–653 | 56 | AppImage + HiveMind auto-update + plugin system |
| **Total** | **644–653** | **~112** | 10 steps |
