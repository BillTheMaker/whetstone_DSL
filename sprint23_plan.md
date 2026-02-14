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

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 23a | 471-476 | 68 | Problem decomposition, tech stack selection, skeleton generation |
| 23b | 477-481 | 56 | Templates, scaffolding, build awareness, multi-language orchestration |
| **Total** | **471-481** | **~124** | 11 steps |
