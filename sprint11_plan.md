# Sprint 11 Plan: Dial It In

## Context

Sprint 10 completed the semantic annotation taxonomy (67+ types across 8 subjects) and environment layer, bringing the test count to 200/200 (steps 284-289). The agent API surface is mature: 38 MCP tools, headless mode, 10-phase annotation taxonomy, environment-aware code generation.

Sprint 11 closes all remaining gaps: 55 annotation types are silently dropped during code generation (only 12 have codegen visitors), only 5 types have validation rules, parsers don't handle classes/generics/async, and the training data infrastructure exists but only covers memory annotations. Sprint 11 closes all these gaps, adds Kotlin and C# as new languages, introduces the Semanno inline comment standard, and builds the machinery for exporting training data.

**What's already built:**
- 67+ annotation AST classes in Annotation.h (Subjects 1-8)
- 8 language parsers (Python, C++, Elisp, JavaScript, TypeScript, Java, Rust, Go)
- 8 language generators with ProjectionGenerator visitor dispatch
- 38 MCP tools, sidecar persistence, environment layer
- AnnotationValidator with 5 memory-only validation rules
- AnnotationConflict with ancestor-only same-type detection
- TransformEngine with constant folding + DCE
- MemoryStrategyInference for memory annotation inference
- TraceGenerator with 6 scenario types

**Deliverables:**
- **0 annotation types silently dropped** during codegen (currently 55 are dropped)
- **10 languages** (currently 8): + Kotlin, C#
- **9 new AST node types**: ClassDeclaration, InterfaceDeclaration, MethodDeclaration, GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation
- **All 10 parsers deepened** with class/async/lambda support
- **Semanno inline comment format**: `// @semanno:type(key=value)` — parseable, roundtrippable, language-aware
- **Validation rules for all annotation types** (E0600-E1299)
- **Cross-type conflict detection** (e.g., @Pure + @Blocking, @Atomic + @Sync)
- **Generalized annotation inference** (all 8 subjects, not just memory)
- **Training data export** to HuggingFace/OpenAI/HuggingFace formats
- **40+ MCP tools** (currently 38)

---

## Phase 11a: Semanno Format + Annotation Codegen (Steps 290-296)

*All 55 previously-dropped annotation types now generate Semanno inline comments in all generators.*

### Step 290: SemannoFormat.h — Comment Format Standard (12 tests)
**Goal:** Define and implement the `@semanno:type(key=value)` inline comment format as a parseable, roundtrippable standard.

**New file: `editor/src/SemannoFormat.h`** (~600 lines)
- `SemannoEmitter::emit(const ASTNode* anno) -> string` — emits `@semanno:type(key="value")` for all 67+ annotation types
- `SemannoParser::parse(string line) -> SemannoEntry{type, properties}` — parses any comment format (`//`, `#`, `;;`, etc.)
- `SemannoParser::isSemannoComment(line) -> bool` — quick detection
- Internal helpers: value escaping, vector serialization (`;`-separated)

**Tests:** emit/parse roundtrip for each annotation subject, property escaping, multi-property annotations.

### Step 291: ProjectionGenerator — Subject 2-4 Visitors (12 tests)
**Goal:** Add dispatch branches for type system, concurrency, and scope annotations.

**New file: `editor/src/ast/AnnotationVisitors.h`** (~150 lines)
- `AnnotationVisitorExtended` — pure virtual interface with 56 visitor methods for Subjects 2-8

**Modifies: `editor/src/ast/ProjectionGenerator.h`**
- Include `AnnotationVisitors.h`, inherit from `AnnotationVisitorExtended`
- Add 24 dispatch branches in `dispatchGenerate()` for Subject 2-4 types
- Default implementations return `""` for backward compatibility

**Tests:** dispatch recognizes all Subject 2-4 annotation types, returns non-empty for visitors with implementations.

### Step 292: ProjectionGenerator — Subject 5-8 + Semantic Visitors (12 tests)
**Goal:** Complete the dispatch table — no annotation type returns "Unknown".

