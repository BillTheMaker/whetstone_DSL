# Sprint 12 Plan: Workflow Model + C++ Depth

## Context

Sprint 11 completed the annotation foundation: Semanno format for all 67+ types,
validation rules across all 8 subjects, parser deepening with 9 new AST node types,
Kotlin + C# as languages 9 and 10, and the workflow annotation foundation (Subject 9
routing annotations, skeleton AST, annotation inference, inference-to-routing bridge).

Sprint 12 builds the **execution layer** on top of that foundation. The skeleton AST
from Sprint 11e describes *what* needs to be done; Sprint 12 adds *how* it gets done.
WorkItems track execution state, the routing engine reads annotations and dispatches
to the right worker type, and review gates ensure humans stay in the loop where they
should be.

In parallel, Sprint 12 deepens C++ AST coverage with 5 construct families from
`feature-requests.md`, progressing toward the self-hosting milestone where Whetstone
can parse its own headers.

**What Sprint 11 delivered that Sprint 12 builds on:**
- `SkeletonAST.h` — skeleton modules, task list generation, SkeletonTask struct
- `AnnotationInference.h` — multi-subject inference + routing bridge + suggestWorkerType()
- Subject 9 annotations: @ContextWidth, @Review, @Ambiguity, @Automatability, @Priority, @ImplementationStatus
- `ResponseBudget.h` — budget truncation + continuation tokens (Sprint 9c)
- `AgentPermissionPolicy.h` — 3-role system (Linter, Refactor, Generator)
- `Pipeline.h` — 8-stage parse→infer→validate→optimize→generate flow
- 42+ MCP tools, 10 language parsers/generators, 73+ annotation types

**Sprint 12 Deliverables:**
- **WorkItem execution model** with lifecycle state tracking
- **Task queue** with dependency resolution and priority scheduling
- **Routing engine** that maps annotations → worker dispatch decisions
- **Worker abstractions** (deterministic, agent, human) with context estimation
- **Review gates** with auto-approve rules and human review queue
- **7 new C++ AST node types**: enhanced ClassDeclaration (multiple inheritance),
  TemplateClass, IncludeDirective, PragmaDirective, MacroDefinition, EnumDeclaration,
  NamespaceDeclaration
- **C++ parser deepened** with inheritance, preprocessor, enum, namespace support
- **50+ MCP tools** (42 current + 8 workflow/routing tools)

---

## Phase 12a: WorkItem Model + Task Queue (Steps 320-325)

*The execution model that turns skeleton tasks into trackable, assignable work.*

### Step 320: WorkItem.h — Core Execution Model (12 tests)
**Goal:** Define the WorkItem lifecycle — the unit of work that flows through the
routing engine, gets assigned to a worker, produces results, and optionally goes
through human review.

**New file: `editor/src/WorkItem.h`** (~350 lines)

**WorkItem struct** (extends SkeletonTask concept):
```
WorkItem:
  // Identity (from SkeletonTask)
  id: string                    // unique work item ID (wi-001, wi-002, ...)
  nodeId: string                // AST node this work item targets
  nodeName: string              // human-readable name
  nodeType: string              // "function" | "class" | "method" | ...
  bufferId: string              // which buffer this belongs to

  // Routing (from annotations, set by routing engine)
  contextWidth: string          // "local" | "file" | "project" | "cross-project"
  workerType: string            // "deterministic" | "template" | "slm" | "llm" | "human"
  reviewRequired: bool          // whether output needs review before acceptance
  priority: string              // "critical" | "high" | "medium" | "low"
  dependencies: vector<string>  // work item IDs that must complete first

  // Lifecycle
  status: string                // "pending" | "ready" | "assigned" | "in-progress" |
                                // "review" | "complete" | "rejected"
  assignee: string              // worker ID or "human:<name>"
  createdAt: string             // ISO timestamp
  assignedAt: string
  completedAt: string

  // Result (populated after execution)
  result: WorkItemResult
```

**WorkItemResult struct:**
```
WorkItemResult:
  generatedCode: string         // the code produced by the worker
  astJson: json                 // the AST subtree produced
  diagnostics: vector<json>     // any diagnostics from validation
  confidence: float             // worker's confidence in the result (0.0-1.0)
  tokensBudget: int             // context tokens consumed
  tokensGenerated: int          // output tokens produced
  reasoning: string             // worker's explanation of decisions made
```

**Status transition rules:**
- `pending` → `ready` (when all dependencies complete)
- `ready` → `assigned` (when routing engine dispatches)
- `assigned` → `in-progress` (when worker begins)
- `in-progress` → `review` (if reviewRequired) or `complete` (if auto-approved)
- `review` → `complete` (human approves) or `rejected` (human rejects)
- `rejected` → `ready` (re-enters queue for re-routing with feedback)

**Helper functions:**
- `createWorkItem(skeletonTask, bufferId) -> WorkItem` — from SkeletonTask
- `isTerminal(status) -> bool` — complete or rejected-and-abandoned
- `canTransition(from, to) -> bool` — validates state machine
- `transitionWorkItem(item, newStatus) -> bool` — applies transition with timestamp
- `workItemToJson(item) -> json` / `workItemFromJson(json) -> WorkItem`

**Tests:** construction from SkeletonTask, status transitions (valid and invalid),
JSON roundtrip, timestamp population, result attachment, dependency tracking,
terminal state detection, ID generation.

