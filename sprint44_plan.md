# Sprint 44 Plan: Taskitem Self-Containment Validator

## Context

The architect intake pipeline (Sprints 32, 36) produces `AnnotatedTaskitem` objects
with `confidence`, `escalate`, `reasons[]`, and `prerequisiteOps[]`. The token
reduction claim — that a well-formed taskitem lets an agent execute without reading
external context — depends on those fields being accurate and complete.

Sprint 44 adds a validation layer: given one or more taskitems, score how
self-contained they actually are, flag which `prerequisiteOps` don't resolve to
real workspace files, and produce a quality report that agents and humans can act on.

This sprint is entirely generic editor tooling. No coupling to any external system.

---

## Goals

1. Resolve each `prerequisiteOp` string to a real file path in the workspace
2. Score each taskitem's self-containment (0–100)
3. Produce a batch quality report with actionable improvement suggestions
4. Expose validation as `whetstone_validate_taskitem` MCP tool

---

## Steps

### Step 679: `PrerequisiteOpResolver` — resolve ops to workspace paths (12 tests)

Create `editor/src/PrerequisiteOpResolver.h`.

A `prerequisiteOp` string is free-form, e.g.:
- `"read editor/src/SchemaToCppGenerator.h"`
- `"read editor/src/mcp/RegisterASTTools.h lines 30-60"`
- `"read editor/CMakeLists.txt"`
- `"run cmake --build editor/build-native --target step664_test"`

Parse each op into a structured `ResolvedOp` and check whether the file referenced
(if any) exists under the workspace root.

```cpp
enum class OpKind { ReadFile, RunCommand, Unknown };

struct ResolvedOp {
    std::string  raw;           // original string
    OpKind       kind;
    std::string  filePath;      // extracted path (empty for non-file ops)
    bool         fileExists;    // false if filePath non-empty but not found
    int          lineStart;     // 0 if not specified
    int          lineEnd;       // 0 if not specified
};

class PrerequisiteOpResolver {
public:
    static ResolvedOp resolve(const std::string& op,
                              const std::string& workspaceRoot);
    static std::vector<ResolvedOp> resolveAll(
        const std::vector<std::string>& ops,
        const std::string& workspaceRoot);
};
```

Tests (12): empty op string → kind==Unknown, "read file.h" → kind==ReadFile,
extracted filePath non-empty for read op, fileExists==true when file present,
fileExists==false when file absent, line range parsed ("lines 30-60" → 30,60),
no line range → lineStart==0 lineEnd==0, "run cmake ..." → kind==RunCommand,
RunCommand has empty filePath, resolveAll processes multiple ops, raw field
preserved, unknown op format → kind==Unknown gracefully.

### Step 680: `SelfContainmentScorer` — score a taskitem 0–100 (12 tests)

Create `editor/src/SelfContainmentScorer.h`.

Score algorithm (points deducted from 100):
- **prerequisiteOps missing or empty:** −30
- **Each prerequisiteOp that doesn't resolve to an existing file:** −10 (max −30)
- **reasons[] missing or empty:** −20
- **reasons[] has < 3 entries:** −10
- **confidence not set (0 or missing):** −10
- **title empty:** −20
- **dependencyTaskIds present but non-empty (has unresolved dependencies):** −5

Score is clamped to [0, 100].

```cpp
struct ScoredTaskitem {
    std::string taskId;
    int         score;          // 0-100
    bool        selfContained;  // score >= 80
    std::vector<std::string> issues;  // human-readable deduction reasons
};

struct TaskitemInput {
    std::string              taskId;
    std::string              title;
    std::vector<std::string> prerequisiteOps;
    std::vector<std::string> reasons;
    int                      confidence;       // 0-100
    std::vector<std::string> dependencyTaskIds;
};

class SelfContainmentScorer {
public:
    static ScoredTaskitem score(const TaskitemInput& item,
                                const std::string& workspaceRoot);
};
```

Tests (12): fully specified taskitem scores >= 80, empty prerequisiteOps deducts 30,
missing reasons deducts 20, empty title deducts 20, unresolved file op deducts 10,
two unresolved files deducts 20, score clamped to 0 (not negative), score clamped
to 100 (not over), selfContained==true when score>=80, selfContained==false when
score<80, issues list non-empty when deductions made, issues empty when perfect score.

