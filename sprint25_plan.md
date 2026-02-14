# Sprint 25 Plan: Integration, Self-Hosting, Polish

## Context

The capstone. Every system built in Sprints 9-24 comes together. Sprint 25 has
three objectives:

1. **Self-hosting:** Whetstone parses, annotates, and transpiles its own source code.
   This is the ultimate integration test — if the tool can understand itself, it can
   understand anything.

2. **End-to-end validation:** Every workflow path is tested with real-world scenarios.
   Not unit tests of individual components, but full journeys: problem description →
   annotated skeleton → orchestrated workflow → transpiled output → security review →
   deployed artifact.

3. **Polish:** Performance optimization, edge case cleanup, documentation for the
   MCP tool surface, and preparation for the post-25 training data phase.

---

## Phase 25a: Self-Hosting (Steps 493-498)

### Step 493: Parse Full Whetstone Codebase (12 tests)
- Feed every .h file in editor/src/ and editor/src/ast/ through the C++ parser
- Track: successful parse rate (target: 95%+), constructs recognized, constructs skipped
- For each file: verify function count, class count, annotation node count
- Log all unparseable constructs for future language depth work
- This is the audit: how complete is our C++ coverage really?

### Step 494: Annotate Whetstone's Own Code (12 tests)
- Run AnnotationInference on parsed AST of Whetstone's source
- Expected findings:
  - @Complexity scores for each major class
  - @Risk annotations on unsafe patterns (raw pointers, void casts)
  - @Owner annotations on unique_ptr/shared_ptr usage
  - @Intent annotations where function names are descriptive enough
  - @TailCall on recursive visitors
- Validate: inferred annotations are actually correct for our code

### Step 495: Transpile Whetstone Modules (12 tests)
- Pick 3 self-contained Whetstone headers and transpile:
  - `AnnotationConflictExtended.h` → Python (structs → classes, simple mapping)
  - `ResponseBudget.h` → Rust (ownership maps cleanly)
  - `Icons.h` → Java (constants → enum class)
- Verify: transpiled code is structurally correct, annotations preserved
- Measure: translation confidence scores, how much was idiomatic vs literal

