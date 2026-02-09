# Whetstone DSL — Architecture & Coding Standards

> **Audience:** All agents (Codex, Claude, future) working on this codebase.
> **Authority:** Treat these norms as hard constraints. Deviations require explicit user approval.

---

## File Size Limits

| Scope | Max Lines | Action When Exceeded |
|-------|-----------|----------------------|
| Header file (`.h`) | 600 | Split into multiple headers |
| `main.cpp` | 1500 | Extract panels/utilities to headers |
| Single function | 80 | Extract sub-routines |

---

## Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes / Structs | `PascalCase` | `LayoutManager`, `BufferState` |
| Methods / Functions | `camelCase` | `getChildren`, `recordSnapshot` |
| Member variables | `trailingUnderscore_` | `cursor_`, `currentPreset_` |
| Constants / Enum values | `PascalCase` | `BufferMode::Structured` |
| Files | `PascalCase.h` | Matches the primary class name |

---

## Architecture Patterns

### Header-Only
All components are `.h` files. The only `.cpp` files are `main.cpp`, `orchestrator_main.cpp`, `FileDialog.cpp`, and vendored backends.

### Panel Extraction
UI panels are free functions in their own headers, included and called from `main.cpp`:

```cpp
// panels/MenuBarPanel.h
#pragma once
#include "EditorState.h"
void renderMenuBar(EditorState& state);
```

### EditorState
Single state struct defined in `EditorState.h`. Panels receive it by reference — they never own global state.

### Generators
One file per language, inheriting from `ProjectionGenerator` base in `ProjectionGenerator.h`. The shared dispatch helper `dispatchGenerate()` eliminates duplicated `generate()` bodies.

### No God Objects
If a struct exceeds 50 fields, group related fields into sub-structs (e.g., `DiffState`, `LSPState`).

---

## Test Requirements

- **Minimum 2 tests per step** for non-trivial features.
- **Real assertions only:** Every test must use `assert()` or the `expect()` helper with actual value checks.
- **No print-only tests:** `std::cout << "PASS"` without a preceding assertion is forbidden.
- **Test pattern:** Use the standard `expect()` helper, `int passed/failed` counters, `return failed ? 1 : 0`.
- **Edge cases:** At least 1 edge case test per step (empty input, null, boundary).
- **Coverage continuity:** If a test is removed or simplified, add equivalent coverage in a new unit or integration test and note the replacement in the commit/progress log.

---

## Dependency Rules

- **Pin all FetchContent tags:** Use release tags (e.g., `v0.23.6`), never `master` or `main`.
- **vcpkg for system libs:** nlohmann-json, SDL2, imgui, tree-sitter core via vcpkg.
- **FetchContent for grammars:** Tree-sitter grammars and tinyfiledialogs via FetchContent with pinned tags.

---

## Code Quality

- **No stale TODOs:** If something is not implemented, mark it `// STUB:` with a reason.
- **No dead code:** Remove commented-out code; git has history.
- **Comments for "why" not "what":** The code shows what; comments explain non-obvious decisions.
- **No `goto`:** Use structured control flow.
- **Platform portability:** Use `#ifdef _WIN32` / `#else` guards for platform-specific code (e.g., `_popen` vs `popen`).
