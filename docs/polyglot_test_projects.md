# Polyglot Test Projects
> Created: 2026-02-28
> Purpose: Validate the polyglot orchestrator at each phase. Projects are ordered
> by complexity — 2-language pairs first, then multi-language, then all-language.
> None of these are production software. They are boundary stress tests.

---

## Tier 1: 2-Language Projects

These prove the basic pipeline: fitness scoring, FFI glue generation, symbol
index, and cross-language LSP. They use language pairs with well-understood
interop stories so failures are clearly attributable to the orchestrator.

---

### poly-sort

**Languages:** Rust + Python
**Pattern:** Performance core in Rust, scripting/validation layer in Python

```
Python (test runner)
  │  generates random arrays, validates output
  │  calls via ctypes (generated from shared AST)
  ▼
Rust (sort implementations)
  │  merge sort, quick sort, radix sort
  │  exposed as C ABI functions
  └─ returns sorted array to Python
```

**Fitness rationale:**
- Rust: flat array mutation, cache-coherent loops, zero GC pressure → systems fitness
- Python: test orchestration, random generation, human-readable assertions → scripting fitness

**What it tests:**
- `LanguageFitnessScorer` recommends Rust for the sort functions, Python for the harness
- `RustPythonBindingEmitter` generates ctypes bindings for `sort_merge`, `sort_quick`, `sort_radix`
- Symbol index links Python `sort_merge` call site to Rust `sort_merge` definition
- Goto-definition from Python lands on Rust
- Debugger can step from Python test into Rust sort function

**Acceptance test:**
- 1000 random arrays of integers: all three sort variants produce output matching Python's `sorted()`
- No hand-written FFI code in the project

---

### poly-api

**Languages:** Go + TypeScript
**Pattern:** API server in Go, typed client in TypeScript

```
TypeScript (browser/Node client)
  │  typed API client generated from shared AST
  │  calls HTTP endpoints
  ▼
Go (HTTP server)
  │  router, handlers, response serialization
  │  types defined in shared AST
  └─ serves JSON responses
```

**Fitness rationale:**
- Go: goroutine-based HTTP handling, simple concurrency, excellent standard library → concurrency fitness
- TypeScript: strong types for API contracts, browser-compatible, React/Node ecosystem → typed scripting fitness

**What it tests:**
- Shared type definitions (request/response structs) generated in both Go and TypeScript from same AST nodes
- No OpenAPI spec written by hand — the shared AST is the source of truth
- Symbol index links TypeScript `UserResponse` interface to Go `UserResponse` struct
- Rename of a field in shared AST propagates to both Go and TypeScript simultaneously

**Acceptance test:**
- TypeScript client calls Go server, receives typed response
- Field rename via LSP orchestrator compiles cleanly in both languages
- No hand-written type definitions or API client code

---

### poly-parse

**Languages:** C++ + Haskell
**Pattern:** High-performance lexer/tokenizer in C++, algebraic parser in Haskell

```
Haskell (parser)
  │  recursive descent, pattern matching on token ADT
  │  calls C++ tokenizer via FFI
  ▼
C++ (lexer)
  │  character-level scanning, zero allocation
  │  exposes token stream as C ABI iterator
  └─ returns tokens to Haskell
```

**Fitness rationale:**
- C++: character-level loops, cache-coherent buffers, zero allocation → systems fitness
- Haskell: recursive ADT pattern matching, algebraic data types → functional fitness

**What it tests:**
- C++ → Haskell FFI (unusual direction; most polyglot FFI is C calling higher-level)
- `CHeaderEmitter` for the token stream iterator
- `GoCppBindingEmitter` is not used here — this exercises a new binding pair (C++↔Haskell via C ABI)
- Debugger: step from Haskell parser into C++ lexer and back

**Acceptance test:**
- Parse a sample grammar (JSON or a simple expression language)
- Output AST from Haskell matches reference Python parser for the same input
- No hand-written FFI

---

## Tier 2: Multi-Language Projects

These stress the orchestrator beyond two-language pairs. Each adds languages
that stress different aspects of the system (more binding pairs, more LSP
servers, larger symbol index, more complex debug adapter coordination).

---

### poly-pipeline

**Languages:** Python + Rust + Go + Haskell + TypeScript (5 languages)
**Pattern:** Data pipeline where each stage is the best language for that computation

```
Python          CSV ingestion, schema validation, error reporting
   ↓ FFI (ctypes)
Rust            Data normalization, outlier detection, statistics
   ↓ FFI (CGo)
Go              Fan-out routing to multiple sinks, concurrent dispatch
   ↓ FFI (C ABI)
Haskell         Statistical transformation, pure functional reduction
   ↓ FFI (N-API)
TypeScript      JSON serialization, HTTP push to downstream consumer
```

**Fitness rationale:**
- Python: schema parsing, human-readable error messages, ecosystem (pandas for validation) → scripting
- Rust: numerical normalization on large arrays, no GC pauses at this stage → systems
- Go: goroutines for fan-out, select for concurrent dispatch → concurrency
- Haskell: pure reduction functions (fold, map), referential transparency at this stage → functional
- TypeScript: JSON is native, HTTP client ecosystem mature, consumer-facing → typed scripting

**What it tests:**
- 4 FFI boundaries generated from shared AST (Python↔Rust, Rust↔Go, Go↔Haskell, Haskell↔TS)
- Symbol index with 5 language targets — navigation across 4 boundaries
- LSP orchestrator running 5 language servers simultaneously
- DAP orchestrator stepping through 5-language call chain
- Parity: replace each stage with Python reference implementation, output must match

