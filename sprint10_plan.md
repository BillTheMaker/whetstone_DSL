# Sprint 10 Plan: Semantic Annotation Taxonomy & Environment Layer

## Context

Sprint 9 delivered a complete agent-first MCP tooling layer (34 tools, 244/244 tests, steps 245-265). The agent API surface is mature: headless mode, multi-file projects, diagnostics, search/rename, undo/redo, save, compact AST, batch queries.

Sprint 10 shifts focus to **what agents actually do with that API**: annotating code with rich semantic metadata from the full annotation taxonomy (docs/annotations/ Subjects 1-8) and modeling runtime environments. This is the core Whetstone value proposition — the semantic gap bridge between languages.

**What's already started:**
- Steps 266-270 TDD tests written (Phase 10a-10b)
- 5 semantic annotation AST classes in Annotation.h (Intent, Complexity, Risk, Contract, SemanticTag)
- SidecarPersistence.h with save/load logic
- Semantic annotation RPC methods in HeadlessAgentRPCHandler.h
- MCP tool registrations and prompt templates in MCPServer.h
- CompactAST.h semantic summary extraction

**What's deferred to Sprint 11:** LSP client, language-specific code actions, GUI overhaul, TUI mode, plugin system.

---

## Phase 10a: Semantic Annotation Core + Sidecar Persistence (Steps 266-268)

*TDD tests already written. Partial implementation exists.*

### Step 266: Semantic Annotation AST Node Types (12 tests)
**Goal:** 5 new AI-oriented annotation types with JSON roundtrip and compact AST support.

**Types:** IntentAnnotation, ComplexityAnnotation, RiskAnnotation, ContractAnnotation, SemanticTagAnnotation

**Implementation already present in:**
- `editor/src/ast/Annotation.h` (lines 145-184) — class definitions
- `editor/src/ast/Serialization.h` — toJson/fromJson dispatch
- `editor/src/CompactAST.h` — extractSemanticSummary()

**Work remaining:** Wire up any missing serialization, ensure all 12 tests pass.

### Step 267: Sidecar AST Persistence (12 tests)
**Goal:** Save/load annotated ASTs as `.whetstone/<filename>.ast.json` sidecar files.

**RPC methods:** saveAnnotatedAST, loadAnnotatedAST, listAnnotatedFiles

**Implementation already present in:**
- `editor/src/SidecarPersistence.h` — sidecarPath(), saveSidecarAST(), loadSidecarAST()

**Work remaining:** Wire RPC handlers in HeadlessAgentRPCHandler.h, register MCP tools (whetstone_save_annotated_ast, whetstone_load_annotated_ast, whetstone_list_annotated_files), permissions in AgentPermissionPolicy.h.

### Step 268: Phase 10a Integration Tests (8 tests)
**Goal:** End-to-end: annotate functions → compact AST shows semantic → save sidecar → reopen → load → verify.

**Tests cover:** multi-file sidecar, all 5 types in compact view, idempotent save/load, annotations don't pollute source output.

---

## Phase 10b: Annotation Agent Interface (Steps 269-271)

*Steps 269-270 TDD tests already written.*

### Step 269: Annotation RPC Methods (12 tests)
**Goal:** High-level set/get/remove/discover RPCs so agents don't need low-level applyMutation calls.

**RPC methods:**
- `setSemanticAnnotation(nodeId, type, fields)` — set or update
- `getSemanticAnnotations(nodeId?)` — query per-node or project-wide
- `removeSemanticAnnotation(nodeId, type)` — remove specific type
- `getUnannotatedNodes(type?, includeHints?)` — find gaps with static analysis hints

**MCP tools:** whetstone_set_semantic_annotation, whetstone_get_semantic_annotations, whetstone_remove_semantic_annotation, whetstone_get_unannotated_nodes

**Implementation partially present in HeadlessAgentRPCHandler.h and MCPServer.h.**

### Step 270: Annotation Prompt Templates (12 tests)
**Goal:** MCP prompt templates that guide frontier models through annotation workflow.

**Prompts:** annotate_intent, annotate_complexity, annotate_risk, annotate_contracts, annotate_full

**Each prompt mentions relevant tools** (setSemanticAnnotation, getUnannotatedNodes, save sidecar).