### Step 321: TaskQueue.h — Priority Queue with Dependencies (12 tests)
**Goal:** Ordered queue that respects both priority levels and dependency chains.
Items whose dependencies are unsatisfied stay blocked; ready items are ordered
by priority then creation time.

**New file: `editor/src/TaskQueue.h`** (~300 lines)

**TaskQueue class:**
- `enqueue(WorkItem) -> void` — adds item, sets status to pending or ready
- `getReady() -> vector<WorkItem>` — returns items whose dependencies are all
  complete, ordered by priority (critical first) then creation time
- `peek() -> optional<WorkItem>` — next ready item without removing
- `dequeue() -> optional<WorkItem>` — removes and returns next ready item,
  transitions to assigned
- `complete(itemId) -> bool` — marks complete, triggers dependency re-evaluation
  (blocked items may become ready)
- `reject(itemId, feedback) -> bool` — marks rejected, re-enqueues as ready
  with feedback attached
- `getBlocked() -> vector<WorkItem>` — items waiting on dependencies
- `getByStatus(status) -> vector<WorkItem>` — filter by lifecycle status
- `size() -> int` / `readyCount() -> int` / `blockedCount() -> int`
- `getItem(itemId) -> optional<WorkItem>` — lookup by ID
- `updateItem(itemId, WorkItem) -> bool` — replace item (for status updates)

**Dependency resolution:**
- When `complete(itemId)` is called, scan all blocked items
- For each blocked item, check if all dependencies are now complete
- If so, transition from pending → ready
- O(n) scan is fine for project-scale queues (hundreds, not millions)

**Priority ordering:**
- critical=0, high=1, medium=2, low=3 (lower number = higher priority)
- Ties broken by createdAt (earlier first)

**Tests:** enqueue/dequeue ordering, priority ordering (critical before low),
dependency blocking (B blocked by A, A completes → B becomes ready), reject and
re-enqueue, getReady returns only unblocked items, empty queue behavior,
getByStatus filtering, size tracking, completion cascading (A→B→C chain),
mixed priorities with dependencies.

### Step 322: WorkflowState.h — Project-Level Workflow Tracking (12 tests)
**Goal:** Top-level state that manages the full workflow lifecycle for a project:
from skeleton creation through routing, execution, review, and completion.

**New file: `editor/src/WorkflowState.h`** (~300 lines)

**WorkflowPhase enum:**
- `Modeling` — architect is creating/annotating skeletons
- `Routing` — routing engine is assigning worker types
- `Executing` — workers are producing results
- `Reviewing` — human review in progress
- `Complete` — all work items complete

**WorkflowState class:**
- `queue: TaskQueue` — the project's task queue
- `phase: WorkflowPhase` — current workflow phase
- `projectName: string`
- `createdAt: string`

**Methods:**
- `populateFromSkeleton(module) -> int` — walks skeleton AST, creates WorkItems
  from SkeletonTask list, enqueues all, returns count
- `routeAll(RoutingEngine&) -> int` — routes all ready items (Sprint 12b),
  returns count routed
- `getStats() -> WorkflowStats` — counts by status, by worker type, by priority
- `getPhase() -> WorkflowPhase` — auto-computed from item statuses
- `getHistory(itemId) -> vector<StatusChange>` — audit trail for an item
- `toJson() / fromJson()` — full state serialization

**WorkflowStats struct:**
```
WorkflowStats:
  total: int
  pending: int, ready: int, assigned: int, inProgress: int
  review: int, complete: int, rejected: int
  byWorkerType: map<string, int>    // deterministic: 5, llm: 3, human: 2
  byPriority: map<string, int>      // critical: 1, high: 4, medium: 5
  completionPercent: float
```

**StatusChange struct** (audit trail):
```
StatusChange:
  itemId: string
  fromStatus: string
  toStatus: string
  timestamp: string
  actor: string           // "routing-engine" | "worker:det-001" | "human:bill"
  note: string            // rejection reason, routing rationale, etc.
```

**Tests:** populateFromSkeleton creates correct item count, phase auto-detection
(all pending → Modeling, some assigned → Executing, some in review → Reviewing,
all complete → Complete), stats accuracy, history tracking, JSON roundtrip,
empty workflow behavior, phase transitions.

### Step 323: Workflow Sidecar Persistence (12 tests)
**Goal:** Save and load workflow state to `.whetstone/<project>.workflow.json`
so workflows survive across sessions.

**New file: `editor/src/WorkflowPersistence.h`** (~200 lines)
- `workflowSidecarPath(workspaceRoot, projectName) -> string`
  — returns `.whetstone/<projectName>.workflow.json`
- `saveWorkflow(workspaceRoot, state) -> SaveResult`
  — serializes WorkflowState to JSON, writes to sidecar path
- `loadWorkflow(workspaceRoot, projectName) -> optional<WorkflowState>`
  — reads sidecar, deserializes, returns nullopt if not found
- `deleteWorkflow(workspaceRoot, projectName) -> bool`
  — removes sidecar file

**SaveResult struct:**
```
SaveResult:
  success: bool
  path: string
  bytesWritten: int
  itemCount: int
```

**Tests:** save/load roundtrip, all work item fields preserved, status history
preserved, missing file returns nullopt, save creates parent directories,
delete removes file, multiple workflows in same workspace, workflow with
results attached, empty workflow save/load.

