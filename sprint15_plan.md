# Sprint 15 Plan: Orchestration Engine

## Context

Sprint 12 built the workflow model: WorkItems, TaskQueue, RoutingEngine, Workers,
ReviewGates. Sprint 15 makes it operational. The difference: Sprint 12 defined the
*parts*; Sprint 15 makes them *run together* as a continuous pipeline.

This is where the thesis becomes real. An architect creates a skeleton, annotates it,
and the orchestration engine takes over: routing tasks to the right worker, assembling
context, executing deterministic tasks, preparing bundles for LLM/SLM/human workers,
enforcing review gates, handling rejections, tracking progress, and surfacing the
right decisions to the right people at the right time.

**What Sprint 12 delivered:**
- WorkItem lifecycle (pending → ready → assigned → in-progress → review → complete)
- TaskQueue with priority + dependency resolution
- RoutingEngine with annotation-based dispatch rules
- Workers: Deterministic, Template, Agent (SLM/LLM), Human
- ContextAssembler with budget enforcement
- ReviewGate with auto-approve policies
- WorkflowPersistence sidecar

**What Sprint 15 adds:**
- **Orchestrator** — the main loop that advances the workflow automatically
- **Execution pipeline** — workers actually produce results that feed back in
- **Parallel dispatch** — independent tasks execute concurrently
- **Feedback loops** — rejection re-routes with accumulated context
- **MCP orchestration protocol** — Claude Code (or any MCP client) can drive
  the workflow step-by-step or let it auto-advance
- **Progress streaming** — real-time workflow status updates via MCP
- **Dependency graph visualization data** — for Sprint 19's GUI

---

## Phase 15a: Orchestrator Core (Steps 378-383)

*The main loop that drives workflows forward.*

### Step 378: Orchestrator.h — The Main Loop (12 tests)
**Goal:** Single class that owns the workflow lifecycle: advance ready tasks
through routing → context assembly → execution → review → completion.

**New file: `editor/src/Orchestrator.h`** (~400 lines)

**Orchestrator class:**
- `Orchestrator(workflowState, routingEngine, workerRegistry, contextAssembler, reviewGate)`
- `step() -> OrchestratorEvent` — advance one task one stage
- `advance() -> vector<OrchestratorEvent>` — advance all advanceable tasks one stage each
- `runToCompletion() -> WorkflowStats` — loop advance() until nothing more can be done
  without external input (all remaining tasks are human-routed or awaiting review)
- `runUntil(predicate) -> WorkflowStats` — advance until predicate says stop
- `getBlockers() -> vector<BlockerInfo>` — what's preventing further progress
  (human tasks, review queue, unresolved dependencies)

**OrchestratorEvent struct:**
```
OrchestratorEvent:
  type: string        // "routed" | "context-assembled" | "executed" | "auto-approved"
                      // | "sent-to-review" | "completed" | "rejected" | "blocked"
  itemId: string
  detail: json        // event-specific data (routing decision, result, etc.)
  timestamp: string
```

**BlockerInfo struct:**
```
BlockerInfo:
  type: string        // "needs-human" | "needs-review" | "needs-external-model"
  itemIds: vector<string>   // which items are blocked
  description: string       // human-readable explanation
```

**Orchestrator step() logic:**
1. Get ready tasks from queue (dependencies satisfied, not assigned)
2. For each ready task:
   a. Route it (RoutingEngine → RoutingDecision)
   b. Assemble context (ContextAssembler at decided width)
   c. If deterministic/template → execute immediately, get result
   d. If agent (SLM/LLM) → prepare context bundle, mark as "needs-external-model"
   e. If human → mark as "needs-human"
3. For deterministic results:
   a. Run through ReviewGate
   b. If auto-approved → complete, trigger dependency cascade
   c. If needs review → mark as "in-review"
4. Return all events from this step

**Tests:** single deterministic task completes in one step(), dependency cascade
on completion, human task blocks at "needs-human", agent task blocks at
"needs-external-model", runToCompletion processes all deterministic tasks, events
emitted for each stage, getBlockers reports human/review items, empty workflow
returns immediately, mixed workflow (some deterministic, some human) processes
what it can.