**getUnannotatedNodes hints:** bodyStatements count, sideEffectHint heuristic.

### Step 271: Phase 10b Integration Tests (8 tests)
**Goal:** Full agent annotation session: discover unannotated → set annotations → query → remove → re-query → save sidecar.

**Tests cover:** multi-function annotation workflow, prompt template content validation, permission enforcement across the pipeline, hint accuracy.

---

## Phase 10c: Type System, Execution & Scope Annotations (Steps 272-277)

*Implements annotation Subjects 2, 3, and 4 from docs/annotations/.*

### Step 272: Type System Annotations — Layout & Constraints (12 tests)
**Goal:** AST nodes for type metadata that drives cross-language projection.

**New annotation classes (from `Type System and Variance.md`):**
- `BitWidthAnnotation` — fields: width (int), e.g. 32 for Java int
- `EndianAnnotation` — fields: order ("big"|"little")
- `LayoutAnnotation` — fields: mode ("packed"|"aligned"), alignment (int)
- `NullabilityAnnotation` — fields: nullable (bool), strategy ("strict"|"nullable")
- `VarianceAnnotation` — fields: variance ("covariant"|"contravariant"|"invariant")

**Deliverables:** Class definitions, JSON roundtrip serialization, compact AST integration, 12 tests.

### Step 273: Type System Annotations — Identity & Mutability (12 tests)
**Goal:** Type identity and immutability depth annotations.

**New annotation classes:**
- `IdentityAnnotation` — fields: mode ("nominal"|"structural")
- `MutAnnotation` — fields: depth ("shallow"|"deep"|"interior")
- `TypeStateAnnotation` — fields: state ("erased"|"reified")

**Plus:** C++ generator integration — MutAnnotation drives const qualification, IdentityAnnotation adds branding comments. 8 annotation types total for Subject 2.

### Step 274: Concurrency Annotations — Primitives & Memory Model (12 tests)
**Goal:** Execution model annotations for thread safety and hardware semantics.

**New annotation classes (from `Exec model and Concurrency.md`):**
- `AtomicAnnotation` — fields: consistency ("seq_cst"|"relaxed"|"acquire"|"release")
- `SyncAnnotation` — fields: primitive ("monitor"|"spin"|"semaphore")
- `ThreadModelAnnotation` — fields: model ("green"|"os"|"fiber")
- `MemoryBarrierAnnotation` — fields: (marker, no extra fields)

**C++ generator:** AtomicAnnotation → `std::atomic` with memory order, SyncAnnotation → `std::mutex`/`std::lock_guard` comments.

### Step 275: Async, Parallelism & Error Handling Annotations (12 tests)
**Goal:** Control flow and error model annotations.

**New annotation classes:**
- `ExecAnnotation` — fields: mode ("async"|"event"), runtimeHint (string)
- `BlockingAnnotation` — fields: kind ("io"|"compute")
- `ParallelAnnotation` — fields: kind ("data"|"task")
- `TrapAnnotation` — fields: signal (string)
- `ExceptionAnnotation` — fields: style ("checked"|"unchecked")
- `PanicAnnotation` — fields: behavior ("abort"|"unwind")

**C++ generator:** ExecAnnotation(async) → `// async: coroutine candidate`, ParallelAnnotation(data) → `// SIMD-safe` / `#pragma omp`.

### Step 276: Scope & Namespace Annotations (12 tests)
**Goal:** Symbol binding, closure capture, and visibility annotations.

**New annotation classes (from `Scope & Namespace resolution.md`):**
- `BindingAnnotation` — fields: time ("static"|"dynamic")
- `LookupAnnotation` — fields: mode ("lexical"|"hoisted")
- `CaptureAnnotation` — fields: strategy ("value"|"ref"|"move")
- `VisibilityAnnotation` — fields: level ("private"|"internal"|"friend"|"public")
- `NamespaceAnnotation` — fields: style ("qualified"|"flat")
- `ScopeAnnotation` — fields: kind ("local"|"global_leaked"|"singleton")

**C++ generator:** CaptureAnnotation → lambda capture list comment, VisibilityAnnotation → access specifier hint.

