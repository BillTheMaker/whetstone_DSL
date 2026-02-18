# Sprint 36 Plan: MCP Expansion — Architect Tools + Project Config

## Context

Sprint 36 exposes the Sprint 32 Architect Intake pipeline as MCP tools.
MarkdownSpecParser, TaskitemGeneratorV2, and TaskitemConfidenceAmbiguity are
fully implemented in C++ headers but not yet wired as MCP tools. Fix that,
add the `instructions` field to MCP initialize, and introduce per-project config.

Primary goals:
1. Expose architect intake + taskitem generation as callable MCP tools
2. Make Claude auto-understand the editor via the initialize instructions field
3. Per-project .whetstone.json config with CWD auto-discovery

Steps continue from Step 613 (last completed: Sprint35OperationalReadiness.h).

---

## Phase 36a: Architect Intake MCP Tools (Steps 614–618)

### Step 614: `whetstone_architect_intake` MCP Tool (12 tests)
Wire MarkdownSpecParser + RequirementNormalizationConflictDetector into a single
MCP tool call. Input: markdown spec string. Output: ParsedMarkdownSpec + conflict
signals + normalized requirements JSON.

### Step 615: `whetstone_generate_taskitems` MCP Tool (12 tests)
Wire TaskitemGeneratorV2 + TaskitemConfidenceAmbiguity. Input: normalized plan
JSON (from Step 614 output). Output: array of AnnotatedTaskitem with confidence,
ambiguityCount, escalate, reasons.

### Step 616: `whetstone_queue_ready` MCP Tool (12 tests)
Wire IntakeToQueueSimulationHarness. Input: array of AnnotatedTaskitem. Output:
{ready: bool, blockers: [...], readyCount, escalateCount, queueJson}.

### Step 617: MCP initialize — add `instructions` field (12 tests)
Add an `instructions` field to the MCP `initialize` response. Content: the text
from tools/claude/system_prompt.txt. This makes Claude auto-understand the editor
without needing a CLAUDE.md file. Test: initialize response includes instructions.

### Step 618: Phase 36a Integration (8 tests)
End-to-end: paste a markdown project spec → whetstone_architect_intake →
whetstone_generate_taskitems → whetstone_queue_ready → verified queue.

---

## Phase 36b: Per-Project Config + tools.json Sync (Steps 619–623)

### Step 619: `.whetstone.json` per-project config schema (12 tests)
Config file: `{workspace, defaultLanguage, agentRole, mcpWorkspaceAlias}`.
MCP server reads this on startup if present in --workspace dir. Lets different
projects have different language defaults without restart.

### Step 620: Config auto-discovery (walk up from CWD) (12 tests)
If no `--workspace` flag: walk up from CWD until .whetstone.json found. This
means `cd myproject && whetstone_mcp` just works.

### Step 621: `whetstone_set_workspace` MCP tool (12 tests)
Runtime tool: switch active workspace + language without restarting the binary.
Input: {workspace, language}. Required for HiveMind use case where one daemon
serves multiple projects.

### Step 622: tools.json refresh — add all Sprint 32-35 tools (8 tests)
Update tools/claude/tools.json to include the new Sprint 36a tools plus any
Sprint 32-35 features that have MCP-appropriate interfaces (debug session,
workflow recorder, benchmark harness). Bump version to 2.0.

### Step 623: Sprint 36 Integration + Summary (8 tests)
Validate: fresh Claude Code session + Whetstone MCP → Claude receives instructions
in initialize → calls whetstone_architect_intake on a real spec → generates queue.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 36a | 614–618 | 56 | Architect intake MCP tools |
| 36b | 619–623 | 56 | Per-project config + tools.json refresh |
| **Total** | **614–623** | **~112** | 10 steps |