### Step 324: Workflow RPC + MCP Tools (12 tests)
**Goal:** Expose workflow management through RPC methods and MCP tools so agents
can create, inspect, and advance workflows.

**Modifies: `editor/src/HeadlessEditorState.h`**
- Add `WorkflowState` member (optional, null until created)
- Include WorkflowState.h, WorkflowPersistence.h

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — 8 new RPC methods:
- `createWorkflow` — create WorkflowState, populate from active buffer's skeleton
  - params: projectName
  - returns: { itemCount, phase, stats }
- `getWorkflowState` — return current workflow stats + phase
  - returns: { phase, stats, readyCount, blockedCount }
- `getReadyTasks` — return work items ready for assignment
  - returns: { items: [...], count }
- `getWorkItem` — return a single work item's full details
  - params: itemId
  - returns: { workItem with result if present }
- `assignTask` — move a ready task to assigned status
  - params: itemId, assignee
  - returns: { success, item }
- `completeTask` — mark a task complete with result
  - params: itemId, result (generatedCode, confidence, reasoning)
  - returns: { success, newlyReady: [...] }
- `rejectTask` — reject a task back to the queue
  - params: itemId, reason
  - returns: { success }
- `saveWorkflow` — persist workflow to sidecar
  - returns: { success, path, bytesWritten }

**Modifies: `editor/src/AgentPermissionPolicy.h`**
- `getWorkflowState` / `getReadyTasks` / `getWorkItem` — read-only (all roles)
- `createWorkflow` / `assignTask` / `completeTask` / `rejectTask` / `saveWorkflow`
  — mutation (Refactor/Generator only)

**Modifies: `editor/src/MCPServer.h`** — registerWorkflowExecutionTools()
- `whetstone_create_workflow` — create and populate workflow
- `whetstone_get_workflow_state` — inspect current workflow
- `whetstone_get_ready_tasks` — get assignable items
- `whetstone_get_work_item` — inspect single item
- `whetstone_assign_task` — assign a ready task
- `whetstone_complete_task` — mark task complete with result
- `whetstone_reject_task` — reject task with feedback
- `whetstone_save_workflow` — persist to sidecar

**Tool count:** 50+ (42 from Sprint 11 + 8 workflow execution tools)

**Tests:** createWorkflow from skeleton module, getReadyTasks ordering, assignTask
transitions, completeTask triggers dependency cascade, rejectTask re-enqueues,
getWorkItem returns full details, Linter role can read but not mutate, MCP tool
registration (8 new tools, 50+ total), saveWorkflow round-trip.

### Step 325: Phase 12a Integration Tests (8 tests)
**Goal:** End-to-end workflow lifecycle from skeleton to completion.

**Tests:**
1. Create skeleton module → createWorkflow → verify item count matches skeleton
2. Priority ordering: critical task appears before low in getReadyTasks
3. Dependency chain: A blocks B blocks C → complete A → B ready → complete B → C ready
4. Full lifecycle: create → assign → in-progress → complete with result → verify
5. Rejection flow: assign → reject with feedback → re-appears in ready queue
6. WorkflowState persistence: save → reload → verify all items and history preserved
7. Stats accuracy: mixed statuses → getWorkflowState returns correct counts
8. Combined: skeleton with 5 functions, mixed priorities and dependencies,
   complete in correct order, verify final phase = Complete

---

## Phase 12b: Routing Engine + Workers (Steps 326-331)

*The brain that reads annotations and decides who handles what.*

### Step 326: RoutingEngine.h — Annotation-to-Dispatch Logic (12 tests)
**Goal:** Given a WorkItem with routing annotations, produce a RoutingDecision
that says what kind of worker should handle it and how much context to provide.

**New file: `editor/src/RoutingEngine.h`** (~350 lines)

**RoutingDecision struct:**
```
RoutingDecision:
  workerType: string            // "deterministic" | "template" | "slm" | "llm" | "human"
  contextWidth: string          // "local" | "file" | "project" | "cross-project"
  contextBudgetTokens: int      // estimated tokens for context window
  reviewRequired: bool          // whether result needs human review
  confidence: float             // engine's confidence in this routing (0.0-1.0)
  reasoning: string             // human-readable explanation of the decision
  agentRole: string             // "linter" | "refactor" | "generator"
```

**RoutingEngine class:**
- `route(WorkItem) -> RoutingDecision` — main entry point
- `routeBatch(vector<WorkItem>) -> vector<RoutingDecision>` — batch routing

**Routing rules (priority order):**
1. **Explicit annotation override** — if @Automatability is set on the node,
   use it directly (human architect has spoken)
2. **Ambiguity gate** — @Ambiguity(high) → human, regardless of other signals
3. **Review gate** — @Review(required, human) → sets reviewRequired=true
4. **Complexity routing:**
   - @Complexity(cognitive<=2) + @ContextWidth(local) → deterministic
   - @Complexity(cognitive<=4) + @ContextWidth(local|file) → slm
   - @Complexity(cognitive<=7) → llm
   - @Complexity(cognitive>7) → llm + reviewRequired
5. **Context width escalation:**
   - cross-project → llm (needs large context window)
   - project → llm or slm depending on complexity
   - file → slm unless complex
   - local → deterministic or template unless complex