**Modifies: `editor/src/ast/ProjectionGenerator.h`**
- Add 32 more dispatch branches for Subjects 5-8, semantic core, and CapabilityRequirement
- New AST node dispatch: ClassDeclaration, InterfaceDeclaration, MethodDeclaration, GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation

**Tests:** exhaustive dispatch test — AST with one of each 56 new types, none returns "Unknown".

### Step 293: Generator Implementations — Python/C++/Rust/Go (12 tests)
**Goal:** All 4 generators emit Semanno comments for every annotation type.

**New file: `editor/src/SemannoAnnotationImpl.h`** (~200 lines)
- CRTP mixin `SemannoAnnotationImpl<Derived>` — provides default implementations for all 56 visitor methods
- Each method calls `commentPrefix() + SemannoEmitter::emit(anno)`
- Derived generators only need to provide `commentPrefix()` (e.g., `"# "` for Python, `"// "` for C++)

**Modifies:** `PythonGenerator.h`, `CppGenerator.h`, `RustGenerator.h`, `GoGenerator.h`
- Each adds `public SemannoAnnotationImpl<XxxGenerator>` mixin
- Each adds `std::string commentPrefix() const` method

**Tests:** each generator emits Semanno comments for mixed old+new annotations.

### Step 294: Generator Implementations — Java/JS/Elisp (12 tests)
**Goal:** Remaining 3 generators get the same SemannoAnnotationImpl mixin.

**Modifies:** `JavaGenerator.h`, `JavaScriptGenerator.h`, `ElispGenerator.h`
- Same mixin pattern: inherit `SemannoAnnotationImpl`, add `commentPrefix()`
- Elisp uses `";; "` prefix

**Tests:** each generator emits Semanno comments, parse-back roundtrip works.

### Step 295: Semanno Sidecar Integration (12 tests)
**Goal:** Save/load `.semanno` sidecar files alongside `.ast.json` sidecars.

**New file: `editor/src/SemannoSidecar.h`** (~150 lines)
- Format: `L<line>: @semanno:type(key=value)` per line
- `saveSemannoSidecar(workspaceRoot, filePath, module)` — walks AST, emits annotations
- `loadSemannoSidecar(workspaceRoot, filePath)` — parses sidecar back to entries

**Modifies:** `SidecarPersistence.h` — import SemannoSidecar
**Modifies:** `HeadlessAgentRPCHandler.h` — `exportSemanno` RPC method

**Tests:** save/load roundtrip, alongside `.ast.json` sidecar, RPC method.

### Step 296: Phase 11a Integration Tests (8 tests)
**Goal:** Full pipeline verification — no annotation type is silently dropped.

**Tests cover:**
1. Parse → annotate with Subject 2-8 → generate → verify Semanno comments present
2. Cross-language: annotations preserved through projection
3. All 10 generators produce non-empty Semanno for every annotation type (exhaustive)
4. Semanno roundtrip: generate code → parse comments back → match original annotations
5. Mixed legacy + new annotations in single module
6. Semanno sidecar alongside AST sidecar
7. Generator comment prefix matches target language convention
8. Empty annotations (markers like @TailCall) produce correct Semanno

---

## Phase 11b: Validation & Conflict Completion (Steps 297-301)

*Extends validation from 5 to 67 types, adds cross-type conflict detection.*

### Step 297: AnnotationValidator — Subject 2-4 Rules (12 tests)
**Goal:** Validation rules for type system, concurrency, and scope annotations.

**New file: `editor/src/AnnotationValidatorExtended.h`** (~300 lines)
- `AnnotationValidatorExtended::validateExtended(root) -> vector<Diagnostic>`
- New diagnostic codes: E0600-E0699 (type system), E0700-E0799 (concurrency), E0800-E0899 (scope)

