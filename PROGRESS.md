# Whetstone DSL - Progress Tracker

> **Purpose:** Living document for cross-session, cross-LLM continuity.
> Updated after each work session. The git log is the authoritative source of truth.

---

## Project Overview

Whetstone is a semantic annotation DSL (SemAnno) and structured editor for cross-language code generation with memory strategy annotations. The core idea: annotate code with memory management intent (`@Reclaim(Tracing)`, `@Owner(Single)`, `@Lifetime(RAII)`, etc.) and generate correct target-language code from a single AST representation.

**Stack:** C++20, Dear ImGui + SDL2 + OpenGL3, nlohmann-json, vcpkg, CMake

---

## Sprint Summary

| Sprint | Steps | Status | Description |
|--------|-------|--------|-------------|
| Sprint 1 | — | Complete | MPS-based prototype (JetBrains MPS language plugin) |
| Sprint 2 | 1–38 | **Complete** | C++ editor stack: AST, serialization, generators, ImGui shell, orchestrator, agents |
| Sprint 3 | 39–75 | **In Progress** | Core functionality: C++ generator, tree-sitter, classical editing, optimization |

---

## Sprint 1: MPS Prototype (Complete)

Built the initial SemAnno language in JetBrains MPS:
- 33 AST concepts (Module, Function, Variable, statements, expressions, types, annotations)
- Editor definitions for all concepts
- TextGen definitions for Python output
- Phase1Example sandbox model

**Key files:** `languages/SemAnno/`, `solutions/SemAnno.sandbox/`

---

## Sprint 2: C++ Editor Stack (Steps 1–38) — COMPLETE

All 38 steps implemented and passing. Each step has a corresponding test (`step1_test.exe` through `step38_test.exe`).

### Phase 2a: AST in C++ (Steps 1–6)
- [x] Step 1: Base `ASTNode` class + `Module` concept
- [x] Step 2: Child links, `Function` and `Variable`
- [x] Step 3: All Statement, Expression, Type, Parameter concepts
- [x] Step 4: Annotation concepts (`DerefStrategy`, `OptimizationLock`, `LangSpecific`)
- [x] Step 5: JSON serialization (`toJson`)
- [x] Step 6: JSON deserialization + round-trip verification

### Phase 2b: Validation & Generation (Steps 7–11)
- [x] Step 7: Schema validation (`ASTSchema`)
- [x] Step 8: Python generator base (Module/Function)
- [x] Step 9: Python generator — statements and expressions
- [x] Step 10: Python generator — remaining concepts
- [x] Step 11: Python generator — annotation output

### Phase 2c: ImGui Editor Shell (Steps 12–15)
- [x] Step 12: Dear ImGui shell scaffolding (SDL2 + OpenGL3)
- [x] Step 13: Text viewport
- [x] Step 14: AST-to-text viewport
- [x] Step 15: Projection toggle

### Phase 2d: Orchestrator (Steps 16–22)
- [x] Step 16: Orchestrator process
- [x] Step 17: JSON-RPC server
- [x] Step 18: AST mutation via RPC
- [x] Step 19: Connect ImGui to orchestrator
- [x] Step 20: Mutation via UI
- [x] Step 21: Undo/Redo journal
- [x] Step 22: Undo/Redo UI

### Phase 2e: Emacs Integration (Steps 23–29)
- [x] Step 23: Emacs integration
- [x] Step 24: Orchestrator spawns Emacs
- [x] Step 25: AST-to-Elisp projection
- [x] Step 26: Orchestrator-to-Emacs RPC
- [x] Step 27: File operations via Emacs
- [x] Step 28: File operations via RPC
- [x] Step 29: AST-to-File synchronization

### Phase 2f: Round-trip & Agents (Steps 30–38)
- [x] Step 30: Emacs-to-MPS synchronization
- [x] Step 31: Tree-sitter integration (stubs)
- [x] Step 32: Import via tree-sitter
- [x] Step 33: Export via generator
- [x] Step 34: Full round-trip / C++ generator basic output
- [x] Step 35: C++ generator — memory strategies
- [x] Step 36: Agent API
- [x] Step 37: Agent batch mode
- [x] Step 38: OptimizationLock warnings

---

## Sprint 3: Core Functionality (Steps 39–75) — IN PROGRESS