### Step 379: Parallel Dispatch + Batching (12 tests)
**Goal:** Independent tasks (no dependency between them) should be processed
in parallel within a single advance() call.

**Modifies: `editor/src/Orchestrator.h`**
- `advance()` processes all ready tasks, not just the first
- Independent tasks in the same priority level are batched
- Deterministic tasks executed sequentially (single-threaded) but
  logically parallel (all advance in one step)
- Context assembly for multiple tasks shares project-level context
  (assemble once, reuse for all project-width items)

**BatchResult struct:**
```
BatchResult:
  events: vector<OrchestratorEvent>
  itemsAdvanced: int
  itemsBlocked: int
  contextTokensSaved: int   // tokens saved by sharing project context
```

- `advanceBatch() -> BatchResult`

**Optimization: shared context**
- If 3 tasks all need project-width context, assemble project summary once
- Each task gets: shared project summary + task-specific local context
- Track tokens saved by this optimization

**Tests:** 3 independent tasks all advance in one call, shared context assembled
once (not 3 times), batch result counts correct, dependent tasks wait (only
independent ones advance), priority ordering within batch, context tokens saved
reported, empty batch, single-item batch, mixed independent + dependent.

### Step 380: Feedback Loop — Rejection Re-Routing (12 tests)
**Goal:** When a human rejects a work item, the rejection feedback becomes
part of the context for the next attempt. The routing engine may choose a
different worker type based on the feedback.

**Modifies: `editor/src/Orchestrator.h`** and `editor/src/RoutingEngine.h`

**Rejection flow:**
1. Human reviews result, provides rejection feedback (text + optional annotation changes)
2. Orchestrator marks item as rejected with feedback
3. Item re-enters ready queue with accumulated context:
   - Original skeleton intent
   - Previous attempt's result
   - Rejection feedback
   - Any annotation updates from the reviewer
4. Routing engine considers rejection history:
   - If deterministic worker was rejected → escalate to SLM or LLM
   - If SLM was rejected → escalate to LLM
   - If LLM was rejected → escalate to human (with full context of attempts)
   - If human was rejected → remains human with updated annotations

**RejectionHistory in WorkItem:**
```
RejectionHistory:
  attempts: vector<Attempt>

Attempt:
  workerType: string
  result: WorkItemResult
  feedback: string
  rejectedAt: string
  rejectedBy: string
```

**Tests:** rejected item re-enters queue, feedback included in next context,
routing escalation (deterministic → slm → llm → human), rejection history
preserved, multiple rejections accumulate, annotation updates from reviewer
applied, re-routed item has wider context than original, escalation doesn't
skip levels, human rejection stays human, rejection count tracked.

### Step 381: Progress Tracking + ETA (12 tests)
**Goal:** Track workflow progress with completion percentage, throughput
metrics, and rough time-to-completion estimates.

**New file: `editor/src/WorkflowProgress.h`** (~200 lines)

**ProgressSnapshot struct:**
```
ProgressSnapshot:
  totalItems: int
  completedItems: int
  completionPercent: float
  itemsPerMinute: float        // throughput (completed items / elapsed time)
  estimatedRemainingMinutes: float  // remaining / throughput
  byWorkerType: map<string, WorkerStats>
  currentPhase: string
  blockers: vector<BlockerInfo>
  startedAt: string
  lastActivityAt: string
```

**WorkerStats struct:**
```
WorkerStats:
  completed: int
  avgConfidence: float
  avgTokensUsed: int
  rejectionRate: float
```

**WorkflowProgress class:**
- `recordEvent(OrchestratorEvent)` — update metrics on each event
- `getSnapshot() -> ProgressSnapshot` — current state
- `getTimeline() -> vector<TimelineEntry>` — event log for visualization
- `getWorkerStats() -> map<string, WorkerStats>`

