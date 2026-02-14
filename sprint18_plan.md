# Sprint 18 Plan: Claude Code Plugin + MCP Workflow Tools

## Context

This is the payoff sprint. Sprints 12 and 15 built the workflow model and
orchestration engine. Sprint 18 builds the **client side** — the thing an
engineer actually installs and uses.

The deliverable: a Claude Code MCP tool configuration that an engineer adds
to their project, and suddenly Claude Code can orchestrate annotation-driven
workflows. The engineer says "modernize this module" and the system:
1. Creates a skeleton with inferred annotations
2. Routes tasks (deterministic for simple stuff, Claude for complex stuff)
3. Executes what it can automatically
4. Surfaces the hard decisions to the human with full context
5. Tracks progress and handles review

**This sprint doesn't build a custom plugin binary.** It builds the MCP server
tools and the prompt templates that make the existing `whetstone_mcp` binary
work as an effective Claude Code tool server. The "plugin" is a configuration
file pointing Claude Code at the whetstone_mcp server.

---

## Phase 18a: Claude Code Integration (Steps 417-422)

### Step 417: MCP Server Configuration + Discovery (12 tests)
- Generate `.mcp.json` configuration file for Claude Code
- Server manifest: all 68+ tools organized by category with descriptions
- Tool categories: AST, Diagnostics, Workflow, Routing, Project, File ops
- Auto-detection: `whetstone_mcp` binary path resolution
- Workspace detection: auto-find project root and language

### Step 418: Workflow Prompt Templates (12 tests)
- MCP prompt templates that guide Claude through the workflow protocol:
  - `architect_project` — "Analyze this codebase and create an annotated skeleton"
  - `modernize_module` — "Modernize this legacy code with safety annotations"
  - `port_to_language` — "Port this module from {source} to {target}"
  - `add_feature` — "Add {feature} to this project using the workflow system"
  - `review_pending` — "Review and approve/reject pending work items"
- Each prompt includes the workflow protocol steps the agent should follow
- Prompts reference specific MCP tools by name

### Step 419: Agent Workflow Loop (12 tests)
- The core interaction loop for an agent using the MCP tools:
  1. `whetstone_infer_annotations` on existing code
  2. `whetstone_create_skeleton` for new functionality
  3. `whetstone_create_workflow` to plan execution
  4. `whetstone_orchestrate_run_deterministic` for auto tasks
  5. `whetstone_get_blockers` to find what needs the agent
  6. For each agent task: read context bundle, generate code, `whetstone_submit_result`
  7. `whetstone_get_progress` to track completion
- Test that this loop works end-to-end through MCP

### Step 420: Human Review Interface via MCP (12 tests)
- Tools for presenting review decisions to the engineer:
  - `whetstone_get_review_queue` — items awaiting human review
  - `whetstone_get_review_context` — full context for a review item
    (original skeleton, generated code, annotations, confidence, reasoning)
  - `whetstone_approve_item` — approve with optional feedback
  - `whetstone_reject_item` — reject with required feedback
- Review context formatted for human readability (not just raw JSON)

### Step 421: Workspace Onboarding Flow (12 tests)
- First-time setup experience for a new workspace:
  1. Detect project languages from file extensions
  2. Index workspace files
  3. Run annotation inference on key files
  4. Suggest initial annotations and workflow
  5. Generate `.whetstone/` directory with sidecar files
- `whetstone_onboard_workspace` tool that does this in one call

### Step 422: Phase 18a Integration (8 tests)
- Full end-to-end: configure MCP → onboard workspace → create workflow →
  orchestrate → agent completes tasks → human reviews → complete
- All MCP tools documented with clear descriptions
- 75+ MCP tools total

---

## Phase 18b: Multi-Model Dispatch (Steps 423-427)

The orchestration engine routes tasks to "slm", "llm", or "human" — but
which specific model? Phase 18b adds model selection awareness.

### Step 423: Model Profile Registry (12 tests)
- ModelProfile struct: name, contextWindow, costPerToken, capabilities, speed
- Pre-configured profiles: claude-opus, claude-sonnet, claude-haiku, local-slm
- Routing engine maps: "llm" → claude-sonnet (default), "slm" → claude-haiku
- Configurable overrides in `.whetstone/config.json`

### Step 424: Context Window Optimization (12 tests)
- Given a model's context window, optimize the context bundle:
  - Small model (8k context) → aggressive truncation, local scope only
  - Medium model (32k) → file scope with budget
  - Large model (200k) → project scope, include related files
- Context assembly adapts automatically based on model profile

### Step 425: Batch Submission for Agent Tasks (12 tests)
- When multiple agent tasks are ready, batch them for efficient submission
- Group by context overlap (tasks in same file share context)
- Batch format: shared context once + per-task instructions
- Reduces total token cost by ~30-50% for related tasks

### Step 426: Cost Tracking + Reporting (12 tests)
- Track actual tokens used per task (from submitExternalResult metadata)
- Compare estimated vs actual costs
- Running cost total for the workflow
- Cost report: breakdown by worker type, language, task complexity

### Step 427: Phase 18b Integration + Sprint Summary (8 tests)
- Model profiles select appropriate context window
- Batch submission reduces total tokens vs individual
- Cost tracking matches expected for a 10-task workflow
- Sprint 18 totals: 75+ tools, workflow operational end-to-end

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 18a | 417-422 | 68 | Claude Code config, prompts, agent loop, review, onboarding |
| 18b | 423-427 | 56 | Model profiles, context optimization, batching, cost tracking |
| **Total** | **417-427** | **~124** | 11 steps |

---

## Key Architectural Decision

The "plugin" is not a custom binary or VS Code extension. It is:
1. The existing `whetstone_mcp` binary (headless MCP server)
2. A `.mcp.json` configuration file that tells Claude Code where to find it
3. MCP prompt templates that guide the agent through the workflow protocol
4. MCP tools organized into a coherent workflow

This means any MCP-compatible client (not just Claude Code) can drive Whetstone
workflows. The protocol is the product, not a proprietary integration.