### Step 496: Self-Annotated Workflow (12 tests)
- Create a workflow for Whetstone's own code modernization:
  - Target: modernize AnnotationValidator.h (add the validation rules it's missing)
  - Skeleton: functions for each new validation rule
  - Route: simple rules → deterministic, complex rules → LLM, new diagnostic codes → human review
- Orchestrate: verify deterministic rules generate valid code
- This is Whetstone improving itself through its own workflow system

### Step 497: Self-Hosting Metrics Report (12 tests)
- Comprehensive self-hosting report:
  - Parse coverage: X/Y files, Z% of constructs
  - Annotation accuracy: how many inferred annotations are correct
  - Transpilation quality: confidence scores across target languages
  - Workflow viability: can we actually use the system to develop itself
- Gap analysis: what's still missing for 100% self-hosting
- Comparison: metrics vs Sprint 16's initial self-hosting attempt (progress delta)

### Step 498: Phase 25a Integration (8 tests)
- Full self-hosting pipeline: parse → annotate → transpile → workflow → validate
- Metrics show 95%+ parse coverage
- At least 3 headers transpile to other languages
- Self-improvement workflow runs and produces valid output
- This is the thesis proven: Whetstone annotates and works on itself

---

## Phase 25b: End-to-End Scenarios (Steps 499-503)

### Step 499: Scenario — Greenfield Project (12 tests)
- Full journey: "Build a URL shortener with user accounts"
  - Problem decomposition → tech stack (Python backend, PostgreSQL, TypeScript frontend)
  - Skeleton generation → annotated modules
  - Workflow creation → orchestration
  - Deterministic tasks auto-completed (CRUD endpoints, model definitions)
  - LLM tasks prepared with context bundles
  - Human review items surfaced for auth logic
  - Security scan on generated code
  - Final project structure on disk

### Step 500: Scenario — Legacy Modernization (12 tests)
- Full journey: modernize a C codebase
  - Ingest legacy C code → safety audit → modernization suggestions
  - Migration plan → Rust as target language
  - Workflow with mixed routing: simple functions deterministic, complex logic LLM
  - API boundary preservation verified
  - Security annotations strengthened (C manual memory → Rust ownership)
  - Migration tests generated

### Step 501: Scenario — Cross-Language Port (12 tests)
- Full journey: port a Python ML pipeline to Rust for performance
  - Parse Python source → annotate → identify hot loops
  - Semantic transpilation: algorithm-level translation
  - Memory model translation: Python GC → Rust ownership
  - Concurrency translation: Python asyncio → Rust tokio patterns
  - Confidence scoring across translation
  - Human review for low-confidence translations
  - Equivalence assertions generated

### Step 502: Scenario — Multi-Model Orchestration (12 tests)
- Full journey: a 20-function project using all worker types
  - 5 deterministic tasks (getters, setters, constructors) → auto-complete
  - 5 template tasks (CRUD, simple transforms) → auto-complete
  - 5 SLM tasks (straightforward logic) → prepared with narrow context
  - 3 LLM tasks (complex logic) → prepared with wide context
  - 2 human tasks (architectural decisions) → flagged for review
  - Progress tracking throughout
  - Cost estimation vs actual comparison
  - All review gates exercised

### Step 503: Phase 25b Integration (8 tests)
- All 4 scenarios run without errors
- Each scenario exercises a different workflow path
- Cost tracking accurate across scenarios
- Security scan produces findings on generated code
- Event stream captures full journey for each scenario
- No regressions across the entire test suite (steps 245-503)

---

## Phase 25c: Polish + Release Prep (Steps 504-508)

### Step 504: Performance Optimization (12 tests)
- Profile the full pipeline on large projects (50+ files, 500+ functions)
- Optimize hot paths:
  - AST serialization/deserialization
  - Annotation inference (batch processing)
  - Context assembly (caching for shared project context)
  - Compact AST generation
- Target: <100ms per function for parse + annotate + route
- Memory: no memory leaks in headless mode under sustained use

### Step 505: MCP Tool Documentation (12 tests)
- Every MCP tool has:
  - Clear description (what it does, when to use it)
  - Input schema with examples
  - Output schema with field descriptions
  - Error codes and their meanings
- Tool categories documented: AST, Diagnostics, Workflow, Routing, Security, etc.
- Generate tool documentation as a resource the MCP client can read

### Step 506: Edge Case Cleanup (12 tests)
- Handle gracefully:
  - Empty files, binary files, extremely large files
  - Malformed annotations, circular dependencies
  - Concurrent workflow modifications
  - Interrupted workflows (save + resume)
  - Unknown language (fallback to plain text)
  - MCP protocol errors (malformed requests)
- Each edge case produces a meaningful error, not a crash

### Step 507: Full Test Suite Regression (12 tests)
- Run the complete test suite: steps 245-506
- Expected: 5000+ tests all passing
- No flaky tests (run 3 times, all pass)
- Performance benchmark: full suite completes in <60 seconds
- Memory baseline: no leaks across full suite

### Step 508: Sprint 25 Summary + Post-25 Readiness (8 tests)
- Final metrics:
  - 19+ language parsers and generators
  - 90+ MCP tools
  - 80+ annotation types across 10 subjects
  - 500+ steps, 5000+ tests
  - Self-hosting at 95%+ coverage
  - 4 end-to-end scenarios validated
- Post-25 readiness:
  - Event stream data accumulated (ready for training data harvest)
  - Workflow decisions logged (routing outcomes, review decisions)
  - Transpilation pairs logged (source → target with confidence)
  - System ready for Sprint 26: Training Data Harvest + Fine-Tuning Pipeline

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 25a | 493-498 | 68 | Self-hosting: parse, annotate, transpile, workflow on own code |
| 25b | 499-503 | 56 | End-to-end scenarios: greenfield, legacy, port, multi-model |
| 25c | 504-508 | 56 | Performance, docs, edge cases, regression, release readiness |
| **Total** | **493-508** | **~180** | 16 steps |

---

## The Numbers

**Across Sprints 9-25:**
- **Steps:** ~508 (245-508)
- **Tests:** ~5000+
- **Languages:** 19+ parsers and generators
- **MCP Tools:** 90+
- **Annotation Types:** 80+ across 10 subjects
- **New AST Node Types:** 30+
- **Sprint Plans:** 17 detailed plan documents

**The system, completed:**
- Universal AST with semantic annotations
- Annotation-driven workflow orchestration (human/AI routing)
- Cross-language semantic transpilation
- Professional GUI with workflow visualization
- MCP-first agent integration (model-agnostic)
- Legacy code ingestion and modernization
- Security-first with OWASP detection and secure-by-default generation
- Self-hosting: the tool understands and works on itself
- Ready for training data harvest from real workflow decisions
