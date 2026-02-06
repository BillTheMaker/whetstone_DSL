# Whetstone DSL: Project Requirements Overview

## Vision Statement

**Source code should be a structured database of logic, not a text file of syntax.**

Whetstone is a Universal Logic Database that stores code as a semantic graph (AST), with traditional programming languages serving as **projections** of that canonical representation. This enables lossless transpilation, massively parallel development (human and AI agents), and complete separation of intent from implementation.

---

## Core Philosophy

### The Problem: Text Blindness
Current software engineering couples **Intent** (what we want to happen) with **Implementation** (how the machine does it) through language-specific syntax. This creates:
- Lock-in to specific languages and ecosystems
- Loss of semantic information during translation
- Inability to express cross-cutting concerns (ownership, risk, optimization hints)
- Barriers to parallel development by multiple agents

### The Solution: Projectional Editing
Move the "Source of Truth" from text files to a **Logical Graph Database**:

| Text-Based | Graph-Based (Whetstone) |
|------------|-------------------------|
| Linear | Structural |
| Ambiguous | Explicit |
| Syntax-heavy | Meaning-heavy |
| Language-specific | Universal |

---

## Architecture: The Three Layers

### Layer 1: The Logic Database (Definition)
A schema of pure logical concepts stored in a graph/tree database (JetBrains MPS).

**Core Principle:** This layer must be a **superset** of all target languages. It must express concepts like memory ownership (Rust-style) even when targeting languages that don't require them (Python).

**Contents:**
- AST node concepts (expressions, statements, functions, types)
- Semantic annotations (SemAnno schema)
- Hypergraph relationships (nodes participate in multiple relationships)

### Layer 2: Semantic Generators (Bridge)
Generators that translate the Logic Database into idiomatic text for specific runtimes.

**Example - SharedMemoryBlock concept:**
```
Generator A (Python):   x = [1, 2, 3]  # Note: Shared usage implied
Generator B (Rust):     let x = Arc::new(Mutex::new(vec![1, 2, 3]));
Generator C (Verilog):  reg [31:0] memory [0:2];
```

### Layer 3: Runtime (Metal)
Standard toolchains (GCC, LLVM, Python interpreter) compile/execute the generated code. Whetstone does not replace compilers; it feeds them optimized, correct input.

---

## SemAnno: Universal Semantic Annotation Schema

Six annotation systems provide metadata for agents (human and AI):

### 1. Complexity & Intelligence ("Rank" System)
Defines who/what is qualified to modify a node.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@complexity-score` | int | Derived from cyclomatic complexity, nesting, mutation |
| `@intelligence-requirement` | junior/mid/senior | Agent tier required for modification |
| `@review-lock` | bool | Requires second agent verification |

### 2. Dependency & Impact ("Radius" System)
Calculates the "blast radius" of changes.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@dependency-count` | int | Incoming references to this node |
| `@dependency-level` | 0-100 | Normalized importance score |
| `@downstream-impact` | list | Affected domains [Network, Security, Billing] |
| `@fragility` | low/high | Load-bearing legacy code flag |

### 3. Performance & Resource ("Machinist" System)
Low-level optimization metadata.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@optimization-priority` | low/mid/high | Generator focus for SIMD/unrolling |
| `@latency-critical` | bool | Real-time execution requirement |
| `@memory-footprint` | estimate | Heap/stack allocation intent |
| `@execution-mode` | deterministic/probabilistic | Formal verification hint |
| `@deref-explicit` | bool | Every pointer touch must be accounted |

### 4. Architectural Intent ("Paradigm" System)
Defines behavioral contracts.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@intent` | Pure/Stateful/Parallel/Legacy | Primary behavioral contract |
| `@preferred-view` | language | Optimal projection for readability |
| `@lifecycle` | owner/borrower/transient | Ownership model hints |