6. **Pattern defaults** (if no annotations):
   - Skeleton with no body → template if simple signature, slm otherwise
   - Function with existing body (modification) → llm
   - Getter/setter/accessor patterns → deterministic

**Context budget estimation:**
- local: ~500 tokens (node + immediate siblings)
- file: ~2000 tokens (buffer content, budget-truncated)
- project: ~8000 tokens (all buffers, compact AST summaries)
- cross-project: ~16000 tokens (project + external references)
- Uses `estimateContextTokens()` from Sprint 11e AnnotationInference

**Tests:** explicit @Automatability overrides inferred routing, high ambiguity →
human, low complexity + local → deterministic, high complexity → llm, cross-project
→ llm, review annotation sets reviewRequired, batch routing, no-annotation defaults,
getter pattern → deterministic, confidence reflects annotation coverage, context
budget reasonable for each width.

### Step 327: WorkerRegistry.h — Worker Abstractions (12 tests)
**Goal:** Define the worker interface and provide concrete implementations for
deterministic and pass-through workers. LLM/SLM workers are stubs that prepare
context — actual model invocation happens externally via MCP.

**New file: `editor/src/WorkerRegistry.h`** (~400 lines)

**WorkerInterface (abstract):**
```
class WorkerInterface:
  type() -> string                    // "deterministic" | "template" | "slm" | "llm" | "human"
  canHandle(WorkItem) -> bool         // whether this worker accepts the item
  execute(WorkItem, context) -> WorkItemResult  // produce result
```

**WorkerContext struct:**
```
WorkerContext:
  nodeAst: json                 // the target node's AST (always present)
  bufferContent: string         // current buffer text (file+ context)
  projectSummary: json          // compact AST summaries of all buffers (project+ context)
  annotations: vector<json>     // all annotations on the target node
  diagnostics: vector<json>     // relevant diagnostics
  skeletonIntent: string        // @Intent annotation value if present
  feedbackFromRejection: string // if this is a re-routed rejected item
```

**DeterministicWorker** — uses Pipeline for code generation:
- Handles: simple skeleton functions with clear signatures and @Intent
- Execute: parses intent into AST manipulations, runs Pipeline.generate()
- Confidence: 0.9+ for pattern-matched templates, 0.5 for heuristic fills

**TemplateWorker** — uses language-specific templates:
- Handles: getters, setters, constructors, simple CRUD patterns
- Execute: pattern-matches skeleton signature → fills template
- Confidence: 0.95 for exact pattern match

**AgentWorker** — prepares context for external LLM/SLM invocation:
- Handles: anything too complex for deterministic/template
- Execute: assembles WorkerContext at appropriate context width,
  returns a "prepared" result with context bundle (the actual model
  invocation happens externally through MCP)
- Subclasses: `SLMAgentWorker` (narrow context), `LLMAgentWorker` (wide context)
- Confidence: 0.0 (deferred to external model)

**HumanWorker** — marks item for human review:
- Handles: items routed to human
- Execute: returns result with status="needs-human", includes context bundle
  and reasoning for why this was routed to human
- Confidence: 0.0 (deferred to human)

**WorkerRegistry class:**
- `registerWorker(WorkerInterface*) -> void`
- `getWorker(workerType) -> WorkerInterface*`
- `getDefaultRegistry() -> WorkerRegistry` — pre-populated with all 5 worker types

**Tests:** deterministic worker produces code for simple skeleton, template worker
fills getter pattern, agent worker prepares context bundle, human worker marks for
review, registry lookup, canHandle filtering, worker type strings, context assembly
at local/file/project widths, rejection feedback included in context, pipeline
integration for deterministic worker.

### Step 328: Context Assembly + Budget (12 tests)
**Goal:** Build the context window for each work item based on its @ContextWidth,
respecting token budgets and using the ResponseBudget system for truncation.

**New file: `editor/src/ContextAssembler.h`** (~250 lines)

**ContextAssembler class:**
- `assembleContext(workItem, bufferStates, contextWidth, budget) -> WorkerContext`
  — main entry point; builds context at the specified width within budget

**Assembly strategies by width:**
- **local:** target node AST + immediate sibling nodes + parent annotations
- **file:** target node + full buffer content (editBuf), truncated if over budget
- **project:** target node + compact AST summaries of all open buffers +
  import graph for the target file + cross-file symbols
- **cross-project:** project context + external dependency annotations
  (from sidecar files in workspace)

**Budget enforcement:**
- Estimate each context component's token cost (json.size() / 4)
- If total exceeds budget, prioritize: target node > annotations > diagnostics >
  siblings > buffer content > project summaries
- Use ResponseBudget::applyBudget() for array truncation
- Return actual tokens used in WorkerContext

**Integration with existing systems:**
- Uses `CompactAST::toJsonCompact()` for project-wide summaries
- Uses `ProjectState::ImportGraph` for cross-file references
- Uses `ResponseBudget::applyBudget()` for truncation
- Uses `AnnotationInference::estimateContextTokens()` for estimates

**Tests:** local context contains only target node, file context includes buffer
content, project context includes other buffer summaries, budget truncation
activates when over limit, priority ordering preserved under truncation, import
graph included in project context, cross-file symbols included, empty project
returns minimal context, token estimate within 20% of actual, budget=0 means
unlimited.

