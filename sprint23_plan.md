# Sprint 23 Plan: Architect Mode + Tech Stack Selection

## Context

The endgame feature: describe a problem in natural language, and the system
proposes a technology stack, project structure, and annotated skeleton. This
is the architect's power tool — not replacing the architect, but giving them
a structured starting point they can review, modify, and approve.

Sprint 23 builds on everything: the annotation taxonomy (Sprints 10-11),
the skeleton AST (Sprint 11e), the workflow model (Sprint 12), the orchestrator
(Sprint 15), the language coverage (17+ languages), and the transpilation engine
(Sprint 21).

---

## Phase 23a: Problem Decomposition (Steps 471-476)

### Step 471: Problem Description Parser (12 tests)
- Accept natural language problem description
- Extract structured requirements:
  - Functional requirements (what it should do)
  - Non-functional requirements (performance, security, scalability)
  - Platform constraints (web, mobile, embedded, server, CLI)
  - Integration requirements (databases, APIs, existing systems)
- Output: StructuredRequirements JSON with confidence per extraction
- This is the one place where LLM assistance is expected in the architecture —
  the extraction is an MCP prompt template, not hardcoded NLP

### Step 472: Module Decomposition Engine (12 tests)
- Given StructuredRequirements, propose module structure:
  - Identify major subsystems (auth, data, API, UI, etc.)
  - Define module boundaries and interfaces
  - Estimate complexity per module
  - Identify cross-cutting concerns (logging, error handling, config)
- Output: ModuleGraph with nodes (modules) and edges (dependencies)
- Multiple decomposition strategies: microservice vs monolith vs layered

### Step 473: Technology Stack Selector (12 tests)
- Given requirements + module graph, recommend tech stack:
  - Language selection per module (guided by requirements):
    - Performance-critical → Rust or C++
    - Web frontend → TypeScript
    - Data processing → Python
    - System scripting → Go or Python
    - Database → SQL dialect based on requirements
    - Embedded → C or assembly
  - Framework suggestions (as annotations, not framework-specific code)
  - Database selection based on data requirements
- Configurable preferences: "I prefer Rust for backend", "must use PostgreSQL"
- Output: TechStackDecision with reasoning per choice

### Step 474: Skeleton Generation from Requirements (12 tests)
- Given module graph + tech stack:
  1. Create skeleton modules in chosen languages
  2. Create skeleton functions/classes with @Intent annotations from requirements
  3. Add routing annotations: @Complexity, @ContextWidth, @Automatability
  4. Add contract annotations: @Contract from functional requirements
  5. Add dependency annotations between modules
- Output: Multi-file skeleton project ready for workflow creation

### Step 475: Architect Review Interface (12 tests)
- Present the proposed stack + skeleton to the architect for review:
  - Tech stack summary with reasoning
  - Module dependency graph (visual in GUI, structured in MCP)
  - Per-module: language choice, complexity estimate, key annotations
  - Modification tools: change language, merge modules, add modules, adjust routing
- Architect approves → skeleton becomes the project spec
- Architect modifies → re-run affected selections

### Step 476: Phase 23a Integration (8 tests)
- Full flow: "Build a REST API for a bookstore with user auth, PostgreSQL,
  and a React frontend" → structured requirements → module decomposition →
  tech stack (TypeScript frontend, Python/Rust backend, PostgreSQL) →
  skeleton project → architect review → approved → workflow created
- 87+ MCP tools total

---

## Phase 23b: Project Templates + Scaffolding (Steps 477-481)

### Step 477: Architecture Templates (12 tests)
- Pre-built architecture patterns:
  - **REST API:** routes + handlers + models + middleware + database
  - **CLI Tool:** argument parsing + commands + output formatting
  - **Library:** public API + internal modules + tests + documentation stubs
  - **Microservice:** service + transport + storage + health check
  - **Full Stack:** frontend + backend + database + deployment config
- Templates are parameterized: entity names, field definitions, auth method
- Each template produces a fully-annotated skeleton project

### Step 478: Scaffold File Generation (12 tests)
- Generate actual project files on disk from approved skeleton:
  - Source files with skeleton code + Semanno annotations
  - Project configuration (package.json, Cargo.toml, pyproject.toml, CMakeLists.txt)
  - Directory structure following language conventions
  - .gitignore appropriate for selected stack
  - .whetstone/ directory with sidecar files and workflow state
- Uses existing fileCreate/fileWrite MCP infrastructure

