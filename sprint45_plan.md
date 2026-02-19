# Sprint 45 Plan: Agent Session Metrics + A/B Harness

## Context

Two prior questions about Whetstone remain unanswered with real data:

1. **Does using Whetstone tools actually reduce token consumption?** The 90%+
   reduction claim is architectural — a well-formed taskitem lets an agent skip
   file-reading. But the claim has never been measured.

2. **Is the output quality equivalent or better?** Speed without quality is useless.

Sprint 45 builds the instrumentation to answer both. It records per-session
metrics (tool calls, estimated token cost, files read, duration) and provides
a lightweight A/B comparison framework: run the same bounded task with and without
Whetstone tools, compare results.

This is entirely generic editor tooling. The metrics format, the comparison harness,
and the MCP tools are not coupled to any external system or project.

---

## Goals

1. Record what an agent does during a session: which tools it called, how much
   context it consumed (estimated tokens), and how long each call took
2. Capture a structured task outcome (what changed, did tests pass)
3. Compare two runs of the same task (Whetstone-assisted vs baseline)
4. Expose recording control and report retrieval as MCP tools

---

## Steps

### Step 684: `AgentSessionRecorder` — per-session tool call instrumentation (12 tests)

Create `editor/src/AgentSessionRecorder.h`.

Records every MCP tool call within a session: tool name, input size (chars),
output size (chars), estimated tokens (input+output), duration (ms), and whether
it resulted in a file read (heuristic: tool name contains "read", "get_ast",
"assemble_context", or "workspace_list").

```cpp
struct ToolCallRecord {
    std::string toolName;
    int         inputChars;
    int         outputChars;
    int         estimatedTokens;  // (inputChars + outputChars) / 4
    long long   durationMs;
    bool        wasFileRead;
};

struct SessionRecord {
    std::string              sessionId;
    std::string              taskDescription;
    std::vector<ToolCallRecord> calls;
    int                      totalToolCalls;
    int                      totalEstimatedTokens;
    int                      fileReadCount;
    long long                totalDurationMs;
};

class AgentSessionRecorder {
public:
    void start(const std::string& sessionId, const std::string& taskDescription);
    void record(const ToolCallRecord& call);
    SessionRecord finish();
    static ToolCallRecord makeRecord(const std::string& toolName,
                                     const std::string& inputJson,
                                     const std::string& outputJson,
                                     long long durationMs);
};
```

Tests (12): start sets sessionId, record() adds to calls, finish() returns complete
record, totalToolCalls matches recorded count, totalEstimatedTokens is sum of call
tokens, fileReadCount counts only file-read calls, wasFileRead true for "read"
tools, wasFileRead false for non-file tools, estimatedTokens = (inputChars +
outputChars) / 4, durationMs preserved, taskDescription preserved in output,
empty session returns zero counts.

### Step 685: `TaskCompletionMetrics` — capture task outcome (12 tests)

Create `editor/src/TaskCompletionMetrics.h`.

Captures the observable outcome of a completed task: lines of code changed
(before/after diff), whether a provided test command succeeded, and a
quality rating (0–100) supplied by the agent or reviewer.

```cpp
struct TaskOutcome {
    std::string taskId;
    int         linesChanged;       // abs(after_lines - before_lines)
    int         linesAdded;
    int         linesRemoved;
    bool        testsProvided;
    bool        testsPassed;
    int         qualityRating;      // 0-100, agent-supplied
    std::string notes;
};

class TaskCompletionMetrics {
public:
    static TaskOutcome diff(const std::string& taskId,
                            const std::string& before,
                            const std::string& after,
                            bool testsPassed,
                            int qualityRating,
                            const std::string& notes = "");
    static int countLines(const std::string& text);
};
```

Tests (12): empty before and after → 0 changes, added lines counted correctly,
removed lines counted correctly, linesChanged == linesAdded + linesRemoved (net),
testsPassed preserved, qualityRating clamped to [0,100], notes preserved, taskId
preserved, countLines("") == 0, countLines("a\nb\n") == 2, countLines("a\nb") == 2,
both before and after empty → testsProvided==false when tests not given.

### Step 686: `ABTestComparison` — compare two session records (12 tests)

Create `editor/src/ABTestComparison.h`.

Given two `SessionRecord` objects (baseline and whetstone-assisted) for the same
task, compute the comparison metrics.