### Step 329: Routing RPC + MCP Tools (12 tests)
**Goal:** Expose routing and worker dispatch through RPC and MCP tools.

**Modifies: `editor/src/HeadlessEditorState.h`**
- Add `RoutingEngine` and `WorkerRegistry` members
- Initialize default routing rules and worker registry

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`** — 4 new RPC methods:
- `routeTask` — route a single work item, return RoutingDecision
  - params: itemId
  - returns: { decision: { workerType, contextWidth, budget, reasoning } }
- `routeAllReady` — route all ready items in the workflow
  - returns: { routed: int, decisions: [...] }
- `executeTask` — assign worker and execute (deterministic/template only;
  agent/human items are prepared but not executed)
  - params: itemId
  - returns: { result, workerType, autoApproved }
- `getRoutingExplanation` — explain why a specific item was routed a certain way
  - params: itemId
  - returns: { reasoning, annotationsUsed, rulesApplied }

**Modifies: `editor/src/AgentPermissionPolicy.h`**
- `getRoutingExplanation` — read-only
- `routeTask` / `routeAllReady` / `executeTask` — mutation (Refactor/Generator)

**Modifies: `editor/src/MCPServer.h`** — registerRoutingTools()
- `whetstone_route_task` — route single item
- `whetstone_route_all_ready` — batch route
- `whetstone_execute_task` — execute deterministic/template tasks
- `whetstone_get_routing_explanation` — explain routing decision

**Tool count:** 54+ (50 + 4 routing tools)

**Tests:** routeTask returns valid decision, routeAllReady batch processes queue,
executeTask runs deterministic worker and produces code, agent items marked as
prepared not executed, human items marked for review, routing explanation includes
annotation references, Linter role can read explanation but not route, MCP tool
registration (4 new tools).

### Step 330: Review Gates (12 tests)
**Goal:** Configurable rules for auto-approving low-risk results and surfacing
high-risk results for human review.

**New file: `editor/src/ReviewGate.h`** (~200 lines)

**ReviewPolicy struct:**
```
ReviewPolicy:
  autoApproveRules: vector<AutoApproveRule>
  defaultAction: string   // "require-review" | "auto-approve"
```

**AutoApproveRule struct:**
```
AutoApproveRule:
  workerType: string         // match worker type ("deterministic", "*")
  maxComplexity: int         // max cognitive complexity to auto-approve
  minConfidence: float       // minimum worker confidence to auto-approve
  riskLevel: string          // max risk level ("none", "low")
```

**ReviewGate class:**
- `shouldAutoApprove(workItem, result, policy) -> ReviewDecision`
- `ReviewDecision`: approved (bool), reasoning (string), rule matched or why not
- Default policy: auto-approve deterministic + confidence>=0.9 + risk<=low;
  everything else requires review

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`**
- `setReviewPolicy` — configure auto-approve rules
- `getReviewPolicy` — inspect current review policy

**Modifies: `editor/src/MCPServer.h`**
- `whetstone_set_review_policy` + `whetstone_get_review_policy`

**Tool count:** 56+ (54 + 2 review tools)

**Tests:** deterministic + high confidence → auto-approve, llm result → require
review, low confidence → require review even if deterministic, high risk overrides
auto-approve, custom policy respected, default policy behavior, explicit
@Review(required) overrides auto-approve, rejection with feedback, policy
serialization roundtrip, empty policy uses defaults, MCP tool registration.

### Step 331: Phase 12b Integration Tests (8 tests)
**Goal:** Full routing pipeline from skeleton through execution and review.

**Tests:**
1. Create skeleton → populate workflow → route all → verify worker type assignments
2. Simple getter skeleton → route → deterministic → execute → auto-approve → complete
3. Complex function skeleton → route → llm → prepare context → verify context width
4. High ambiguity skeleton → route → human → verify marked for review
5. Dependency chain with mixed routing: A(deterministic)→B(llm)→C(human) in order
6. Rejection flow: execute → reject → re-route with feedback in context
7. Review policy: set strict policy → nothing auto-approved → set permissive → deterministic auto-approved
8. Full lifecycle: 5-function skeleton with mixed complexity → workflow → route → execute deterministic ones → verify stats

---

## Phase 12c: C++ AST Depth — Inheritance + CRTP (Steps 332-336)

*Multiple inheritance, virtual inheritance, and CRTP — Whetstone's own patterns.*

### Step 332: Multiple Inheritance in ClassDeclaration (12 tests)
**Goal:** Upgrade ClassDeclaration from single `superClass` string to a vector
of base classes with access specifiers and virtual flags.

**Modifies: `editor/src/ast/ClassDeclaration.h`**
- New struct `BaseClass { name, accessSpecifier, isVirtual }`
  - accessSpecifier: "public" | "protected" | "private" (default: "private" for C++)
  - isVirtual: bool (default: false)
- `ClassDeclaration` gains: `vector<BaseClass> baseClasses`
- Backward compat: existing `superClass` string still works (converted to
  single BaseClass with public access), deprecated but supported
- Helper: `addBase(name, access, isVirtual)`, `getBases() -> vector<BaseClass>`
- Helper: `hasDiamondInheritance(classMap) -> bool` (detects shared bases)

**Modifies: `editor/src/ast/Serialization.h`**
- `propertiesToJson` — serialize baseClasses array
- `setPropertiesFromJson` — deserialize baseClasses, handle legacy superClass
- `createNode` — no change (ClassDeclaration factory exists from Sprint 11c)