### 5. Security & Risk ("Sentinel" System)
Granular risk tracking.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@risk-type` | identifier | BufferOverflow, RaceCondition, SideChannel |
| `@trust-boundary` | marker | Secure environment transition point |
| `@provenance` | hash | Chain of agent modifications |

### 6. Projective Metadata ("Lens" System)
Controls rendering in projectional environments.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@view-mask` | category | Toggle annotation visibility by type |
| `@cognitive-level` | 0-100 | Detail density filter |
| `@projection-alias` | string | Localized renaming without AST change |
| `@hidden-detail` | bool | Collapse boilerplate for reviews |

### 7. Language-Specific Idioms ("Polyglot" System)
Preserves language-specific features that don't map directly to other languages.

| Annotation | Type | Purpose |
|------------|------|---------|
| `@lang_specific` | structured | Captures idiom with language, type, raw syntax, and semantic hint |

**Architecture:**
```
@lang_specific(python, "decorator", "@lru_cache(maxsize=128)", hint="memoization")
@lang_specific(cpp, "template", "template<typename T>", hint="generic")
```

**Projection Behavior:**
- In native language: renders as original syntax
- In foreign language: renders as comment with semantic hint

**Long-term Vision:** These annotations enable future semantic equivalence detection. When the system understands that `@lru_cache` and a C++ `std::map`-based cache serve the same purpose ("memoization"), it can generate idiomatic implementations in either direction rather than just commenting.

---

## Memory Dereferencing Strategies

Memory management is decoupled from logic as a first-class AST field.

| Strategy | Description | Generator Responsibility |
|----------|-------------|-------------------------|
| `@deref(imperative)` | Manual control - developer defines time/location | Whetstone Generator / Substrate |
| `@deref(streamed)` | Ownership-based - lifetime tracked at declaration (Rust-like) | Whetstone Generator |
| `@deref(batched)` | Garbage collected - background process or ref-counting | Whetstone Generator |
| `@deref(content-addressed)` | Immutable - identity by hash, location irrelevant | Whetstone Generator |

### The "Unfilled Node Constraint"
In `@deref(imperative)` mode, the AST requires an explicit **Dereference Time Field**. If empty, the editor flags it as a "Missing Intent" error. This makes it impossible to forget a dereference.

---

## Strategy Toggles

Declarative optimization patterns applicable without manual implementation:

| Toggle | Effect |
|--------|--------|
| `@strategy(TCO)` | Convert recursive logic to iterative machine code |
| `@strategy(Memoize)` | Inject thread-safe cache for function results |
| `@strategy(Unroll)` | Duplicate loop bodies per target-hardware heuristic |
| `@strategy(SSO)` | Enable Small String/Buffer Optimization |

---

## Transpilation Modes

### Direct Transpilation ("Wrapper")
Maps nodes 1-to-1. High compatibility, poor performance.
- Python dynamic list → `std::vector<std::any>` with runtime type-checking

### Idiomatic Transpilation ("Intent")
Analyzes usage patterns. High performance, hardware sympathy.
- Python list (append-only, sequential read) → `std::vector<T>` or Memory Arena

---

## The Resolution Slider

Four levels from intent to execution:

| Level | Description | Example |
|-------|-------------|---------|
| 1 | Pseudocode/Intent | "Find a user in a list" |
| 2 | High-Level (JS/Python) | `Array.find()` |
| 3 | Intermediate (Rust/Java) | Type safety + ownership scopes |
| 4 | Execution Substrate (C++/C) | B-Trees, SIMD, Branch Hints |

---

## Multi-Agent Collaboration

### The Round-Trip Problem
When a systems engineer optimizes code that a high-level developer later modifies.

### Solution: Persistent Metadata Separation
- **Logic Field:** Algorithmic intent (editable by junior)
- **Optimization Field:** Systems-level instructions (requires senior)

### Conflict Management
When high-level changes break low-level optimizations:
1. **Shadowed Metadata:** Old optimizations hidden but not deleted
2. **Alerting:** "Performance Guardrail" warning shown
3. **Diff Lens:** Visualizes where intent changes invalidated hardware instructions

---

## Hypergraph Requirement

Standard ASTs are strictly hierarchical (Parent → Child). Real-world logic is a **Hypergraph**:

- A Variable is **owned by** Function A
- But **used by** Function B
- And **monitored by** Agent C

**Implementation:** Smart References (Hyperedges) allow a single logical node to participate in multiple relationships simultaneously.

---

## Agent-Native Development

### Task Decomposition Model
1. **High-context coordinator agent** builds semantic graph and queues tasks
2. **Small Language Models (8B)** handle tightly scoped tasks
3. **Human developers** work from the same task queue

### Projection Optimization
Tasks can be projected in:
- **Human-optimized view:** Natural language descriptions, readable code
- **AI-optimized view:** Structured data, explicit constraints, minimal ambiguity

### Agent Tiers
| Tier | Capabilities |
|------|--------------|
| Junior | Basic logic, no memory management, no high-risk annotations |
| Mid | Standard logic, `@deref(streamed)` and `@deref(batched)` |
| Senior | `@deref(imperative)`, hardware optimization, high-risk refactoring |

---

## Target Applications

### Legacy Modernization
Parse old Fortran/COBOL → Logic Database → Generate modern C++/Rust/Julia

### Cross-Platform Logic
Write business logic once → Generate Kotlin (Android), Swift (iOS), Rust (backend)

### Agentic Coding
Agents manipulate Graph Nodes directly (no syntax errors from text generation)

### Parallel Development
Multiple developers/agents work on same codebase with explicit conflict boundaries

---

## Technical Principles

### Behavior Over Implementation
A node's primary definition is its **Behavioral Contract** (e.g., "Sort this collection") rather than implementation (e.g., "Quicksort with pointer increments").

### Substrate-Agnostic Core Nodes
Core AST nodes must not assume specific memory models or hardware constraints.
- **Bad:** LoopNode requires a "Pointer" field
- **Good:** LoopNode requires an "IteratorIntent" - realization is metadata

### The Semantic Boundary
Strict boundary between **What** (Logic) and **How** (Execution Strategy). Future capabilities are implemented by adding Strategy Engines to existing nodes, not re-architecting nodes.

### Strictest Common Denominator
The Logic Database enforces strict rules by default (explicit ownership), while Generators can "relax" rules for permissive languages or "poly-fill" rules for strict languages.

---

## The C Primitives Substrate (Long-term Vision)

All high-level language features ultimately resolve to C-level primitives:

| High-Level Feature | C Primitive Resolution |
|-------------------|------------------------|
| Python `@lru_cache` | Hash table + function pointer + malloc/free |
| C++ `template<T>` | N compiled copies with concrete types |
| Python decorator | Function pointer manipulation |
| C++ RAII | Constructor/destructor at scope boundaries |
| Rust ownership | Compile-time tracking → equivalent to manual C |
| Garbage collection | Reference counting or mark-sweep algorithms |

**Architectural Implication:**

```
Level 4: Language Idioms     @py_decl, @cpp_tmpl, @rust_lifetime
            │
            ▼
Level 3: Universal AST       Function, Loop, Variable, Expression
            │
            ▼
Level 2: Semantic Intent     "memoize", "parameterize by type", "scope-bound cleanup"
            │
            ▼
Level 1: C Primitives        pointers, structs, malloc/free, function calls
```

**End State:** Languages become pure *syntactic projections* over C primitives + semantic annotations. This enables true lossless transpilation because the canonical form captures both the intent (semantic annotations) and the implementation (C primitives).

---

## Implementation Platform

**JetBrains MPS (Meta Programming System)**
- Projectional editor (edit AST directly, not text)
- Language workbench for defining DSLs
- Built-in support for generators
- Extensible type system and constraints

---

## Success Criteria

1. **Lossless Round-Trip:** Python → AST → C++ → AST → Python produces semantically equivalent code
2. **Parallel Safety:** Junior developer cannot accidentally break senior's optimizations
3. **Agent Compatibility:** Tasks can be completed by SLMs with sufficient annotation context
4. **Performance Parity:** Generated C++ matches hand-written C++ for common patterns
5. **Adoption Path:** Existing codebases can be incrementally imported via tree-sitter parsing