### Step 479: Dependency + Build System Awareness (12 tests)
- Annotate dependencies between modules with build system info:
  - Python: imports and pip requirements
  - Rust: crate dependencies in Cargo.toml format
  - Node: npm packages in package.json format
  - C++: include paths and CMake targets
  - SQL: migration ordering
- Not full build system integration — annotation-level awareness for context

### Step 480: Multi-Language Project Orchestration (12 tests)
- Workflow creation for multi-language projects:
  - Each module may be a different language
  - Cross-language interfaces annotated with @Link and @Shim
  - Build order respects cross-language dependencies
  - FFI boundaries explicitly annotated for human review
- Orchestrator handles multi-language workflow as a single project

### Step 481: Phase 23b Integration + Sprint Summary (8 tests)
- REST API template → scaffold files on disk → workflow → orchestrate
- Multi-language project: Python service + Rust core + SQL schema
- Architect modifies tech stack mid-planning → skeleton regenerates
- All scaffolded files have proper annotations and structure
- Sprint 23 totals: architect mode operational, templates working

---

## Phase 23c: Self-Hosting — Architect/Worker Dispatch (Steps 482-487)

### Context

Phases 23a and 23b build architect mode for *user projects*. Phase 23c
turns it inward: can the system architect and dispatch *its own* sprint
steps to worker agents? This is the self-hosting proof for the
architect/worker thesis.

The fixed "12 tests per step" invariant was a useful scaffold during early
sprints, but it's wrong in principle. The number, type, and depth of tests
should emerge from the complexity of what's being built — not from a
template. A trivial utility struct needs 3 unit tests. A safety-critical
pipeline needs unit tests, integration tests, contract tests, fuzz
boundaries, and negative tests. Phase 23c replaces the fixed count with
a test plan generator that reasons about *what kind of testing* a step
needs.

### Testing taxonomy used by the test plan generator

| Type | Purpose | When to use |
|------|---------|-------------|
| **Unit** | Single function/method correctness | Every public API surface |
| **Negative** | Confirm rejection of bad input | Validators, parsers, security boundaries |
| **Integration** | Multiple components working together | Phase-end steps, pipeline steps |
| **Contract** | Pre/post condition verification | Functions with @Contract annotations |
| **Regression** | Prior steps still pass | Every step (run prior test binaries) |
| **Boundary** | Edge cases at type/size limits | Arithmetic, buffers, collections |
| **Property** | Invariant holds across random inputs | Sorting, serialization roundtrips |
| **Smoke** | Quick "does it build/link/run" | Build system changes, scaffolding |
| **Performance** | No regression in speed/memory | Hot paths, algorithms with @Complexity |

The test plan generator selects from these types based on the step
description, not a fixed formula. A step adding a data struct might get
4 unit tests and 1 boundary test. A step wiring an RPC pipeline might
get 3 unit tests, 4 integration tests, and 2 negative tests. The total
count varies.

---

### Step 482: Step Spec Expander (tests: unit, negative, boundary)
- Given a sprint plan step description (the markdown block), produce:
  1. **Test plan**: which test types apply, how many of each, named test
     function stubs with one-line descriptions of what each verifies
  2. **Header skeleton**: main struct/class outlines with field names,
     method signatures, and doc-comments derived from the step description
  3. **Build system entry**: CMake target (or Cargo.toml entry, etc.)
     appropriate for the project's build system
- Input: step markdown + project conventions + prior step headers
- Output: `StepSpec` containing test plan, header skeleton, build entry
- The expander does NOT write final implementations — it writes enough
  structure that a worker agent can fill in the bodies without guessing
  at API shapes or test naming conventions
- Complexity classification: the expander reads the step description and
  tags it as `trivial` (1-4 tests), `standard` (5-10 tests),
  `complex` (10-15 tests), or `pipeline` (integration suite, 6-10 tests)

### Step 483: Convention Extractor + Validator (tests: unit, negative, regression)
- Scan an existing codebase and extract a machine-readable conventions doc:
  - Test harness pattern (macros, framework, assertion style)
  - Naming conventions (struct names, file names, function names)
  - File organization (where headers go, where tests go, include paths)
  - Size limits (max lines per file, max functions per class)
  - Build system patterns (how targets are added, link libraries)
  - Code style (header-only vs .cpp, namespace usage, include guards)
- Output: `ProjectConventions` JSON that the step expander and worker
  validator consume
- **Validator mode**: given a completed step's files + conventions,
  report violations:
  - Header over size limit
  - Wrong test harness pattern
  - Missing include path
  - Struct/function naming doesn't match project style
  - Build target missing or malformed