**Tests:** single base (backward compat), multiple bases, access specifiers,
virtual inheritance flag, BaseClass JSON roundtrip, mixed virtual and non-virtual,
diamond detection (A→B, A→C, D→B+C), empty bases list, legacy superClass
migration, addBase helper.

### Step 333: Template Class Declarations (12 tests)
**Goal:** Represent `template<typename T> class Foo : public Bar<T> { ... }` —
template parameters on classes (not just functions), including CRTP patterns.

**Modifies: `editor/src/ast/GenericType.h`**
- `GenericType` gains: `isClassTemplate: bool` (vs function template)
- When attached to a ClassDeclaration, represents `template<...> class`
- TypeParameter gains: `isVariadic: bool` (for `typename... Args`)

**New recognition helpers in `editor/src/ast/GenericType.h`:**
- `isCRTP(classDecl) -> bool` — detects when a class inherits from a base
  parameterized by itself: `class Foo : public Base<Foo>`
- `getTemplateParameters(classDecl) -> vector<TypeParameter>` — extracts
  template params from attached GenericType children

**Modifies: `editor/src/ast/Serialization.h`**
- GenericType: serialize `isClassTemplate`
- TypeParameter: serialize `isVariadic`

**Modifies: `editor/src/CompactAST.h`**
- `getNodeName` for GenericType includes template indicator

**Tests:** class template construction, CRTP detection (class Foo : Base<Foo>),
non-CRTP base parameterized by other type, variadic type parameter, JSON roundtrip
for isClassTemplate, template class with multiple type params, GenericType
attached to ClassDeclaration, compact AST name, isCRTP with diamond inheritance,
template + multiple inheritance combined.

### Step 334: C++ Parser — Inheritance + Templates (12 tests)
**Goal:** Deepen CppParser to handle multiple inheritance, virtual inheritance,
access specifiers, template class declarations, and CRTP patterns.

**Modifies: `editor/src/ast/CppParser.h`**
- Parse `class Foo : public Bar, private virtual Baz { ... }`
  → ClassDeclaration with baseClasses vector
- Parse `template<typename T> class Foo { ... }`
  → ClassDeclaration with GenericType child (isClassTemplate=true)
- Parse `template<typename T, typename... Args>`
  → TypeParameters including variadic
- Parse `class Foo : public Base<Foo>` → CRTP detection
- Parse `struct` same as `class` but default public access

**Backward compatibility:** existing class parsing (single base) continues
to work, now populates baseClasses[0] instead of superClass.

**Tests:** single inheritance (backward compat), multiple inheritance parsed,
access specifiers detected, virtual inheritance flag, template class parsed,
CRTP pattern detected, struct vs class default access, nested class (inner class),
template with variadic, combined template + multiple inheritance, existing
function-level tests still pass, real Whetstone-style class signature.

### Step 335: C++ Generator — Inheritance + Templates (12 tests)
**Goal:** CppGenerator outputs correct C++ syntax for multiple inheritance,
virtual inheritance, access specifiers, and template classes.

**Modifies: `editor/src/ast/CppGenerator.h`**
- Generate `class Foo : public Bar, private virtual Baz { ... }`
- Generate `template<typename T> class Foo { ... }`
- Generate `template<typename T, typename... Args>` for variadic
- Access sections: `public:`, `protected:`, `private:` grouping for methods

**Cross-language considerations:**
- Other generators handle multiple inheritance differently:
  - Java/Kotlin/C# → single extends + multiple implements (interfaces)
  - Python → multiple inheritance directly
  - Rust → no inheritance, use traits
  - Go → embedding
- Add `adaptInheritance(classDecl, targetLanguage) -> ClassDeclaration` helper
  in CrossLanguageProjector that converts multiple inheritance to
  target-appropriate pattern

**Modifies: `editor/src/CrossLanguageProjector.h`**
- `adaptInheritance()` — converts C++ multiple inheritance to target patterns

**Tests:** C++ multiple inheritance output, access specifier output, virtual
keyword present, template class output, CRTP output, cross-language: C++ → Java
(multiple bases → extends + implements), C++ → Python (direct), C++ → Rust
(traits), cross-language: Java → C++ (implements → base classes), roundtrip:
parse C++ → generate C++, combined template + inheritance output.

### Step 336: Phase 12c Integration Tests (8 tests)
**Goal:** Full C++ inheritance + template roundtrip and cross-language projection.

**Tests:**
1. Parse Whetstone-style class: `class KotlinGenerator : public ProjectionGenerator, public virtual AnnotationVisitorExtended, public SemannoAnnotationImpl<KotlinGenerator>` → verify AST structure
2. CRTP detection: `SemannoAnnotationImpl<KotlinGenerator>` recognized as CRTP
3. Round-trip: parse complex C++ class → generate → parse again → AST equivalent
4. Cross-language: C++ class with multiple inheritance → Java output (extends + implements)
5. Cross-language: C++ class with multiple inheritance → Python output (multiple bases)
6. Template class with 3 type params → parse → generate → verify
7. Diamond inheritance detection: parse class hierarchy → hasDiamondInheritance returns true
8. Mixed: class with templates, multiple inheritance, virtual methods, annotations → full pipeline

---