### Phase 3a: C++ Generator Implementation (Steps 39–44) — COMPLETE
- [x] Step 39: CppGenerator basic skeleton
- [x] Step 40: Statement generation (Assignment, Return, If)
- [x] Step 41: Expression generation (BinaryOp, literals, precedence)
- [x] Step 42: Type generation (PrimitiveType → C++ types, containers)
- [x] Step 43: Memory strategy code generation (canonical annotations)
- [x] Step 44: C++-specific idioms (const, references, includes)

> Steps 39–44 have TDD tests that compile and pass. The CppGenerator and canonical
> memory annotation classes (`DeallocateAnnotation`, `LifetimeAnnotation`,
> `ReclaimAnnotation`, `OwnerAnnotation`, `AllocateAnnotation`) are implemented in
> `editor/src/ast/Generator.h`.

### Phase 3b: Tree-sitter Integration (Steps 45–49) — TDD STUBS ONLY
- [x] Step 45: TDD test written (compiles, passes basic assertions)
- [x] Step 46: TDD test written (compiles, passes basic assertions)
- [x] Step 47: TDD test written (compiles, passes basic assertions)
- [ ] Step 48: CST-to-AST mapping refinement — not started
- [ ] Step 49: Error recovery and diagnostics — not started

> Steps 45–47 have TDD test stubs that compile and pass placeholder assertions.
> Real tree-sitter C bindings are NOT yet linked. The `TreeSitterParser` in
> `Parser.h` uses stub implementations from Sprint 2 Step 31.

### Phase 3c: Classical Editing Mode (Steps 50–54) — TDD STUBS ONLY
- [ ] Step 50: Text editor component — not started
- [x] Step 51: TDD test written (does not link — depends on unimplemented code)
- [ ] Step 52: Syntax highlighting — not started
- [x] Step 53: TDD test written (does not link)
- [ ] Step 54: Emacs-style keybindings — not started

### Phase 3d: Emacs Integration Complete (Steps 55–58) — TDD STUBS ONLY
- [ ] Step 55: Emacs splash screen — not started
- [x] Step 56: TDD test written (does not link)
- [x] Step 57: TDD test written (does not link)
- [ ] Step 58: Mode-specific behavior — not started

### Phase 3e: Agent API Extend (Steps 59–63) — TDD STUBS ONLY
- [ ] Step 59: WebSocket agent endpoint — not started
- [x] Step 60: TDD test written (does not link)
- [x] Step 61: TDD test written (does not link)
- [x] Step 62: TDD test written (does not link)
- [x] Step 63: TDD test written (does not link)

### Phase 3f: Advanced Memory Management (Steps 64–67) — TDD STUBS ONLY
- [x] Step 64: TDD test written (does not link)
- [x] Step 65: TDD test written (does not link)
- [x] Step 66: TDD test written (does not link)
- [x] Step 67: TDD test written (compile errors — references undefined types: `HotColdAnnotation`, `InlineAnnotation`, etc.)

### Phase 3g: Optimization Pipeline (Steps 68–71) — TDD STUBS ONLY
- [x] Step 68: TDD test written (does not link)
- [x] Step 69: TDD test written (does not link)
- [x] Step 70: TDD test written (does not link)
- [x] Step 71: TDD test written (does not link)

### Phase 3h: Integration & Validation (Steps 72–75) — NOT STARTED
- [ ] Step 72: End-to-end pipeline test
- [ ] Step 73: Error handling and edge cases
- [ ] Step 74: Performance profiling and benchmarks
- [ ] Step 75: API documentation

---

## Build Infrastructure

Created in the most recent session:

| File | Purpose |
|------|---------|
| `editor/vcpkg.json` | vcpkg manifest (dependencies declaration) |
| `editor/CMakePresets.json` | Standardized CMake presets (Windows/Linux) |
| `installer/windows/build.ps1` | PowerShell build script (prerequisites, vcpkg, cmake, staging) |
| `installer/windows/setup.iss` | Inno Setup installer script (produces `.exe` installer) |
| `installer/linux/build.sh` | Bash build script for Linux |
| `installer/linux/install.sh` | Linux install script (system deps, desktop entry) |

### Building on Windows

```powershell
# Prerequisites: VS2022, CMake 3.20+, vcpkg at E:\vcpkg
cd E:\Whetstone_DSL\installer\windows
.\build.ps1 -Config Release -VcpkgRoot E:\vcpkg

# Then compile installer (requires Inno Setup 6):
& "C:\Users\Bill\AppData\Local\Programs\Inno Setup 6\ISCC.exe" setup.iss
```