- Violations are warnings, not hard errors — the architect decides which
  to enforce

### Step 484: Prior-Step Context Injector (tests: unit, integration)
- When generating or dispatching a step, automatically gather context
  from prior steps that the worker will need:
  - Read the previous step's header(s) to extract struct definitions,
    enum values, and public method signatures
  - Identify which types from prior steps the new step is expected to
    use, extend, or compose
  - Build a condensed "context brief" (not the full file — just the API
    surface: struct names, field types, method signatures)
- Output: `StepContext` containing:
  - prior step API summaries (types + methods, no implementations)
  - dependency graph (which prior headers this step needs to #include)
  - naming patterns observed (if prior steps use `FooReport`, this step
    should use `BarReport`, not `bar_report_t`)
- The context brief is small enough to fit in a worker agent's prompt
  without consuming most of its context window

### Step 485: Worker Dispatch + Result Validation (tests: unit, integration, negative)
- Orchestrate the full architect→worker flow:
  1. Architect provides step description (from sprint plan)
  2. Step expander produces `StepSpec` (test plan + skeleton + build entry)
  3. Context injector produces `StepContext` (prior step APIs)
  4. Convention extractor produces `ProjectConventions`
  5. Worker agent receives: StepSpec + StepContext + ProjectConventions
  6. Worker produces: completed header + completed test file + build entry
  7. Convention validator checks the worker's output
  8. Build system compiles and runs tests
  9. If tests pass and conventions are met → step is done
  10. If tests fail → worker gets error output and retries (bounded)
  11. If conventions violated → report to architect for decision
- This step models the dispatch loop, not the actual agent execution —
  it validates that the data structures and handoff protocol work
- Retry budget: configurable, default 3 attempts before escalating
  to architect

### Step 486: Test Plan Generator (tests: unit, boundary, negative)
- The intelligence behind "how many tests and what kind":
  - Reads step description and classifies complexity
  - Reads prior step test files to understand testing patterns
  - Proposes test types from the taxonomy (unit, negative, integration,
    contract, boundary, property, smoke, performance, regression)
  - For each proposed test: name, type, one-line description, expected
    routing (can a worker write this, or does the architect need to?)
- Heuristics:
  - Step mentions "detect" or "validate" → include negative tests
  - Step mentions "pipeline" or "workflow" → include integration tests
  - Step mentions "RPC" or "MCP" → include smoke tests (does it respond?)
  - Step mentions "score" or "arithmetic" → include boundary tests
  - Step is a phase-end integration → regression suite
  - Step adds a new struct → unit tests for construction/accessors
  - Step modifies existing behavior → regression tests mandatory
- Output: `TestPlan` with typed, named, described test stubs
- The test plan is reviewed by the architect before the worker starts —
  the architect can add/remove/modify tests before dispatch

### Step 487: Phase 23c Integration (tests: integration, regression)
- Full self-hosting loop on a real sprint step:
  1. Take a step description from an existing sprint plan
  2. Run convention extractor on the current codebase
  3. Run context injector for the step's position in the sprint
  4. Run step expander to produce spec
  5. Run test plan generator to produce typed test plan
  6. Validate that the spec + plan + context is sufficient for a
     worker to implement without asking questions
  7. Run convention validator on a known-good completed step to
     verify zero violations
  8. Run convention validator on a deliberately-broken step to
     verify it catches violations
- Verify the full data flow: sprint plan → step spec → worker brief →
  output validation
- This does NOT require actual LLM worker dispatch — it validates the
  data structures and protocol that make dispatch possible

---

## Revised Architecture Invariant

The old invariant was:
> "12 tests per step, 8 for integration" — consistent test density

The new invariant is:
> **Test plans are generated per-step from complexity analysis.**
> Test count and type emerge from what's being built, not from a template.
> The test plan generator proposes; the architect approves.
> Regression tests on prior steps are always included.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 23a | 471-476 | ~68 | Problem decomposition, tech stack selection, skeleton generation |
| 23b | 477-481 | ~56 | Templates, scaffolding, build awareness, multi-language orchestration |
| 23c | 482-487 | varies | Self-hosting: step expansion, conventions, worker dispatch, test planning |
| **Total** | **471-487** | **varies** | 17 steps |

Phase 23c test counts are intentionally not fixed — they will be determined
by the test plan generator itself (Step 486) once it exists. Until then,
each step lists which test *types* apply rather than a count.