## Phase 12d: C++ AST Depth — Preprocessor, Enums, Namespaces (Steps 337-341)

*The remaining C++ constructs needed for self-hosting progress.*

### Step 337: Preprocessor AST Nodes (12 tests)
**Goal:** Represent `#include`, `#pragma`, and `#define` as first-class AST nodes.

**New file: `editor/src/ast/PreprocessorNodes.h`** (~150 lines)

**IncludeDirective** (Statement-level):
- `path: string` — the include path ("ast/ASTNode.h" or <string>)
- `isSystem: bool` — angle brackets (true) vs quotes (false)

**PragmaDirective** (Statement-level):
- `directive: string` — the pragma content ("once", "pack(push, 1)", etc.)

**MacroDefinition** (Statement-level):
- `name: string` — macro name
- `parameters: vector<string>` — macro params (empty for object-like macros)
- `body: string` — macro body text (unparsed — macros are fundamentally textual)
- `isFunctionLike: bool` — has parameter list

**Design note:** Preprocessor directives are statement-level nodes added to Module's
children. They are *not* deeply parsed into sub-AST (macro bodies stay as text).
This matches the reality that preprocessor operates on text, not AST. For annotation
purposes, `@Synthetic(macro-expanded)` can mark nodes that result from macro expansion.

**Modifies: `editor/src/ast/Serialization.h`**
- propertiesToJson/createNode/setPropertiesFromJson for all 3 types

**Modifies: `editor/src/CompactAST.h`**
- getNodeName for IncludeDirective, PragmaDirective, MacroDefinition

**Tests:** IncludeDirective construction + roundtrip, system vs local flag,
PragmaDirective construction + roundtrip, MacroDefinition with params,
function-like vs object-like distinction, compact AST names, module with
mixed preprocessor + function nodes, empty macro body, Serialization createNode
dispatch.

### Step 338: EnumDeclaration + NamespaceDeclaration (12 tests)
**Goal:** Scoped enums and namespaces as first-class AST nodes.

**New file: `editor/src/ast/EnumNamespaceNodes.h`** (~150 lines)

**EnumDeclaration** (Statement-level, has children):
- `name: string`
- `isScoped: bool` — `enum class` (true) vs plain `enum` (false)
- `underlyingType: string` — optional (e.g., "int", "uint8_t")
- Children role "members": vector of EnumMember nodes

**EnumMember** (leaf node):
- `name: string`
- `value: string` — optional explicit value ("1", "0xFF")

**NamespaceDeclaration** (Statement-level, has children):
- `name: string` — namespace name (empty for anonymous)
- Children role "body": vector of statements/declarations inside the namespace

**TypeAlias** (Statement-level):
- `aliasName: string` — the new name
- `targetType: string` — what it aliases
- `isUsing: bool` — `using` (true) vs `typedef` (false)

**Modifies: `editor/src/ast/Serialization.h`**
- propertiesToJson/createNode/setPropertiesFromJson for all 4 types

**Modifies: `editor/src/CompactAST.h`**
- getNodeName for EnumDeclaration, EnumMember, NamespaceDeclaration, TypeAlias

**Tests:** EnumDeclaration construction, scoped vs unscoped, EnumMember with explicit
value, enum with 4 members as children, NamespaceDeclaration with body children,
TypeAlias using vs typedef, JSON roundtrip for all 4 types, compact AST names,
nested namespace (namespace inside namespace), enum inside namespace.

### Step 339: C++ Parser — Preprocessor, Enum, Namespace (12 tests)
**Goal:** Parse these constructs from C++ source code.

**Modifies: `editor/src/ast/CppParser.h`**
- Parse `#include "path"` and `#include <path>` → IncludeDirective
- Parse `#pragma once` and `#pragma ...` → PragmaDirective
- Parse `#define NAME(args) body` → MacroDefinition
- Parse `enum class Name : Type { A = 1, B = 2 }` → EnumDeclaration + EnumMembers
- Parse `enum Name { ... }` → EnumDeclaration (isScoped=false)
- Parse `namespace Name { ... }` → NamespaceDeclaration with body
- Parse `using Name = Type;` → TypeAlias (isUsing=true)
- Parse `typedef Type Name;` → TypeAlias (isUsing=false)

**Tests:** #include system/local parsed, #pragma once, #define object-like,
#define function-like, enum class with values, plain enum, namespace with
contents, using alias, typedef, mixed file with preprocessor + namespace +
enum + functions, backward compat (existing C++ tests pass), real Whetstone
header snippet.

### Step 340: C++ Generator — Preprocessor, Enum, Namespace (12 tests)
**Goal:** CppGenerator produces correct output for all new node types.

**Modifies: `editor/src/ast/CppGenerator.h`**
- Generate `#include "path"` / `#include <path>` with correct brackets
- Generate `#pragma once`
- Generate `#define NAME(args) body`
- Generate `enum class Name : Type { A = 1, B };`
- Generate `namespace Name { ... }`
- Generate `using Name = Type;`

**Cross-language considerations:**
- Python: no preprocessor, no enums (use class), no namespaces (use modules)
- Java: import instead of include, enum keyword, package instead of namespace
- Rust: use instead of include, enum, mod instead of namespace
- Add basic adaptation in generators for enum/namespace equivalents