```cpp
struct ABComparisonResult {
    std::string taskId;

    // Token comparison
    int    baselineTokens;
    int    whetstoneTokens;
    double tokenReductionPct;     // (baseline - whetstone) / baseline * 100

    // File read comparison
    int    baselineFileReads;
    int    whetstoneFileReads;
    double fileReadReductionPct;

    // Duration comparison
    long long baselineDurationMs;
    long long whetstoneDurationMs;
    double    durationDeltaPct;   // positive = whetstone faster

    // Quality comparison (from TaskOutcome)
    int    baselineQuality;
    int    whetstoneQuality;
    int    qualityDelta;          // whetstone - baseline (positive = better)

    bool   whetstoneWon;          // tokenReductionPct > 0 && qualityDelta >= 0
};

class ABTestComparison {
public:
    static ABComparisonResult compare(
        const std::string&    taskId,
        const SessionRecord&  baseline,
        const SessionRecord&  whetstone,
        int                   baselineQuality,
        int                   whetstoneQuality);
};
```

Tests (12): tokenReductionPct computed correctly, fileReadReductionPct computed
correctly, durationDeltaPct computed correctly (positive = faster), qualityDelta =
whetstoneQuality - baselineQuality, whetstoneWon==true when tokens reduced and
quality >=, whetstoneWon==false when tokens higher, tokenReductionPct==0 when
equal tokens, tokenReductionPct negative allowed (whetstone used more tokens),
baselineTokens preserved, whetstoneTokens preserved, taskId preserved,
zero baseline tokens does not divide by zero (returns 0.0).

### Step 687: `whetstone_start_recording` + `whetstone_get_metrics` MCP tools (8 tests)

Add two MCP tools. Wire into MCPServer.h + RegisterOnboardingAndAllTools.h +
tools.json.

**`whetstone_start_recording`** — begins a new recording session.
```json
{
  "session_id":       { "type": "string",  "description": "Unique session identifier." },
  "task_description": { "type": "string",  "description": "What task this session is executing." }
}
```
Response: `{"success": true, "session_id": "..."}`

**`whetstone_get_metrics`** — retrieves and finalizes the current session record,
optionally comparing against a provided baseline JSON.
```json
{
  "session_id":      { "type": "string" },
  "baseline":        { "type": "object", "description": "Optional prior SessionRecord JSON for A/B comparison." },
  "quality_rating":  { "type": "integer", "description": "Agent's self-assessment 0-100 for this session." },
  "baseline_quality":{ "type": "integer", "description": "Quality rating for baseline (required if baseline provided)." }
}
```
Response: `{"success": true, "session": {...}, "comparison": {...} }` (comparison
only present if baseline was provided).

MCPServer needs an `AgentSessionRecorder` member to hold in-flight session state.

Tests (8): start_recording registers tool, get_metrics registers tool, both in
tools.json, tool count updated, start_recording returns success=true, get_metrics
returns session record, comparison present when baseline provided, comparison absent
when no baseline.

### Step 688: Sprint 45 Integration Summary (8 tests)

Create `editor/src/Sprint45IntegrationSummary.h`.
Record: steps_completed=5 (684-688), files_added=[AgentSessionRecorder.h,
TaskCompletionMetrics.h, ABTestComparison.h, Sprint45IntegrationSummary.h],
success=true.

Full sprint regression: all steps 684-688 pass.

Tests (8): struct constructable, recorder_works, metrics_works, comparison_works,
steps_completed==5, files_added contains AgentSessionRecorder.h,
files_added contains ABTestComparison.h, success==true.

---

## Architecture Gate (All Steps)

- Header-only C++ for all new source files
- 600-line limit — `AgentSessionRecorder.h` may need Extended split if state
  management grows; check at step 684
- No new vcpkg dependencies
- Token estimation must NOT call any model API — purely character-count heuristic
- `ABTestComparison::compare` must handle zero-token baseline without crashing
- The recording state in MCPServer is per-server-instance (not persisted to disk)
- Full sprint regression at step 688

---

## Using the A/B Harness

After Sprint 45, a comparison session looks like this:

**Baseline run (no Whetstone tools):**
1. `whetstone_start_recording` with session_id="baseline-T1"
2. Agent executes task using only native file reads (no Whetstone tools)
3. `whetstone_get_metrics` with session_id="baseline-T1" → save result as baseline JSON

**Whetstone-assisted run:**
1. `whetstone_start_recording` with session_id="ws-T1"
2. Agent executes same task using Whetstone tools (assemble_context, validate_taskitem, etc.)
3. `whetstone_get_metrics` with session_id="ws-T1", baseline=<prior result>
   → returns comparison with tokenReductionPct, fileReadReductionPct, qualityDelta

The comparison is self-contained — no external logging service, no network calls.
Results can be copied to a markdown file or fed back into architect_intake for review.