### Step 681: `TaskitemQualityAuditor` — batch report (12 tests)

Create `editor/src/TaskitemQualityAuditor.h`.

Given a list of `TaskitemInput` objects and a workspace root, score all of them
and produce a summary report with improvement suggestions.

```cpp
struct QualityAuditReport {
    int totalTaskitems;
    int selfContainedCount;     // score >= 80
    int warningCount;           // score 50-79
    int failingCount;           // score < 50
    double averageScore;
    std::vector<ScoredTaskitem> results;  // one per input item
    std::vector<std::string>    topIssues; // most common deduction reasons
};

class TaskitemQualityAuditor {
public:
    static QualityAuditReport audit(const std::vector<TaskitemInput>& items,
                                    const std::string& workspaceRoot);
};
```

Tests (12): empty input returns valid report with 0 counts, single passing item
counted, single failing item counted, averageScore computed correctly, topIssues
non-empty when issues exist, selfContainedCount + warningCount + failingCount ==
totalTaskitems, results.size() == input.size(), all results have taskId set,
mixed batch (some pass some fail) counted correctly, report is deterministic
(same input → same output), totalTaskitems matches input size, warningCount correct
for 50-79 range.

### Step 682: `whetstone_validate_taskitem` MCP tool (8 tests)

Add `whetstone_validate_taskitem` to `RegisterCodegenTools.h` (or a new
`RegisterValidationTools.h` if approaching 600 lines).

Input schema:
```json
{
  "taskitems": {
    "type": "array",
    "description": "One or more taskitems to validate.",
    "items": {
      "type": "object",
      "properties": {
        "task_id":           { "type": "string" },
        "title":             { "type": "string" },
        "prerequisite_ops":  { "type": "array", "items": { "type": "string" } },
        "reasons":           { "type": "array", "items": { "type": "string" } },
        "confidence":        { "type": "integer" },
        "dependency_task_ids": { "type": "array", "items": { "type": "string" } }
      },
      "required": ["task_id"]
    }
  },
  "workspace": { "type": "string", "description": "Workspace root to resolve file paths against." }
}
```

Output:
```json
{
  "success": true,
  "report": {
    "total_taskitems": 3,
    "self_contained_count": 2,
    "warning_count": 1,
    "failing_count": 0,
    "average_score": 87.3,
    "results": [
      { "task_id": "T1", "score": 95, "self_contained": true, "issues": [] },
      ...
    ],
    "top_issues": ["prerequisiteOps missing or empty", "reasons[] has fewer than 3 entries"]
  }
}
```

Wire into MCPServer.h, RegisterOnboardingAndAllTools.h, tools.json (86→88 or 87→88
depending on sprint 43 completion).

Tests (8): tool registered, empty taskitems array returns valid report with 0 counts,
single valid taskitem returns score, missing task_id returns error, prerequisite_ops
resolved against workspace, report.results.length matches input length,
self_contained_count + warning_count + failing_count == total, tool count updated.

### Step 683: Sprint 44 Integration Summary (8 tests)

Create `editor/src/Sprint44IntegrationSummary.h`.
Record: steps_completed=5 (679-683), files_added=[PrerequisiteOpResolver.h,
SelfContainmentScorer.h, TaskitemQualityAuditor.h, Sprint44IntegrationSummary.h],
success=true.

Tests (8): struct constructable, resolver_works, scorer_works, auditor_works,
steps_completed==5, files_added contains PrerequisiteOpResolver.h,
files_added contains SelfContainmentScorer.h, success==true.

---

## Architecture Gate (All Steps)

- Header-only C++ for all new source files
- 600-line max per file — split at `Extended.h` if approaching
- No new vcpkg dependencies — uses only std::filesystem and string operations
- Scoring algorithm is deterministic and purely computational (no model calls)
- `whetstone_validate_taskitem` must work against any workspace path, not just
  the current project — no hardcoded paths
- Full sprint regression at step 683