**Rules:**
- E0600: BitWidth must be power of 2 (1, 2, 4, 8, 16, 32, 64, 128)
- E0601: Endian order must be "big" or "little"
- E0602: Layout mode must be "packed" or "aligned"
- E0603: Nullability strategy must be "strict" or "nullable"
- E0700: Atomic consistency must be valid ordering (seq_cst, relaxed, acquire, release)
- E0701: Sync primitive must be "monitor", "spin", or "semaphore"
- E0704: ExecAnnotation(async) needs ThreadModel on module or ancestor
- E0800: Binding time must be "static" or "dynamic"
- E0804: Visibility level must be standard keyword

**Modifies:** `AnnotationValidator.h` — include extended, call new validators.

**Tests:** each rule with valid/invalid inputs, structured diagnostic code verification.

### Step 298: AnnotationValidator — Subject 5-8 Rules (12 tests)
**Goal:** Complete validation coverage for remaining 32 annotation types.

**Modifies: `AnnotationValidatorExtended.h`** — add rules for remaining types.
- E0900: CallingConv convention must be "stdcall", "cdecl", or "fastcall"
- E0901: Shim strategy must be valid (vtable, trampoline, union_tag, cast)
- E1000: LoopAnnotation hint must be "unroll", "vectorize", or "fuse"
- E1001: AlignAnnotation bytes must be power of 2
- E1100: Meta state must be "quoted" or "unquoted"
- E1200: Policy strictness must be "high" or "low"

**Tests:** each Subject 5-8 rule, no false positives on valid annotations.

### Step 299: Cross-Type Conflict Detection (12 tests)
**Goal:** Detect semantic conflicts between different annotation families on the same node.

**New file: `editor/src/AnnotationConflictExtended.h`** (~200 lines)
- `collectCrossTypeConflicts(node) -> vector<CrossTypeConflict>`

**Conflict pairs:**
1. `@Pure` + `@Blocking` — purity violated by blocking behavior
2. `@Atomic` + `@Sync` — conflicting synchronization models
3. `@Capture(move)` + `@Owner(Shared_ARC)` — move semantics incompatible with shared ref counting
4. `@ConstExpr` + `@Exec(async)` — compile-time eval can't be async
5. `@Inline(Always)` + `@TailCall` — inlining defeats tail call optimization
6. `@Pack` + `@Layout(aligned)` — packing and alignment contradict

**Modifies:** `AnnotationConflict.h` — include extended.

**Tests:** each conflict pair detected, no false positives, recursive detection.

### Step 300: TransformEngine Enhancements (12 tests)
**Goal:** New transforms that interact with the annotation taxonomy.

**New file: `editor/src/TransformEngineExtended.h`** (~250 lines)
- `floatConstantFolding()` — folds BinaryOperation on FloatLiterals
- `deadVariableElimination()` — removes unreferenced Variable declarations
- `annotationAwareOptimize()` — respects @OptimizationLock (skip), @BoundsCheck (preserve)

**Tests:** float folding, dead var removal, OptimizationLock blocks optimization, BoundsCheck preserved.

### Step 301: Phase 11b Integration Tests (8 tests)
**Goal:** Full pipeline with validation and conflict detection.

**Tests cover:**
1. Full pipeline with validation: conflicting annotations → correct E-codes
2. No regressions on original E01xx-E05xx diagnostics
3. Structured diagnostics include all new E06xx-E12xx codes
4. Cross-type conflicts reported with clear messages
5. Annotation-aware optimization respects locks
6. Combined validation + conflict + transform pipeline
7. All 67 annotation types have at least one validation rule
8. Quick-fix suggestions for common validation errors

---

## Phase 11c: Parser Deepening (Steps 302-308)

*9 new AST node types, all 8 existing parsers deepened with class/async/lambda support.*

### Step 302: New AST Nodes — Class/Interface/Generic (12 tests)
**Goal:** Structured types for OOP and generic programming.

**New file: `editor/src/ast/ClassDeclaration.h`** (~100 lines)
- `ClassDeclaration` — name, superClass, isAbstract; children: interfaces, fields, methods, annotations
- `InterfaceDeclaration` — name; children: methods, annotations
- `MethodDeclaration` (extends Function) — className, isStatic, visibility, isOverride, isVirtual