### Step 277: Phase 10c Integration Tests (8 tests)
**Goal:** Cross-subject annotation workflows — annotate a module with type, concurrency, and scope metadata, verify compact AST, sidecar roundtrip, and C++ generator output reflects annotations.

**Tests cover:**
1. Type annotations drive C++ const/alignment output
2. Concurrency annotations produce thread-safety comments
3. Scope annotations influence lambda capture hints
4. All Subject 2-4 annotations survive sidecar save/load
5. Compact AST includes type/exec/scope summaries
6. Mixed annotations on single function (type + concurrency + scope)
7. getSemanticAnnotations returns Subject 2-4 types
8. setSemanticAnnotation works for new types via RPC

---

## Phase 10d: Shims, Optimization, Meta-Programming & Policy Annotations (Steps 278-283)

*Implements annotation Subjects 5, 6 (gaps), 7, and 8 from docs/annotations/.*

### Step 278: Shim & Escape Hatch Annotations (12 tests)
**Goal:** Preserve platform-specific and inexpressible constructs.

**New annotation classes (from `Shims and Escape Hatches.md`):**
- `IntrinsicAnnotation` — fields: instruction (string), arch (string)
- `RawAnnotation` — fields: language (string), code (string)
- `CallingConvAnnotation` — fields: convention ("stdcall"|"cdecl"|"fastcall")
- `LinkAnnotation` — fields: symbolName (string), library (string)
- `ShimAnnotation` — fields: strategy ("vtable"|"trampoline"|"union_tag"|"cast")
- `PointerArithmeticAnnotation` — fields: (marker)
- `OpaqueAnnotation` — fields: reason (string)

### Step 279: Platform & Provenance Annotations (12 tests)
**Goal:** Conditional compilation and transformation history tracking.

**New annotation classes:**
- `TargetAnnotation` — fields: platform (string), arch (string)
- `FeatureAnnotation` — fields: flag (string), enabled (bool)
- `OriginalAnnotation` — fields: sourceCode (string), sourceLanguage (string)
- `MappingAnnotation` — fields: history (vector<string>), transformations applied

**C++ generator:** TargetAnnotation → `#ifdef` guards, OriginalAnnotation → preserved-source comment block.

### Step 280: Optimization Annotation Completion (12 tests)
**Goal:** Fill gaps in Subject 6 — loop hints, data locality, hardware mapping, safety overrides.

**New annotation classes (from `6 optimization and intent.md`):**
- `TailCallAnnotation` — fields: (marker, for TCO eligibility)
- `LoopAnnotation` — fields: hint ("unroll"|"vectorize"|"fuse"), factor (int, optional)
- `DataAnnotation` — fields: hint ("prefetch"|"restrict")
- `AlignAnnotation` — fields: bytes (int)
- `PackAnnotation` — fields: (marker)
- `BoundsCheckAnnotation` — fields: enabled (bool)
- `OverflowAnnotation` — fields: behavior ("wrap"|"saturation"|"panic")

**C++ generator:** LoopAnnotation(vectorize) → `#pragma omp simd`, AlignAnnotation → `alignas(N)`, PackAnnotation → `#pragma pack(push, 1)`, OverflowAnnotation → wrapping arithmetic comment.

### Step 281: Meta-Programming Annotations (12 tests)
**Goal:** Code-as-data, macro expansion tracking, reflection policies.

**New annotation classes (from `Meta-Programming & Homoiconicity.md`):**
- `MetaAnnotation` — fields: state ("quoted"|"unquoted"), phase ("compile"|"runtime")
- `SymbolAnnotation` — fields: mode ("gensym"|"interned")
- `EvaluateAnnotation` — fields: phase ("compile_time"|"runtime")
- `TemplateAnnotation` — fields: specialization ("trait"|"monomorphize"|"erasure")
- `SyntheticAnnotation` — fields: generator (string), isStructuralRisk (bool)

### Step 282: Strategy & Policy Annotations (12 tests)
**Goal:** Decision support — multiple valid transformations, guardrails, conflict markers.