### vcpkg Packages Required
- `nlohmann-json:x64-windows`
- `sdl2:x64-windows`
- `imgui[docking-experimental,opengl3-binding]:x64-windows`
- `glad:x64-windows`

### Known Issue: imgui SDL2 Backend
vcpkg's imgui 1.91.9 removed the `sdl2-binding` feature (only `sdl3-binding` exists). The SDL2 backend files are vendored locally in `editor/src/imgui_backends/` and compiled directly by CMake.

---

## Test Results (Last Verified)

**Steps 1–47:** All compile and pass (47 executables in `editor/build/Release/`)
**Steps 48–75:** Either not started or TDD stubs that don't link yet

The build only compiles `whetstone_editor` and `orchestrator` targets plus steps 1–47. Steps 51+ are TDD stubs that reference unimplemented code and will fail to link until their implementations exist.

---

## Key Source Files

| File | Contents |
|------|----------|
| `editor/src/ast/ASTNode.h` | All 33+ AST node classes, JSON serialization |
| `editor/src/ast/Generator.h` | PythonGenerator, CppGenerator, ElispGenerator, all canonical annotation classes |
| `editor/src/ast/Schema.h` | AST schema validation |
| `editor/src/ast/Parser.h` | TreeSitterParser (stub implementations) |
| `editor/src/Orchestrator.h` | Orchestrator: Emacs integration, file ops, undo/redo, agent API |
| `editor/src/main.cpp` | ImGui editor shell (SDL2 + OpenGL3) |
| `editor/src/orchestrator_main.cpp` | Orchestrator standalone process (JSON-RPC server) |
| `editor/CMakeLists.txt` | CMake build config (vcpkg-based) |

---

## Architecture Notes

- **Editor** (`whetstone_editor.exe`): ImGui-based GUI. Renders AST as structured editor panels with file tree, toolbar, projection toggles. Currently a Step 12 scaffold — functional but minimal.
- **Orchestrator** (`orchestrator.exe`): Standalone JSON-RPC server. Manages AST state, undo/redo journal, file I/O, Emacs daemon integration, agent API.
- **Communication**: Editor ↔ Orchestrator via JSON-RPC over stdin/stdout pipes. When launched standalone (no pipe), the editor runs in disconnected mode.
- **Generators**: AST → Python, C++, Elisp text output. C++ generator handles canonical memory annotations.
- **Parsers**: Text → AST via tree-sitter (currently stub implementations from Sprint 2).

---

## What's Next

The next logical work is **Phase 3b real implementation** (Steps 45–49): replace tree-sitter stubs with real C bindings. This requires:
1. Add `tree-sitter`, `tree-sitter-python`, `tree-sitter-cpp`, `tree-sitter-elisp` as CMake FetchContent dependencies
2. Replace `TreeSitterParser` stubs in `Parser.h` with real CST-to-AST mapping
3. Ensure the existing TDD tests for steps 45–47 pass with real implementations

Alternatively, jump to **Phase 3c** (Steps 50–54) for classical text editing mode, or **Phase 3e** (Steps 59–63) for WebSocket agent API — these are independent of tree-sitter.

---

## Session Log

| Date | Agent | Work Done |
|------|-------|-----------|
| Pre-Sprint 2 | Multiple | Sprint 1 MPS prototype, language definitions, textgen |
| Sprint 2 | Multiple | Steps 1–38 implemented sequentially |
| Sprint 3 start | Unknown | Sprint 3 plan written, TDD stubs for steps 39–71 committed |
| Sprint 3 | Unknown | Phase 3a steps 39–44 implemented (CppGenerator + canonical annotations) |
| Sprint 3 | Unknown | Steps 34–38 re-committed with different implementations (agent API, batch mode) |
| 2025-latest | Claude | Build infrastructure: vcpkg.json, CMakePresets.json, build.ps1, setup.iss, Linux scripts |
| 2025-latest | Claude | Fixed 7 build errors (ElispGenerator, Orchestrator, main.cpp, orchestrator_main.cpp) |
| 2025-latest | Claude | Vendored imgui SDL2 backend (vcpkg removed sdl2-binding feature) |
| 2025-latest | Claude | Created Windows installer (Inno Setup), verified it installs and runs |
| 2025-latest | Claude | Fixed editor hang when launched without orchestrator pipe |