**New file: `editor/src/ast/GenericType.h`** (~60 lines)
- `GenericType` — baseName; children: typeParameters
- `TypeParameter` — name, constraint

**Tests:** construction, property access, child roles, serialization roundtrip.

### Step 303: New AST Nodes — Async/Lambda/Decorator (12 tests)
**Goal:** Modern language constructs as first-class AST nodes.

**New file: `editor/src/ast/AsyncNodes.h`** (~100 lines)
- `AsyncFunction` (extends Function) — isAsync flag
- `AwaitExpression` — children: expression
- `LambdaExpression` — captureList (vector); children: parameters, body
- `DecoratorAnnotation` — name; children: arguments

**Tests:** construction, serialization, compact AST names. Can run in parallel with Step 302.

### Step 304: Serialization + Dispatch for New Nodes (12 tests)
**Goal:** Full JSON roundtrip and generator dispatch for all 9 new node types.

**Modifies: `editor/src/ast/Serialization.h`**
- `propertiesToJson()` — 9 new branches for new node types
- `createNode()` — 9 new factory entries
- `setPropertiesFromJson()` — 9 new deserialization branches

**Modifies: `editor/src/ast/ProjectionGenerator.h`**
- Add visitor methods + `dispatchGenerate()` branches for all 9 new node types

**Tests:** JSON roundtrip for each type, dispatch returns non-empty, nested structures.

### Step 305: Python + Java Parser Deepening (12 tests)
**Goal:** Parse classes, async, lambdas from Python and Java source.

**Modifies: `editor/src/ast/PythonParser.h`**
- `class_definition` → ClassDeclaration
- `async def` → AsyncFunction
- `await` → AwaitExpression
- `lambda` → LambdaExpression
- `@decorator` → DecoratorAnnotation

**Modifies: `editor/src/ast/JavaParser.h`**
- `class_declaration` → ClassDeclaration
- `interface_declaration` → InterfaceDeclaration
- `type_parameters` → GenericType

**Tests:** backward compatibility (existing function-level tests pass), new constructs parsed.

### Step 306: TypeScript/JavaScript + Rust Parser Deepening (12 tests)
**Goal:** Parse classes, async/await, arrow functions, traits.

**Modifies: `editor/src/ast/JavaScriptParser.h`**
- class declarations, async/await, arrow functions → LambdaExpression

**Modifies: `editor/src/ast/RustParser.h`**
- struct → ClassDeclaration, impl → MethodDeclaration
- trait → InterfaceDeclaration, async fn → AsyncFunction, closures → LambdaExpression

**Tests:** each new construct parsed correctly, mixed with existing constructs.

### Step 307: Go + C++ + Elisp Parser Deepening (12 tests)
**Goal:** Complete parser deepening for remaining 3 languages.

**Modifies: `editor/src/ast/GoParser.h`**
- struct/interface types, method receivers, goroutines

**Modifies: `editor/src/ast/CppParser.h`**
- class/struct → ClassDeclaration, templates → GenericType, virtual methods → MethodDeclaration, lambdas → LambdaExpression

**Modifies: `editor/src/ast/ElispParser.h`**
- lambda forms → LambdaExpression

**Tests:** each language's new constructs parsed, no regressions.

### Step 308: Generator Updates + Phase 11c Integration (8 tests)
**Goal:** All 8 generators produce language-appropriate output for new AST node types.

**Modifies:** All 8 generators — add visitor implementations for ClassDeclaration, InterfaceDeclaration, MethodDeclaration, GenericType, TypeParameter, AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation.

**Tests:**
1. Parse class → generate → output has language-appropriate class syntax
2. Async roundtrip: parse async def → generate → contains async def
3. Generic type roundtrip
4. Lambda/closure roundtrip
5. Decorator roundtrip (Python → Python)
6. Class with methods and fields
7. Interface with abstract methods
8. Mixed: class + async + lambda in single module

---

## Phase 11d: New Languages — Kotlin + C# (Steps 309-313)

*Brings total from 8 to 10 language parsers and generators.*

### Step 309: Kotlin Parser (12 tests)
**Goal:** Parse Kotlin source into Whetstone AST.