**New annotation classes (from `8 strategy choice and policy.md`):**
- `PolicyAnnotation` — fields: strictness ("high"|"low"), perf ("critical"|"normal"), style ("idiomatic"|"literal"), binaryStable (bool)
- `AmbiguityAnnotation` — fields: intent (string), options (vector<string>)
- `CandidateAnnotation` — fields: inferredTypes (vector<string>)
- `TradeoffAnnotation` — fields: reason (string), safetyCost (string), perfCost (string)
- `ChoiceAnnotation` — fields: choiceId (string), options (vector<string>)
- `DecisionAnnotation` — fields: choiceId (string), selection (string), author (string), reason (string)

### Step 283: Phase 10d Integration Tests (8 tests)
**Goal:** Full Subject 5-8 annotation workflow with sidecar persistence and generator output.

**Tests cover:**
1. Shim annotations on FFI boundary function
2. OriginalAnnotation preserves legacy source through projection
3. Optimization annotations drive C++ pragma output
4. Meta-programming annotations mark macro-expanded code
5. Policy + Choice + Decision annotation workflow (strategy menu)
6. All Subject 5-8 annotations survive sidecar roundtrip
7. setSemanticAnnotation RPC works for all new types
8. Annotation taxonomy completeness check: all 8 subjects represented

---

## Phase 10e: Environment Layer (Steps 284-289)

*Implements the Environment Layer feature request from `docs/annotations/Environment Layer.md`.*

### Step 284: EnvironmentSpec Schema & AST Node (12 tests)
**Goal:** First-class EnvironmentSpec attached to modules.

**New class: `EnvironmentSpec` (extends ASTNode)**
Fields:
- `envId` (string) — "posix_process", "browser", "jvm", "python_cpython", "emacs_runtime"
- `envVersion` (string, optional)
- `capabilities` (vector<string>) — required capabilities
- `constraints` (vector<string>) — forbidden behaviors
- `scheduler` (string) — "event_loop"|"threads"|"fibers"|"coroutines"|"single_thread"
- `memory` (string) — "manual"|"raii"|"refcount"|"tracing_gc"|"region"
- `bindingTimes` (json object) — per-feature: "compile"|"link"|"load"|"runtime"
- `exceptions` (string) — "unwind"|"checked"|"typed"|"untyped"
- `ffi` (string) — "c_abi"|"dynamic_linking"|"none"

**Deliverables:** Class definition, JSON roundtrip, attachment to Module via `setEnvironment`/`getEnvironment` RPC, compact AST env summary.

### Step 285: Capability Vocabulary & Validation (12 tests)
**Goal:** Fixed capability vocabulary with validation and per-node capability requirements.

**Capability vocabulary (initial set):**
`io.fs`, `io.net`, `io.stdinout`, `clock.time`, `timers`, `event_loop`, `threads`, `process`, `signals`, `modules`, `dynamic_loading`, `reflection`, `gc`, `jit`, `ui.dom`, `editor.emacs`, `host.imgui`

**New class: `CapabilityRequirement` (extends Annotation)**
- fields: capability (string), required (bool)
- Attaches to any node that requires a specific env capability

**Validation:** `validateCapabilities(module)` checks all CapabilityRequirements against module's EnvironmentSpec.capabilities, returns diagnostics for unmet requirements.

### Step 286: Environment-Aware Pipeline Hooks (12 tests)
**Goal:** Transforms receive EnvironmentSpec and adjust behavior.