**Tests:** completion percent accurate, throughput calculated from events,
ETA reasonable (within 2x of actual for uniform tasks), worker stats
per type, rejection rate tracked, timeline records all events, empty
workflow returns 0% with no ETA, blockers reported in snapshot, snapshot
JSON serialization, progress monotonically increases (except rejections).

### Step 382: Orchestrator RPC + MCP (12 tests)
**Goal:** MCP tools for driving the orchestrator. An agent should be able
to step through a workflow, auto-advance deterministic tasks, or query
what's blocked and needs attention.

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — 6 new RPC methods:
- `orchestrateStep` — advance one task one stage, return event
- `orchestrateAdvance` — advance all ready tasks, return batch result
- `orchestrateRunDeterministic` — run all deterministic/template tasks to
  completion, stop at agent/human items
- `getBlockers` — what needs external input
- `getProgress` — current progress snapshot
- `submitExternalResult` — an agent submitting LLM/SLM output for a task
  that was prepared by the orchestrator

**Modifies: `editor/src/MCPServer.h`** — registerOrchestratorTools()
- `whetstone_orchestrate_step`
- `whetstone_orchestrate_advance`
- `whetstone_orchestrate_run_deterministic`
- `whetstone_get_blockers`
- `whetstone_get_progress`
- `whetstone_submit_result`

**Tool count:** 62+ (56 from Sprint 12 + 6 orchestrator tools)

**Key design: submitExternalResult**
This is how the Claude Code plugin (Sprint 18) will close the loop:
1. Orchestrator prepares context bundle for an LLM task
2. Claude Code receives the bundle via MCP
3. Claude Code sends the context to Claude, gets a response
4. Claude Code calls `submitExternalResult` with the generated code
5. Orchestrator runs it through review gate and continues

**Tests:** step advances one task, advance processes batch, runDeterministic
completes all template tasks and stops, getBlockers reports human tasks,
getProgress returns valid snapshot, submitExternalResult accepts LLM output
and advances the task, Linter role can read progress but not orchestrate,
MCP tool registration, combined: run deterministic → get blockers → submit
result → progress updates.

### Step 383: Phase 15a Integration Tests (8 tests)
**Goal:** End-to-end orchestration from skeleton to partial completion.

**Tests:**
1. Create 5-function skeleton (2 simple, 2 medium, 1 complex) → workflow →
   orchestrate → 2 deterministic complete, 2 agent-prepared, 1 human-blocked
2. Dependency chain respected: orchestrator doesn't advance B until A completes
3. Rejection escalation: deterministic result rejected → re-routed to LLM
4. Shared context optimization: 3 project-width tasks share context assembly
5. Progress tracking: completion % and throughput match actual events
6. submitExternalResult for agent task → review gate → auto-approve → complete
7. getBlockers shows exactly the human/review tasks
8. Workflow persistence: save mid-orchestration → reload → continue from same state

---

## Phase 15b: MCP Orchestration Protocol (Steps 384-388)

*Define the protocol that an MCP client (like Claude Code) uses to drive
a full project workflow. This is the contract Sprint 18 implements against.*

### Step 384: Workflow Session Protocol (12 tests)
**Goal:** Define the stateful MCP session protocol for project workflows.

**New file: `editor/src/WorkflowProtocol.h`** (~300 lines)