**New file: `editor/src/ast/KotlinParser.h`** (~400 lines)
- Regex-based parser (no tree-sitter dependency)
- Parses: `fun`, `suspend fun` (→ AsyncFunction), `class`, `data class` (→ ClassDeclaration), `val`/`var` (→ Variable)
- `parseKotlin()` and `parseKotlinWithDiagnostics()` entry points

**Modifies:** `Parser.h` — include KotlinParser.h
**Modifies:** `Pipeline.h` — add `"kotlin"` routing in parse()

**Tests:** function parsing, class parsing, suspend function → AsyncFunction, data class, type inference.

### Step 310: Kotlin Generator (12 tests)
**Goal:** Generate idiomatic Kotlin from Whetstone AST.

**New file: `editor/src/ast/KotlinGenerator.h`** (~500 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<KotlinGenerator>`
- Kotlin syntax: `fun`, `val`, `listOf()`, `for..in`, `when`, nullable `?`
- Type mapping: Int, Double, String, Boolean, Unit, Array<>, Pair<>

**Modifies:** `Generator.h` — include KotlinGenerator.h
**Modifies:** `Pipeline.h` — add `"kotlin"` routing in generate()

**Tests:** function generation, class output, cross-language: Python → Kotlin, Kotlin → Java projection.

### Step 311: C# Parser (12 tests)
**Goal:** Parse C# source into Whetstone AST.

**New file: `editor/src/ast/CSharpParser.h`** (~450 lines)
- Regex-based parser (no tree-sitter dependency)
- Parses: methods (with visibility/return-type detection), `async` methods (→ AsyncFunction), `class`, `interface`
- Handles `using` directives, `namespace` wrappers

**Modifies:** `Parser.h` — include CSharpParser.h
**Modifies:** `Pipeline.h` — add `"csharp"` routing in parse()

**Tests:** method parsing, class parsing, interface parsing, async methods, generics.

### Step 312: C# Generator (12 tests)
**Goal:** Generate idiomatic C# from Whetstone AST.

**New file: `editor/src/ast/CSharpGenerator.h`** (~500 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<CSharpGenerator>`
- C# syntax: `public void`, Allman braces, `foreach..in`, semicolons
- Type mapping: int, float, double, string, bool, void, List<>, HashSet<>, Dictionary<>

**Modifies:** `Generator.h` — include CSharpGenerator.h
**Modifies:** `Pipeline.h` — add `"csharp"` routing in generate()

**Tests:** method generation, class output, cross-language: Java → C#, C# → TypeScript projection.

### Step 313: New Languages Integration + MCP (8 tests)
**Goal:** Full integration of Kotlin + C# across the pipeline.

**Modifies:**
- `MCPServer.h` — MCP describes 10 languages in tool descriptions
- `CrossLanguageProjector.h` — Kotlin + C# in adaptReclaimStrategy, generic annotation cloning
- `MemoryStrategyInference.h` — Kotlin defaults (GC/Tracing)
- `HeadlessAgentRPCHandler.h` — Kotlin + C# accepted in parse/generate routes

**Tests:**
1. MCP tools/list mentions Kotlin and C#
2. Cross-language projection: Python → Kotlin with annotation preservation
3. Cross-language projection: Java → C# with annotation preservation
4. Pipeline.run() with Kotlin source
5. Pipeline.run() with C# source
6. MemoryStrategyInference returns correct defaults for Kotlin
7. All 10 generators registered and produce output
8. Final language count verification: 10 parsers, 10 generators

---

## Phase 11e: Workflow Annotation Foundation (Steps 314-319)

*Generalized inference engine + routing annotation types + skeleton AST support.
Lays the groundwork for Sprint 12's workflow model.*

### Step 314: Generalized Annotation Inference Engine (12 tests)
**Goal:** Infer annotations across all 8 subjects, not just memory.

**New file: `editor/src/AnnotationInference.h`** (~400 lines)
- `AnnotationInference::inferAll(root) -> vector<InferredAnnotation>`
- `InferredAnnotation` struct: annotationType, targetNodeId, confidence, reasoning
- Delegates memory annotations to existing `MemoryStrategyInference` (backward compat)

**Pattern-based rules:**
- loops → @Loop
- async functions → @Exec(async)
- class methods → @Visibility
- recursive functions → @TailCall
- pure functions (no side effects) → @Pure
- try/catch patterns → @Exception
- goroutines/parallel patterns → @Parallel
- nested complexity → @Complexity

**Tests:** each pattern detected correctly, confidence scores reasonable, memory delegation works.

### Step 315: Routing Annotation Types (12 tests)
**Goal:** New annotation types that describe *how a task should be handled*, not just
what the code does. These are the vocabulary Sprint 12's workflow model will route on.

**Modifies: `editor/src/ast/Annotation.h`** — 6 new annotation classes (Subject 9: Workflow)
- `ContextWidthAnnotation` — width: "local" | "file" | "project" | "cross-project"
  (how much context is needed to implement or review this node)
- `ReviewAnnotation` — required: bool, reviewer: "human" | "agent" | "either",
  reason: string (whether this task needs human review and why)
- `AmbiguityAnnotation` — level: "none" | "low" | "medium" | "high",
  description: string (how ambiguous the requirements are)
- `AutomatabilityAnnotation` — strategy: "deterministic" | "template" | "slm" | "llm" | "human",
  confidence: float (what kind of worker should handle this)
- `PriorityAnnotation` — level: "critical" | "high" | "medium" | "low",
  blockedBy: vector<string> (task ordering hints)
- `ImplementationStatusAnnotation` — status: "skeleton" | "partial" | "complete" | "needs-review",
  assignee: string (tracks where a node is in the workflow)

**Modifies: `editor/src/ast/Serialization.h`** — propertiesToJson/createNode/setPropertiesFromJson
for all 6 new types
**Modifies: `editor/src/CompactAST.h`** — workflow annotations in extractSemanticSummary
**Modifies: `editor/src/SidecarPersistence.h`** — workflow annotations in isSemanticAnnotation

**Tests:** construction, property access, JSON roundtrip, compact AST summary, sidecar
recognition, validation of enum values.

### Step 316: Skeleton AST Support (12 tests)
**Goal:** Modules and functions with annotations but no implementation — the architect's
project specification. A skeleton AST is the project model before code exists.

**New file: `editor/src/SkeletonAST.h`** (~300 lines)
- `createSkeletonModule(name, language) -> Module*` — empty module with metadata
- `addSkeletonFunction(module, name, params, returnType, annotations) -> Function*`
  — function with signature and annotations but empty body (body = single
  PlaceholderStatement or no children)
- `addSkeletonClass(module, name, methods[], fields[]) -> ClassDeclaration*`
  — class structure with method signatures but no implementations
- `isSkeletonNode(node) -> bool` — checks if a node has no implementation
- `getSkeletonSummary(module) -> SkeletonSummary` — counts: total nodes,
  implemented, skeleton, by-annotation-type
- `skeletonToTaskList(module) -> vector<SkeletonTask>` — converts skeleton nodes
  into a flat task list with routing annotations extracted

**SkeletonTask struct:**
- nodeId, nodeName, nodeType
- contextWidth (from @ContextWidth or inferred)
- automatability (from @Automatability or inferred)
- priority (from @Priority or default)
- reviewRequired (from @Review or inferred from @Ambiguity)
- status (from @ImplementationStatus)
- dependencies (from @Priority.blockedBy)

**Tests:** create skeleton module, add skeleton functions with annotations, skeleton
detection, summary counts, task list generation, annotation extraction, empty module
handling, dependency ordering.

### Step 317: Architect Tooling — RPC + MCP (12 tests)
**Goal:** Expose skeleton creation and project modeling via RPC and MCP tools.

**Modifies: `editor/src/HeadlessAgentRPCHandler.h`**
- `createSkeleton` — create a new skeleton module with language and description
- `addSkeletonNode` — add a function/class skeleton with annotations to a module
- `getProjectModel` — return the skeleton summary + task list for a buffer
- `inferAnnotations` — run AnnotationInference on a buffer, return suggestions

**Modifies: `editor/src/AgentPermissionPolicy.h`**
- `getProjectModel` / `inferAnnotations` as read-only
- `createSkeleton` / `addSkeletonNode` as mutation (Refactor/Generator)

**Modifies: `editor/src/MCPServer.h`** — registerWorkflowTools()
- `whetstone_create_skeleton` — create skeleton module
- `whetstone_add_skeleton_node` — add annotated skeleton function/class
- `whetstone_get_project_model` — get skeleton summary + task list
- `whetstone_infer_annotations` — auto-infer annotations on existing code

**Tool count:** 42+ (was 38): +4 workflow tools

**Tests:** RPC dispatch, MCP tool registration, skeleton creation through MCP,
project model response structure, infer annotations on Python code, permission
enforcement.

### Step 318: Inference-to-Routing Bridge (12 tests)
**Goal:** Connect annotation inference to routing decisions. When inference runs on
code (or a skeleton is created manually), the routing annotations should be derivable.

**Modifies: `editor/src/AnnotationInference.h`**
- `inferRoutingAnnotations(node) -> vector<InferredAnnotation>` — infers workflow
  annotations from code patterns:
  - High cyclomatic complexity → @Ambiguity(high) + @Review(required, human)
  - Simple getter/setter → @Automatability(deterministic)
  - Cross-file dependencies detected → @ContextWidth(project)
  - Async + error handling → @Automatability(llm) + @ContextWidth(file)
  - Single local transform → @Automatability(template) + @ContextWidth(local)
- `estimateContextTokens(node, width) -> int` — rough token count needed for
  the given context width (local: node only, file: buffer, project: all buffers)
- `suggestWorkerType(annotations) -> string` — given a node's annotations,
  suggest "deterministic" | "slm" | "llm" | "human"

**Tests:** complexity → ambiguity mapping, getter → deterministic, cross-file →
project width, token estimation reasonable, worker suggestion matches annotations,
manual annotations override inferred, combined inference + routing pipeline.

### Step 319: Phase 11e Integration + Sprint Summary (8 tests)
**Goal:** Full workflow annotation pipeline and sprint verification.

**Tests cover:**
1. Parse Python → infer annotations across all 8 subjects → verify suggestions
2. Create skeleton module → add annotated functions → get project model → verify task list
3. Infer routing annotations: simple function → deterministic, complex function → llm
4. Skeleton task list respects dependency ordering from @Priority.blockedBy
5. MCP tools/list returns 42+ tools with 4 workflow tools present
6. Sprint totals: 10 parsers, 10 generators, 42+ MCP tools, 0 annotation types dropped
7. Inference + routing + skeleton combined: create skeleton → infer → route → verify
8. Semanno roundtrip includes workflow annotations (Subject 9)

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 11a | 290-296 | 80 | Semanno format + annotation codegen for all 56 missing types |
| 11b | 297-301 | 56 | Validation, conflict detection, transforms for all annotations |
| 11c | 302-308 | 80 | New AST nodes (class/generic/async) + 8 parser deepening |
| 11d | 309-313 | 56 | Kotlin + C# parsers and generators |
| 11e | 314-319 | 68 | Workflow annotation foundation (inference, routing types, skeleton AST) |
| **Total** | **290-319** | **~340** | 30 steps |

**MCP tool count projection:** 42+ tools (38 current + 4 workflow tools)

---

## Key Files to Modify

| File | Changes |
|------|---------|
| `editor/src/ast/ProjectionGenerator.h` | 56 new visitor dispatch branches + 9 new AST node visitors |
| `editor/src/ast/Annotation.h` | Source of truth for all annotation classes; +6 workflow annotations (Subject 9) |
| `editor/src/ast/Serialization.h` | propertiesToJson/createNode/setPropertiesFromJson for 9 new AST node types + 6 workflow annotations |
| `editor/src/AnnotationValidator.h` | Include extended validator, expand from 5 to 67 validated types |
| `editor/src/AnnotationConflict.h` | Include extended, add cross-type conflict detection |
| `editor/src/TransformEngine.h` | Extended with float folding, dead variable elimination, annotation-aware optimization |
| `editor/src/ast/Parser.h` | Include KotlinParser.h + CSharpParser.h |
| `editor/src/Pipeline.h` | Add Kotlin + C# language routing in parse() and generate() |
| `editor/src/MCPServer.h` | Register 4 workflow tools |
| `editor/src/HeadlessAgentRPCHandler.h` | createSkeleton, addSkeletonNode, getProjectModel, inferAnnotations RPCs |
| `editor/src/CompactAST.h` | Workflow annotations in extractSemanticSummary |
| `editor/src/SidecarPersistence.h` | Workflow annotations in isSemanticAnnotation |
| `editor/src/CrossLanguageProjector.h` | Kotlin + C# support, generic annotation cloning |
| `editor/src/MemoryStrategyInference.h` | Kotlin language defaults |
| `editor/src/ast/PythonGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/CppGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/RustGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/GoGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/JavaGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/JavaScriptGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/ElispGenerator.h` | SemannoAnnotationImpl mixin + commentPrefix() |
| `editor/src/ast/Generator.h` | Include KotlinGenerator.h + CSharpGenerator.h |
| `editor/CMakeLists.txt` | Test targets for steps 290-319 |

**New files:**
- `editor/src/SemannoFormat.h` — SemannoEmitter + SemannoParser (11a, done)
- `editor/src/SemannoAnnotationImpl.h` — CRTP mixin for generators (11a, done)
- `editor/src/SemannoSidecar.h` — .semanno sidecar save/load (11a, done)
- `editor/src/ast/AnnotationVisitors.h` — AnnotationVisitorExtended interface (11a, done)
- `editor/src/AnnotationValidatorExtended.h` — E0600-E1299 validation rules
- `editor/src/AnnotationConflictExtended.h` — cross-type conflict detection
- `editor/src/TransformEngineExtended.h` — float folding, dead var, annotation-aware
- `editor/src/ast/ClassDeclaration.h` — ClassDeclaration, InterfaceDeclaration, MethodDeclaration
- `editor/src/ast/GenericType.h` — GenericType, TypeParameter
- `editor/src/ast/AsyncNodes.h` — AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation
- `editor/src/ast/KotlinParser.h` — regex-based Kotlin parser
- `editor/src/ast/KotlinGenerator.h` — Kotlin code generator
- `editor/src/ast/CSharpParser.h` — regex-based C# parser
- `editor/src/ast/CSharpGenerator.h` — C# code generator
- `editor/src/AnnotationInference.h` — generalized multi-subject annotation inference + routing bridge
- `editor/src/SkeletonAST.h` — skeleton module/function creation, project model, task list generation
- `editor/tests/step290_test.cpp` through `editor/tests/step319_test.cpp`

---

## Verification

After each phase:
1. Build all new test targets: `cmake --build . --target stepNNN_test`
2. Run each test: `./stepNNN_test` — expect "Results: 12/12" (or 8/8 for integration)
3. Phase integration test validates no regressions on earlier steps

Full sprint verification:
- All ~340 tests pass across steps 290-319
- Run full test suite (steps 245-319) to verify no regressions
- MCP `tools/list` returns 42+ tools
- Skeleton creation + project model extraction works end-to-end
- Annotation inference covers all 8 subjects + routing annotations
- Semanno roundtrip: generate code with annotations → parse comments back → match
- All 10 languages parse and generate correctly
- 0 annotation types silently dropped during codegen

---

## Sprint 12 Preview

See `roadmap.md` for the full Sprint 12-25+ roadmap.

Sprint 12 builds directly on 11e's foundation:
- WorkItem model (extends SkeletonTask with execution state, assignment, results)
- Task queue with dependency resolution and priority scheduling
- Routing engine that reads annotations and dispatches to workers
- C++ feature-requests items 1-5 for self-hosting progress