**Changes to Pipeline:**
- `runPipeline` gains optional `env` parameter (or reads from module's EnvironmentSpec)
- `projectLanguage` checks env.scheduler and env.memory to choose lowering strategy
- Diagnostics flag env-incompatible annotations (e.g., `@Parallel(Data)` in single_thread env)

**New diagnostic codes:** E05xx for environment-related issues.

### Step 287: Environment-Aware Lowering Decisions (12 tests)
**Goal:** Concrete code generation changes driven by environment model.

**Lowering rules:**
- `env.scheduler=event_loop` + ExecAnnotation(async) → callback-based pattern in C++
- `env.scheduler=threads` + ExecAnnotation(async) → `std::async`/`std::future` in C++
- `env.memory=tracing_gc` → suppress explicit deallocation in C++ output
- `env.memory=manual` → inject explicit `delete` calls
- `env.exceptions=unwind` → `try/catch` in C++; `env.exceptions=typed` → `std::expected`

**Tests:** Same AST with different EnvironmentSpecs generates different C++ output.

### Step 288: Host Boundary AST Nodes (12 tests)
**Goal:** Explicit nodes for host-environment interactions.

**New AST node types:**
- `HostCall` (extends Expression) — fields: capability (string), name (string), maps to explicit env boundary
- `ScheduleTask` (extends Statement) — fields: queue ("microtask"|"macrotask"|"thread"), body (Statement children)
- `ModuleLoad` (extends Statement) — fields: moduleName (string), mode ("static"|"dynamic")

**Serialization, compact AST, and generator support for all 3 types.**

**C++ generator:** HostCall → capability-namespaced function call, ScheduleTask → thread/task dispatch, ModuleLoad → `#include` (static) or `dlopen` (dynamic).

### Step 289: Phase 10e Integration Tests (8 tests)
**Goal:** End-to-end environment-aware workflow.

**Tests cover:**
1. Set EnvironmentSpec on module, verify in compact AST
2. CapabilityRequirement diagnostic for missing capability
3. Same module + different env → different C++ output (threads vs event_loop)
4. HostCall generates correct capability-namespaced code
5. ScheduleTask + env.scheduler drives dispatch pattern
6. Environment survives sidecar save/load
7. Pipeline with env parameter applies correct lowering
8. Full workflow: set env → annotate → generate → verify output

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 10a | 266-268 | 32 | Semantic annotations + sidecar |
| 10b | 269-271 | 32 | Agent annotation interface |
| 10c | 272-277 | 80 | Type, execution, scope annotations (Subjects 2-4) |
| 10d | 278-283 | 80 | Shims, optimization, meta, policy (Subjects 5-8) |
| 10e | 284-289 | 80 | Environment layer |
| **Total** | **266-289** | **304** | |

**MCP tool count projection:** ~42 tools (34 current + 4 sidecar + 4 semantic RPC + environment tools)

---

## Key Files to Modify

| File | Changes |
|------|---------|
| `editor/src/ast/Annotation.h` | ~50 new annotation classes across Phases 10c-10e |
| `editor/src/ast/Serialization.h` | toJson/fromJson dispatch for all new types |
| `editor/src/CompactAST.h` | Semantic summary extraction for new annotation subjects |
| `editor/src/HeadlessAgentRPCHandler.h` | New RPC methods (sidecar, env, extended setSemanticAnnotation) |
| `editor/src/AgentPermissionPolicy.h` | Permissions for new methods |
| `editor/src/MCPServer.h` | Tool/prompt registrations |
| `editor/src/SidecarPersistence.h` | Already exists, may need extensions |
| `editor/src/ast/ProjectionGenerator.h` | Visit methods for generator-relevant annotations |
| `editor/src/ast/CppGeneratorTypes.h` | Annotation-driven C++ output |
| `editor/CMakeLists.txt` | Test targets for steps 266-289 |

**New files:**
- `editor/src/EnvironmentSpec.h` — EnvironmentSpec class, capability vocabulary, validation
- `editor/src/ast/HostBoundary.h` — HostCall, ScheduleTask, ModuleLoad AST nodes
- `editor/tests/step271_test.cpp` through `editor/tests/step289_test.cpp`

---

## Verification

After each phase:
1. Build: `cd editor/build-native && cmake .. && make -j$(nproc)`
2. Run step tests: `./step26X_test` for each step in the phase
3. Verify test counts match expected (12 per step, 8 for integration)
4. Check MCP tool count via `tools/list` in integration test

Full sprint verification:
- All 304 tests pass across steps 266-289
- Annotation taxonomy covers all 8 subjects from docs/annotations/
- Environment Layer meets acceptance criteria from Environment Layer.md
- Sidecar persistence works for all annotation types
- C++ generator output reflects annotation metadata

---

## Sprint 11 Preview (deferred from original Sprint 10)

1. LSP client for headless mode (clangd, pyright, rust-analyzer)
2. Language-specific code actions (Go imports, Rust borrow fixes)
3. GUI overhaul: pin floating windows, sane defaults, usable color scheme
4. Terminal UI mode (TUI) as alternative to ImGui GUI
5. Plugin system: load language support and tools dynamically