**Tests:** #include output, #pragma output, #define output, enum class output,
namespace output, using alias output, round-trip parse → generate → verify,
cross-language: C++ enum → Java enum, C++ namespace → Java package (comment),
C++ enum → Python class, C++ enum → Rust enum, mixed file generation.

### Step 341: Phase 12d Integration + Sprint 12 Summary (8 tests)
**Goal:** Full C++ depth validation and sprint verification.

**Tests:**
1. Parse realistic C++ header: includes + pragma + namespace + enum + class + functions → full AST
2. Round-trip: complex C++ header → AST → generate → parse again → equivalent
3. Enum inside namespace: parse → verify scoping in AST
4. Cross-language: C++ header → Java output with imports + package + enums adapted
5. Combined workflow + C++: create skeleton with C++ class patterns → workflow → route
6. All new AST nodes serialize/deserialize correctly (batch verification)
7. Sprint 12 totals: 56+ MCP tools, all new C++ constructs, workflow engine operational
8. C++ self-hosting progress: parse simplified Whetstone header fragment → verify structure

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 12a | 320-325 | 68 | WorkItem model, task queue, workflow state, persistence, RPC/MCP |
| 12b | 326-331 | 68 | Routing engine, workers, context assembly, review gates, RPC/MCP |
| 12c | 332-336 | 56 | C++ multiple inheritance, CRTP, template classes, cross-language |
| 12d | 337-341 | 56 | C++ preprocessor, enum, namespace, type alias, generators |
| **Total** | **320-341** | **~248** | 22 steps |

**MCP tool count projection:** 56+ tools (42 from Sprint 11 + 8 workflow + 4 routing + 2 review)

---

## Key Files to Modify

| File | Changes |
|------|---------|
| `editor/src/HeadlessEditorState.h` | Add WorkflowState, RoutingEngine, WorkerRegistry members |
| `editor/src/HeadlessAgentRPCHandler.h` | 12 new RPC methods (8 workflow + 4 routing) |
| `editor/src/AgentPermissionPolicy.h` | Permission mapping for 12 new methods |
| `editor/src/MCPServer.h` | registerWorkflowExecutionTools() + registerRoutingTools() + registerReviewTools() |
| `editor/src/ast/ClassDeclaration.h` | BaseClass struct, baseClasses vector, diamond detection |
| `editor/src/ast/GenericType.h` | isClassTemplate, isVariadic, isCRTP detection |
| `editor/src/ast/CppParser.h` | Inheritance, templates, preprocessor, enum, namespace parsing |
| `editor/src/ast/CppGenerator.h` | Multiple inheritance, templates, preprocessor, enum, namespace output |
| `editor/src/ast/Serialization.h` | 7+ new node types in all 3 dispatch points |
| `editor/src/CompactAST.h` | getNodeName for all new node types |
| `editor/src/CrossLanguageProjector.h` | adaptInheritance() for cross-language inheritance mapping |
| `editor/CMakeLists.txt` | Test targets for steps 320-341 |

**New files:**
- `editor/src/WorkItem.h` — WorkItem struct, WorkItemResult, lifecycle state machine
- `editor/src/TaskQueue.h` — Priority queue with dependency resolution
- `editor/src/WorkflowState.h` — Project-level workflow tracking, phase detection, stats
- `editor/src/WorkflowPersistence.h` — Workflow sidecar save/load
- `editor/src/RoutingEngine.h` — Annotation-to-dispatch routing logic
- `editor/src/WorkerRegistry.h` — Worker abstractions (deterministic, template, agent, human)
- `editor/src/ContextAssembler.h` — Context window assembly with budget enforcement
- `editor/src/ReviewGate.h` — Auto-approve rules and review policy
- `editor/src/ast/PreprocessorNodes.h` — IncludeDirective, PragmaDirective, MacroDefinition
- `editor/src/ast/EnumNamespaceNodes.h` — EnumDeclaration, EnumMember, NamespaceDeclaration, TypeAlias
- `editor/tests/step320_test.cpp` through `editor/tests/step341_test.cpp`

---

## Verification

After each phase:
1. Build all new test targets: `cmake --build . --target stepNNN_test`
2. Run each test: `./stepNNN_test` — expect "Results: 12/12" (or 8/8 for integration)
3. Phase integration test validates no regressions on earlier steps

Full sprint verification:
- All ~248 tests pass across steps 320-341
- Run full test suite (steps 245-341) to verify no regressions
- MCP `tools/list` returns 56+ tools
- Create skeleton → workflow → route → execute deterministic tasks → verify results
- Routing engine produces sensible decisions for varied annotation patterns
- C++ parser handles: multiple inheritance, CRTP, preprocessor, enum class, namespace
- Parse simplified Whetstone header → verify AST structure (self-hosting progress)
- Cross-language: C++ → Java/Python/Rust inheritance adaptation works
- All 10 languages parse and generate correctly (no regressions)

---

## Dependencies on Sprint 11

Sprint 12 assumes all Sprint 11 phases are complete:
- **11a** (done): Semanno format, annotation codegen
- **11b**: Validation rules (E0600-E1299) — used by routing engine to assess risk
- **11c**: 9 new AST nodes including ClassDeclaration — Sprint 12c extends it
- **11d**: Kotlin + C# — 10 languages total
- **11e**: SkeletonAST, routing annotations, inference engine — Sprint 12a/12b build directly on these