**Acceptance test:**
- CSV file in → HTTP POST to mock consumer
- All 5 stages running as one connected process (not microservices)
- End-to-end wall time within 2x of a pure Python reference implementation

---

### poly-compiler

**Languages:** C++ + Haskell + Rust + Lisp (Scheme) + Go (5 languages)
**Pattern:** A toy compiler where each phase is the language most fit for it

```
C++             Lexer — character-level scanning, no allocations per token
   ↓ C ABI
Haskell         Parser — recursive descent, algebraic token → AST types
   ↓ C ABI
Rust            Type checker — ownership-tracking symbol table, zero-cost
   ↓ C ABI
Lisp/Scheme     Optimizer — s-expression AST rewriting, pattern-matched rules
   ↓ C ABI
Go              Code emitter — parallel emission of bytecode, concurrent writes
```

**Fitness rationale:**
- C++: character-level tight loops, arena allocation for token buffer → systems
- Haskell: token ADTs, recursive descent naturally expressed as algebraic functions → functional
- Rust: symbol table with lifetime-safe references, borrow checker prevents dangling refs → systems/safety
- Lisp: the optimizer manipulates an AST that is itself S-expressions — homoiconicity → meta-programming
- Go: parallel bytecode emission with goroutines, simple coordination → concurrency

**What it tests:**
- The most philosophically interesting FFI direction: Lisp optimizing Rust's output then handing to Go
- The Lisp binding pair (Scheme↔C ABI) is a new surface for the glue generator
- poly-compiler is itself written in the languages it compiles — not intentionally self-hosting,
  but the structural similarity is notable

**Acceptance test:**
- Compiles the following source string: `(let x 42) (+ x 1)` → bytecode `[PUSH 42, STORE x, LOAD x, PUSH 1, ADD]`
- Each phase independently replaceable with Python reference implementation (parity check)

---

## Tier 3: The Proof

---

### poly-everything

**Languages:** All supported Whetstone generation languages
**Pattern:** One function per language, fitness-assigned. Each calls the next.

The fitness scorer assigns each language to the function most suited to it.
The chain is determined by the scorer, not specified manually.
All FFI glue generated. All boundaries in the symbol index.
Full LSP + DAP.

**Current supported generation languages in Whetstone:**
Python, C++, Rust, Go, JavaScript/TypeScript, Java, Haskell, Elixir, Lisp/Scheme,
Julia, PHP, Rust (embedded), WASM (via Rust), Kotlin, Swift, Zig

**Suggested chain** (fitness scorer should reproduce this independently):
```
Zig             Memory-mapped file reader (ultra-low-level, no runtime)
   ↓
C++             Binary deserialization (struct parsing, no GC)
   ↓
Rust            Validation and normalization (safety-critical, owned types)
   ↓
Haskell         Schema transformation (pure functional, algebraic types)
   ↓
Lisp/Scheme     Rule application (S-expression pattern matching)
   ↓
Julia           Statistical analysis (array operations, broadcasting)
   ↓
Python          Result formatting (readable output, ecosystem)
   ↓
Go              HTTP dispatch (concurrency, fan-out)
   ↓
TypeScript      JSON response (typed client contract)
   ↓
Java            Persistent storage (JDBC, enterprise ecosystem)
   ↓
Kotlin          Android notification (Kotlin coroutines, Android SDK)
   ↓
Swift           iOS notification (Swift async/await, Apple SDK)
   ↓
Elixir          Real-time broadcast (actor model, Phoenix channels)
   ↓
PHP             CMS integration (WordPress hook, legacy ecosystem)
   ↓
WASM            Browser computation (sandboxed, portable)
   └─ returns to Zig (closes the chain)
```

**What it proves:**
- The shared AST can represent computations across the full spectrum of
  language paradigms: systems, functional, OO, scripting, actor, array, meta
- FFI glue can be generated for every adjacent pair in the chain
- The symbol index spans all language targets
- The LSP orchestrator can route queries across N language boundaries
- The DAP orchestrator can present a coherent stack trace spanning all languages
- Parity: every function replaceable by a Python reference implementation

**Acceptance test:**
- Binary file in → push notification + browser computation + CMS update + database write
- No hand-written FFI, no hand-written bindings, no hand-written type definitions
- The only human artifact is the spec

**Parity acceptance test:**
- Replace each language stage with a Python reference implementation
- Output must be identical for all 100 test inputs
- This is the formal proof of lossless transpiling

---

## What This Actually Means

If poly-everything passes, we have demonstrated:

1. **For integration layers**: Any environment boundary (Python↔C++, Go↔Java, Rust↔WASM)
   can be generated from a shared spec. This was previously only possible for major projects
   with dedicated platform integration teams (GraalVM, JPype, pybind11, etc.).

2. **For legacy modernization**: An existing system in language A can have a spec written
   from it, and a reimplementation in language B generated with correct type-safe boundaries.
   The language A and language B versions run in parallel and their outputs are compared
   automatically to verify the migration.

3. **For new projects**: Language selection stops being a permanent architectural decision.
   A component can be written in the fitness-optimal language and later regenerated in a
   different language if the fitness profile changes (e.g. scaling requirements shift
   from scripting-friendly to systems-performance).

The practical value is the integration layer capability. The theoretical value is
the proof that language is a generation parameter, not an architectural commitment.