**Protocol phases (from the MCP client's perspective):**
1. **Init:** `whetstone_create_skeleton` → skeleton module created
2. **Model:** Series of `whetstone_add_skeleton_node` calls → project modeled
3. **Annotate:** `whetstone_infer_annotations` + manual annotation adjustments
4. **Plan:** `whetstone_create_workflow` → work items created and queued
5. **Route:** `whetstone_orchestrate_advance` → routing decisions made
6. **Execute:** `whetstone_orchestrate_run_deterministic` → auto tasks complete
7. **Assist:** Loop: `whetstone_get_blockers` → prepare context →
   call external model → `whetstone_submit_result`
8. **Review:** Human reviews items in review queue
9. **Complete:** All items complete, `whetstone_save_workflow`

**WorkflowSession struct:**
```
WorkflowSession:
  sessionId: string
  projectName: string
  currentPhase: string    // which protocol phase
  startedAt: string
  commands: vector<string>   // history of MCP commands issued
```

**Protocol helpers:**
- `getNextAction(session) -> ProtocolAction` — suggest what the MCP client
  should do next based on current workflow state
- `validateTransition(from, to) -> bool` — is this phase transition valid
- `getSessionSummary(session) -> json` — current state for client display

**Tests:** protocol phases in order, getNextAction suggests correctly at each
phase, invalid transition rejected, session tracks command history, summary
accurate at each phase, phase auto-detection from workflow state, session
persistence, protocol works with empty project.

### Step 385: Context Bundle Format (12 tests)
**Goal:** Standardize the context bundle that agent workers prepare for
external model invocation. This is what the MCP client receives and
forwards to an LLM.

**New file: `editor/src/ContextBundle.h`** (~200 lines)

**ContextBundle struct:**
```
ContextBundle:
  taskDescription: string       // human-readable task description
  skeletonCode: string          // the skeleton function/class to implement
  intent: string                // @Intent annotation value
  constraints: vector<string>   // from @Contract, @Risk, @Policy annotations
  contextCode: string           // surrounding code (scope-appropriate)
  projectSummary: string        // compact summary of project structure
  existingTests: string         // any test expectations
  previousAttempts: vector<AttemptSummary>  // from rejection history
  tokenBudget: int              // how many tokens the response should target
  outputFormat: string          // "code-only" | "code-with-explanation"
```

**AttemptSummary struct:**
```
AttemptSummary:
  workerType: string
  generatedCode: string
  feedback: string
```

**ContextBundle builders:**
- `buildBundle(workItem, workerContext, routingDecision) -> ContextBundle`
- `bundleToPrompt(bundle) -> string` — render as a prompt string that any
  LLM can understand (not Claude-specific)
- `bundleToJson(bundle) -> json` — structured format for MCP transport
- `estimateBundleTokens(bundle) -> int`

**Tests:** bundle contains all fields, bundleToPrompt readable and structured,
rejection history included when present, token estimate reasonable, budget
respected, constraints from annotations included, intent from @Intent,
empty fields handled gracefully, bundleToJson roundtrip, prompt format
model-agnostic (no model-specific tokens).

### Step 386: Result Acceptance Protocol (12 tests)
**Goal:** Standardize how external results are validated, accepted, and
integrated back into the workflow.

**Modifies: `editor/src/Orchestrator.h`**

**ResultSubmission struct:**
```
ResultSubmission:
  itemId: string
  generatedCode: string
  confidence: float           // model's self-assessed confidence (optional)
  reasoning: string           // model's explanation (optional)
  suggestedAnnotations: vector<json>  // model may suggest annotations
```

**Acceptance pipeline:**
1. Parse generated code through Pipeline.parse() → verify it's valid syntax
2. Run AnnotationValidator on the result → check for violations
3. Run diagnostic pipeline → check for errors
4. Apply ReviewGate policy → auto-approve or send to review
5. If accepted: update WorkItem result, complete the task, cascade dependencies
6. If validation fails: reject with structured feedback, re-route

**ResultAcceptance struct:**
```
ResultAcceptance:
  accepted: bool
  validationPassed: bool
  diagnosticCount: int
  autoApproved: bool
  reviewRequired: bool
  validationErrors: vector<string>
```

**Tests:** valid code accepted, syntax error rejected with feedback, annotation
violation flagged, diagnostic errors prevent auto-approve, confidence below
threshold sends to review, suggested annotations attached to node, acceptance
pipeline runs all checks in order, partial acceptance (code valid but review
needed), multiple submissions for same item (latest wins), cascade on acceptance.

### Step 387: Orchestrator Event Stream (12 tests)
**Goal:** Real-time event stream that MCP clients can poll for workflow
progress. Foundation for Sprint 19's GUI visualization.

**New file: `editor/src/EventStream.h`** (~200 lines)

**EventStream class:**
- `emit(OrchestratorEvent)` — add event to stream
- `poll(sinceVersion) -> vector<OrchestratorEvent>` — get events since version
- `subscribe(callback)` — register listener (for GUI integration later)
- `getVersion() -> int` — current stream position
- `getRecent(count) -> vector<OrchestratorEvent>` — last N events

**Event types emitted:**
- `workflow.created` — new workflow initialized
- `task.routed` — routing decision made (includes decision details)
- `task.context-assembled` — context prepared (includes token count)
- `task.executed` — worker produced result
- `task.auto-approved` — review gate approved
- `task.sent-to-review` — awaiting human review
- `task.completed` — fully done
- `task.rejected` — human rejected
- `task.escalated` — re-routed after rejection
- `workflow.progress` — periodic progress snapshot
- `workflow.blocked` — nothing more can advance
- `workflow.complete` — all items done

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`**
- `getEventStream` — poll events since version
- `getRecentEvents` — get last N events

**Modifies: `editor/src/MCPServer.h`**
- `whetstone_get_event_stream` + `whetstone_get_recent_events`

**Tool count:** 64+ (62 + 2 event stream tools)

**Tests:** events emitted during orchestration, poll returns only new events,
version tracking correct, subscribe callback fires, event types cover all
workflow transitions, getRecent returns correct count, empty stream returns
nothing, high-frequency polling doesn't duplicate, event JSON serialization,
MCP tool registration.

### Step 388: Phase 15b Integration — Full Protocol Test (8 tests)
**Goal:** Simulate a complete MCP client session through the protocol.

**Tests:**
1. Full protocol walkthrough: init → model → annotate → plan → route → execute →
   submit external → review → complete
2. Event stream captures every transition in order
3. Context bundle for LLM task contains skeleton + intent + project summary
4. Result acceptance: valid code auto-approved for deterministic, review-required for LLM
5. Rejection → re-route → submit better result → accepted
6. Progress snapshot at each phase matches actual state
7. getNextAction correctly guides client through protocol
8. Session persistence: save mid-protocol → reload → resume

---

## Phase 15c: Advanced Routing + Optimization (Steps 389-393)

*Make the routing engine smarter and the orchestration more efficient.*

### Step 389: Routing Rules Engine (12 tests)
**Goal:** Make routing rules configurable and composable rather than hardcoded.

**New file: `editor/src/RoutingRules.h`** (~300 lines)

**RoutingRule struct:**
```
RoutingRule:
  name: string
  priority: int               // lower = higher priority
  conditions: vector<RuleCondition>    // all must match (AND)
  action: RoutingAction
```

**RuleCondition types:**
- `annotation(type, property, value)` — node has annotation with property match
- `complexity(op, threshold)` — cognitive complexity comparison
- `contextWidth(width)` — context width annotation match
- `nodeType(type)` — function vs class vs method
- `rejectionCount(op, threshold)` — number of previous rejections
- `language(lang)` — source language match
- `pattern(name)` — recognized pattern (getter, setter, constructor, etc.)

**RoutingAction:**
- `workerType: string`
- `reviewRequired: bool`
- `contextWidthOverride: string` (optional)
- `budgetMultiplier: float` (optional, for giving complex tasks more tokens)

**RulesEngine class:**
- `addRule(rule)` — add to rule set
- `evaluate(workItem) -> RoutingDecision` — first matching rule wins
- `getDefaultRules() -> vector<RoutingRule>` — sensible defaults
- `loadRules(json) / saveRules() -> json` — persistence

**Tests:** first matching rule wins, conditions AND together, annotation
condition matches, complexity threshold, pattern matching, default rules
sensible, custom rules override defaults, no match falls through to default,
rule priority ordering, rules persist to JSON, budgetMultiplier applied,
rejection count escalation.

### Step 390: Cost Estimation + Optimization (12 tests)
**Goal:** Before executing the workflow, estimate the total cost in tokens
and suggest optimizations.

**New file: `editor/src/CostEstimator.h`** (~200 lines)

**CostEstimate struct:**
```
CostEstimate:
  totalContextTokens: int      // total context tokens across all tasks
  totalOutputTokens: int       // estimated output tokens
  byWorkerType: map<string, int>  // tokens per worker type
  deterministicTasks: int      // tasks with zero token cost
  estimatedCost: float         // rough $ cost at current model pricing
  optimizationSuggestions: vector<string>
```

**CostEstimator class:**
- `estimate(workflowState, routingEngine) -> CostEstimate` — run routing
  on all items, sum up context budgets
- `suggest(estimate) -> vector<OptimizationSuggestion>` — recommendations:
  - "5 getter functions could use template worker (saving ~10k tokens)"
  - "3 file-width tasks could be narrowed to local (saving ~4.5k tokens)"
  - "Consider batching these 4 related tasks for shared context"

**OptimizationSuggestion struct:**
```
OptimizationSuggestion:
  description: string
  tokensSaved: int
  itemIds: vector<string>
  action: string        // "narrow-context" | "use-template" | "batch" | "skip-review"
```

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — `estimateCost` RPC
**Modifies: `editor/src/MCPServer.h`** — `whetstone_estimate_cost`

**Tool count:** 65+ (64 + 1 cost tool)

**Tests:** estimate returns reasonable token counts, deterministic tasks have
zero token cost, optimization suggestions generated, getter detection suggests
template, context narrowing suggested where possible, batch suggestion for
related tasks, empty workflow returns zero cost, large workflow (50 items)
completes in reasonable time, cost estimate JSON serialization, MCP tool
registration.

### Step 391: Workflow Templates (12 tests)
**Goal:** Pre-built workflow templates for common project patterns.

**New file: `editor/src/WorkflowTemplates.h`** (~250 lines)

**Templates:**
- **CRUD API:** generates skeleton for create/read/update/delete operations
  for a given entity. Deterministic routing for getters/setters, LLM for
  business logic, human review for security-sensitive operations.
- **Module Refactor:** given existing code, creates work items for extracting
  functions, renaming, restructuring. Mostly LLM with human review.
- **Cross-Language Port:** given source in language A, creates skeleton in
  language B with annotation-guided work items for each function/class.
  Template workers for simple functions, LLM for complex logic.
- **Test Suite Generation:** given production code, creates skeleton test
  functions with @Intent annotations describing what to test. SLM for
  simple unit tests, LLM for integration tests.
- **Legacy Modernization:** given old code, creates work items for updating
  idioms, replacing deprecated patterns, adding safety annotations.

**WorkflowTemplate class:**
- `getTemplates() -> vector<TemplateInfo>`
- `applyTemplate(templateName, params) -> WorkflowState` — creates populated
  workflow from template
- Templates are composable (e.g., cross-language port + test generation)

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — `listTemplates`, `applyTemplate`
**Modifies: `editor/src/MCPServer.h`** — `whetstone_list_templates`, `whetstone_apply_template`

**Tool count:** 67+ (65 + 2 template tools)

**Tests:** 5 templates available, CRUD template creates correct skeleton,
cross-language template populates work items with annotation-guided routing,
template params validated, test generation template creates @Intent annotations,
applyTemplate produces valid WorkflowState, composing templates works,
MCP tool registration, template list includes descriptions, empty params
uses defaults.

### Step 392: Dependency Graph Data (12 tests)
**Goal:** Export workflow dependency graph as structured data that Sprint 19's
GUI can visualize.

**New file: `editor/src/DependencyGraph.h`** (~200 lines)

**GraphData struct:**
```
GraphData:
  nodes: vector<GraphNode>
  edges: vector<GraphEdge>
```

**GraphNode:**
```
  id: string
  label: string
  type: string           // "function" | "class" | "method"
  status: string         // WorkItem status
  workerType: string     // routing decision
  priority: string
```

**GraphEdge:**
```
  from: string
  to: string
  type: string          // "depends-on" | "blocks"
```

- `buildDependencyGraph(workflowState) -> GraphData`
- `graphToJson(graph) -> json` — for MCP transport
- `getCriticalPath(graph) -> vector<string>` — longest dependency chain
  (determines minimum completion time)

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — `getDependencyGraph` (workflow context)
**Modifies: `editor/src/MCPServer.h`** — `whetstone_get_dependency_graph`

**Tool count:** 68+ (67 + 1 graph tool)

**Tests:** graph has correct node count, edges match dependencies, critical
path is longest chain, node status matches workflow state, worker type
populated from routing, JSON serialization, empty workflow produces empty
graph, single-node graph, complex graph (diamond dependencies), graph
updates after task completion, MCP tool registration.

### Step 393: Phase 15c Integration + Sprint 15 Summary (8 tests)
**Goal:** Full orchestration engine validation.

**Tests:**
1. Custom routing rules: override default to use template for all simple functions →
   verify template worker used
2. Cost estimation: 10-function workflow → estimate → suggestions include
   "use template for getters"
3. CRUD template: create → route → execute deterministic → submit LLM results →
   complete workflow
4. Dependency graph: 5-function chain → critical path = 5 → verify
5. Event stream: run full workflow → event stream captures all transitions
6. Protocol: full MCP client simulation through all 9 phases
7. Optimization: apply suggestion to narrow context → re-estimate → lower cost
8. Sprint 15 totals: 68+ MCP tools, orchestrator operational, protocol defined

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 15a | 378-383 | 68 | Orchestrator loop, parallel dispatch, feedback, progress, RPC |
| 15b | 384-388 | 56 | MCP protocol, context bundles, result acceptance, event stream |
| 15c | 389-393 | 56 | Rules engine, cost estimation, templates, dependency graph |
| **Total** | **378-393** | **~180** | 16 steps |

**MCP tool count projection:** 68+ tools

---

## Key Files

**New files:**
- `editor/src/Orchestrator.h` — main orchestration loop, step/advance/runToCompletion
- `editor/src/WorkflowProgress.h` — progress tracking, ETA, worker stats
- `editor/src/WorkflowProtocol.h` — MCP session protocol, phase management
- `editor/src/ContextBundle.h` — standardized context for external model invocation
- `editor/src/EventStream.h` — real-time event stream for workflow transitions
- `editor/src/RoutingRules.h` — configurable routing rules engine
- `editor/src/CostEstimator.h` — token cost estimation and optimization suggestions
- `editor/src/WorkflowTemplates.h` — pre-built workflow templates (CRUD, refactor, port, test, modernize)
- `editor/src/DependencyGraph.h` — dependency graph export for visualization

**Modified files:**
- `editor/src/HeadlessEditorState.h` — Orchestrator member
- `editor/src/HeadlessAgentRPCHandler.h` — ~10 new RPC methods
- `editor/src/MCPServer.h` — register 12+ new MCP tools
- `editor/src/RoutingEngine.h` — delegates to RoutingRules
- `editor/src/WorkItem.h` — RejectionHistory, AttemptSummary
- `editor/CMakeLists.txt` — test targets

---

## Architectural Notes

### Why the orchestrator doesn't call LLMs directly
The orchestrator prepares context bundles but never invokes a model. This is deliberate:
1. **Model-agnostic:** Any LLM/SLM can be used. The system doesn't prefer Claude, GPT, or local models.
2. **Cost control:** The human/agent decides when to spend tokens, not the orchestrator.
3. **Auditability:** Every token spent is traceable to an explicit submitExternalResult call.
4. **Offline capability:** The orchestrator can run all deterministic tasks without any API access.

### The Sprint 18 connection
Sprint 15 defines the *server side* of the orchestration protocol. Sprint 18 builds
the *client side* — the Claude Code plugin that:
1. Connects to the MCP server
2. Follows the workflow protocol
3. Receives context bundles
4. Sends them to Claude (or another model)
5. Submits results back
6. Presents human review items to the engineer

Sprint 15 is complete when an MCP client *could* drive a full workflow. Sprint 18
builds the client that *does*.
