# Whetstone DSL — Claude Code Session Guide

> Read this before starting any work on the Whetstone editor.
> This project is independent of HiveMind. Do not conflate them.

---

## What Whetstone Is

Whetstone is a **general-purpose constructive editor and MCP server** for
cross-language code generation. It is NOT a HiveMind-specific tool — HiveMind
is one of many projects that consumes it. Whetstone's core capabilities:

- Semantic annotation DSL (SemAnno): annotate code with memory/ownership intent
- 19+ language parsers and generators (Python, C++, Rust, Go, JS/TS, Java, and more)
- Cross-language transpilation (AST-level, annotation-guided)
- 91+ MCP tools callable by any MCP client over stdio
- Headless `whetstone_mcp` binary + full ImGui GUI editor (`whetstone_editor`)

**Stack:** C++20, Dear ImGui + SDL2 + OpenGL3, nlohmann-json, tree-sitter, vcpkg, CMake

---

## Current State

| Item | Value |
|------|-------|
| Last step | **Step 1912** |
| Last sprint | **Sprint 276 — COMPLETE** |
| MCP tool count | **93 tools** |
| Architecture gate | All headers ≤ 600 lines (pre-existing BufferOps.h violation) |
| `whetstone_mcp` binary | `editor/build-native/whetstone_mcp` |
| Active track | **Polyglot Orchestrator** (Phase 3: LSP Orchestration — Sprint 276 complete. Next: Sprint 277) |

Sprint history: see `progress.md` (15k+ lines) — single authoritative file,
sprint summary table at top, step-by-step detail below.

---

## Sprint Workflow (TDD Pattern)

Every sprint follows this exact pattern:
1. Write a `sprintN_plan.md` with 5 steps
2. Each step: implement → test → verify → log to `progress.md`
3. Step naming: `stepNNN_test.cpp` in `editor/tests/`
4. Architecture check: run `file_limits_test` (headers ≤ 600 lines)
5. Sprint integration summary step (always the 5th step)
6. Full matrix verification before closing sprint

**Next sprint starts at Step 1898. Sprint 274: see `docs/polyglot_orchestrator_sprint_plan.md`.**
Sprint 273 (Steps 1893–1897) COMPLETE: ABIBoundaryExtractor, CHeaderEmitter, RustPythonBindingEmitter, GoCppBindingEmitter, integration test.

---

## Build

```bash
# Native build (this machine: Linux x86_64)
cd /home/bill/Documents/CLionProjects/whetstone_DSL
cmake --build editor/build-native --target whetstone_mcp --parallel

# Build the GUI editor too
cmake --build editor/build-native --target whetstone_editor --parallel

# Run a specific step test
./editor/build-native/step1887_test

# Run the architecture gate
./editor/build-native/file_limits_test

# Full rebuild from scratch
cd editor
cmake -S . -B build-native -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/home/bill/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build-native --parallel
```

**vcpkg packages** (already installed in `editor/build-native/vcpkg_installed/`):
nlohmann-json, sdl2, imgui, glad, tree-sitter, and language grammars via FetchContent.

---

## Key Directories

```
editor/
  src/             C++ source — all components as single-class headers
  src/mcp/         MCP tool registration headers (Register*.h)
  src/ast/         AST nodes, parsers, generators
  src/state/       EditorState sub-states
  src/panels/      ImGui panel components
  tests/           One test file per step (stepNNN_test.cpp)
  build-native/    Build output (do not commit binaries)
tools/
  claude/
    tools.json     MCP tool catalog (update when adding new tools)
```

---

## Adding a New MCP Tool (Pattern)

1. Implement the C++ class in `editor/src/MyFeature.h` (≤ 600 lines)
2. Create `editor/src/mcp/RegisterMyFeatureTools.h`:
   - Define tool name, input schema, handler lambda
   - Handler calls `MyFeature::run(args)` and returns JSON result
3. Add `#include "mcp/RegisterMyFeatureTools.h"` to `RegisterOnboardingAndAllTools.h`
4. Add entry to `tools/claude/tools.json`
5. Rebuild `whetstone_mcp` and verify tool appears via MCP initialize response

Reference implementations: `RegisterCodegenTools.h`, `RegisterContextTools.h`,
`RegisterValidationTools.h`, `RegisterMetricsTools.h`

---

## MCP Tools by Sprint (Key Tools)

| Tool | Sprint | What It Does |
|------|--------|-------------|
| `whetstone_architect_intake` | 36 | Markdown spec → normalized requirements + conflict report |
| `whetstone_generate_taskitems` | 36 | Requirements → AnnotatedTaskitems with confidence/escalate |
| `whetstone_queue_ready` | 36 | Validate taskitem set → confirmed queue |
| `whetstone_schema_to_cpp` | 41 | JSON Schema → typed C++ header + CMake snippet |
| `whetstone_generate_dispatch_table` | 41 | Job entries → C++ dispatch table header |
| `whetstone_assemble_context` | 43 | Workspace symbol lookup → token-budget-trimmed context slice |
| `whetstone_validate_taskitem` | 44 | Score taskitem self-containment (0–100), batch audit report |
| `whetstone_start_recording` | 45 | Start per-session tool-call recording |
| `whetstone_get_metrics` | 45 | Get session metrics + A/B comparison vs baseline |
| `whetstone_score_language_fitness` | 271 | Score AST features against language profiles → ranked list |

Full tool list: `tools/claude/tools.json` (91 entries).

---

## Architecture Constraints

- **No header may exceed 600 lines** — enforced by `file_limits_test`
- Single-class-per-header pattern throughout
- MCP tools are thin wiring only — business logic lives in the feature class
- `tools.json` must be updated every time a tool is added/removed
- Never touch `whetstone_mcp` for GUI-only features; keep headless binary lean

---

## Relationship to HiveMind

Whetstone generates code for HiveMind (schema→C++ structs, dispatch tables).
HiveMind is a **consumer** of the MCP tools. The two projects are independent:

- **Whetstone source:** `CLionProjects/whetstone_DSL/` — this directory
- **HiveMind source:** `Documents/hivemind/` — separate project
- Do NOT modify Whetstone when working on HiveMind, and vice versa

Whetstone MCP is registered at `Documents/.mcp.json` so it's available
as a tool server in Claude Code sessions opened from `Documents/`.
