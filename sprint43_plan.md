# Sprint 43 Plan: Cross-File Context Assembly

## Context

Sprint 42 completes the backlog of unregistered MCP tools (tool count 84→86).
The architect intake pipeline (Sprints 32, 36) generates annotated taskitems with
`prerequisiteOps` lists — file paths and symbol names the executing agent needs.
Currently, agents read full files to satisfy those ops, negating much of the token
budget advantage taskitems are designed to create.

Sprint 43 adds surgical context slicing: given a taskitem's `prerequisiteOps`,
extract only the relevant declarations, function bodies, or type definitions from
each file — not the whole file. Respects a configurable token budget. Entirely
generic — no coupling to any external system.

---

## Goals

1. Index workspace files by symbol definition (fast lookup by name, not full-file scan)
2. Assemble minimal context slices for a given set of file paths + symbol names
3. Enforce a token budget — trim to fit, report what was dropped
4. Expose as `whetstone_assemble_context` MCP tool

---

## Steps

### Step 674: `WorkspaceFileIndex` — symbol definition index (12 tests)

Create `editor/src/WorkspaceFileIndex.h`.

Index all files under a workspace root. For each file, extract top-level
declarations (function signatures, class names, struct names, type aliases,
constants) using line-level heuristics — no full parse required for this index.

```cpp
struct SymbolLocation {
    std::string file;
    int         lineStart;
    int         lineEnd;    // end of signature or short body
    std::string symbolName;
    std::string symbolKind; // "function" | "class" | "struct" | "type" | "const"
};

class WorkspaceFileIndex {
public:
    static WorkspaceFileIndex build(const std::string& workspaceRoot);
    std::vector<SymbolLocation> find(const std::string& symbolName) const;
    std::vector<SymbolLocation> findInFile(const std::string& filePath) const;
    int fileCount() const;
    int symbolCount() const;
};
```

Tests (12): empty root returns empty index, single-file workspace indexes it,
function signatures found by name, class declarations found by name, struct
declarations found by name, findInFile returns all symbols in that file, find()
returns all matches across files, symbolCount > 0 after indexing real file,
fileCount matches input file count, duplicate symbol names all returned, build()
doesn't throw on unreadable file (skips gracefully), kind field is non-empty.

### Step 675: `ContextSliceAssembler` — extract minimal file sections (12 tests)

Create `editor/src/ContextSliceAssembler.h`.

Given a list of `{file, symbol_or_line_range}` requests and a workspace index,
extract only the relevant lines from each file. A "slice" is:
- For a named symbol: the lines from its declaration through the closing brace/semicolon
- For a line range: exactly those lines
- For a file with no symbol specified: first N lines (configurable, default 40)

```cpp
struct SliceRequest {
    std::string filePath;
    std::string symbolName; // empty = use lineHint or head
    int         lineHint;   // 0 = not specified
    int         headLines;  // fallback if no symbol match; default 40
};

struct ContextSlice {
    std::string filePath;
    std::string symbolName;
    int         lineStart;
    int         lineEnd;
    std::string content;    // the actual extracted lines
    bool        found;
};

class ContextSliceAssembler {
public:
    static std::vector<ContextSlice> assemble(
        const std::vector<SliceRequest>& requests,
        const WorkspaceFileIndex& index);
};
```

Tests (12): empty requests returns empty result, named symbol found returns
correct lines, unknown symbol returns head fallback, line range returns correct
lines, content non-empty on success, found==false when symbol missing, multiple
requests all processed, head fallback respects headLines parameter, function body
slice includes closing brace, class slice includes closing brace, file path
preserved in result, slice content is valid UTF-8 substring of source file.

### Step 676: `TokenBudgetEnforcer` — trim assembled context to budget (12 tests)

Create `editor/src/TokenBudgetEnforcer.h`.

Given a list of `ContextSlice` objects and a max token budget, trim the list
to fit. Strategy: priority order (slices earlier in the list are higher priority),
each slice is dropped whole if it doesn't fit. Returns the slices that fit plus
a report of what was dropped.

Token estimation: 1 token ≈ 4 characters (conservative approximation, no model
dependency). The enforcer does NOT call any model API.

```cpp
struct BudgetReport {
    int    totalSlices;
    int    includedSlices;
    int    droppedSlices;
    int    estimatedTokensUsed;
    int    budgetTokens;
    bool   budgetExceeded;   // true if any slice was dropped
    std::vector<std::string> droppedFiles;
};

class TokenBudgetEnforcer {
public:
    static std::pair<std::vector<ContextSlice>, BudgetReport>
        enforce(const std::vector<ContextSlice>& slices, int maxTokens);

    static int estimateTokens(const std::string& text);
};
```

Tests (12): empty slices returns empty + zero report, single slice under budget
kept, single slice over budget dropped, multiple slices: high-priority kept when
budget tight, droppedFiles list matches dropped slices, estimatedTokensUsed <=
budgetTokens when no drops, budgetExceeded==false when all fit, budgetExceeded==true
when any dropped, estimateTokens("") == 0, estimateTokens("abcd") == 1,
report.totalSlices == input count, report.includedSlices + report.droppedSlices
== report.totalSlices.

### Step 677: `whetstone_assemble_context` MCP tool (8 tests)

Add to `RegisterCodegenTools.h` (or new `RegisterContextTools.h` if file nears 600 lines).

Input schema:
```json
{
  "files": {
    "type": "array",
    "description": "List of slice requests.",
    "items": {
      "type": "object",
      "properties": {
        "path":        { "type": "string",  "description": "File path relative to workspace." },
        "symbol":      { "type": "string",  "description": "Symbol name to extract (optional)." },
        "line_hint":   { "type": "integer", "description": "Line number hint (optional)." },
        "head_lines":  { "type": "integer", "description": "Head fallback line count (optional, default 40)." }
      },
      "required": ["path"]
    }
  },
  "max_tokens": { "type": "integer", "description": "Token budget (default 8000)." }
}
```

Output:
```json
{
  "success": true,
  "slices": [
    { "file": "...", "symbol": "...", "line_start": 10, "line_end": 25, "content": "..." }
  ],
  "budget_report": {
    "total_slices": 3,
    "included_slices": 2,
    "dropped_slices": 1,
    "estimated_tokens_used": 1840,
    "budget_tokens": 2000,
    "budget_exceeded": true,
    "dropped_files": ["large_file.cpp"]
  }
}
```

Wire into MCPServer.h, RegisterOnboardingAndAllTools.h, and tools.json.
Tool count 86→87.

Tests (8): tool registered, empty files array returns success with empty slices,
valid file path returns slice, missing path arg returns error, symbol slicing
returns content, budget_report present in output, budget_exceeded==false when
all slices fit, tool count in tools.json == 87.

### Step 678: Sprint 43 Integration Summary (8 tests)

Create `editor/src/Sprint43IntegrationSummary.h`.
Record: tool_count_before=86, tool_count_after=87, steps_completed=5 (674-678),
files_added=[WorkspaceFileIndex.h, ContextSliceAssembler.h, TokenBudgetEnforcer.h,
Sprint43IntegrationSummary.h], success=true.

Tests (8): struct constructable, workspace_index_builds, slice_assembler_works,
budget_enforcer_works, tool_count_before==86, tool_count_after==87,
steps_completed==5, success==true.

---

## Architecture Gate (All Steps)

- Header-only C++ for all new source files (no new .cpp except test files)
- 600-line max per file
- No new vcpkg dependencies — uses only `std::filesystem` and string operations
- `estimateTokens` must not call any external model API
- All slicing is done on raw file text — no AST dependency (fast, portable)
- Full sprint regression at step 678
