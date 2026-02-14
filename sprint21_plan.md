# Sprint 21 Plan: Cross-Language Transpilation Engine

## Context

Until now, "cross-language projection" has been syntactic: map AST nodes from
source language conventions to target language conventions. Sprint 21 makes it
semantic: the transpiler understands what code *does*, not just what it *looks like*,
and produces behaviorally equivalent output in the target language.

The key difference: syntactic projection maps `for i in range(10)` to `for (int i = 0; i < 10; i++)`.
Semantic transpilation understands that this is "iterate 10 times" and selects the
most idiomatic, efficient representation in the target: `(0..10).for_each(|i| ...)` in
Rust, `(dotimes (i 10) ...)` in Common Lisp, `repeat(10) { ... }` in Kotlin.

Annotations drive the translation: @Intent tells the transpiler what the code means,
@Complexity guides how much to restructure, @Risk determines how conservative to be.

---

## Phase 21a: Semantic Translation Layer (Steps 449-454)

### Step 449: Intent-Driven Translation (12 tests)
- @Intent annotations on functions describe what they do, not how
- Translation engine reads intent and selects target-idiomatic implementation:
  - @Intent("sort list ascending") → Python: `sorted()`, Rust: `.sort()`, SQL: `ORDER BY`
  - @Intent("find element by key") → Python: dict lookup, Java: HashMap.get, SQL: WHERE
- When intent is clear + target has idiomatic equivalent → direct translation
- When intent is ambiguous → preserve source structure + flag for review

### Step 450: Algorithm-Level Equivalence (12 tests)
- Recognize algorithmic patterns in source AST:
  - Linear search → map to target's standard library search
  - Sort implementations → map to target's standard library sort
  - Map/filter/reduce patterns → map to target's functional idioms
  - Builder patterns → map to target's construction idioms
- Pattern library: 20+ common algorithmic patterns with cross-language equivalents
- @Complexity annotation guides whether to use standard library or preserve manual implementation

### Step 451: Type System Translation (12 tests)
- Map type systems across languages with annotation guidance:
  - Nullable types: Java Optional → Rust Option → Python Optional → C nullable pointer
  - Generic types: Java List<T> → Rust Vec<T> → Python list[T] → C void* + size
  - Enum types: Java enum → Rust enum → Python Enum → C enum/constants
  - Class hierarchies: inheritance → traits/interfaces → protocols → struct+vtable
- @TypeState and @Nullability annotations preserve type-safety intent across languages
- Lossy translations (e.g., generics → void*) get @Risk(medium) annotation

### Step 452: Concurrency Model Translation (12 tests)
- Map concurrency patterns across language runtimes:
  - Python async/await → Rust async/await (different runtime model!)
  - Java threads → Go goroutines → Rust threads → C pthreads
  - Python GIL-safe → Rust Send+Sync → Go channel-based → C mutex-based
  - JavaScript Promises → Rust futures → Python asyncio
- @Exec, @Sync, @ThreadModel annotations guide the translation
- @Blocking annotation prevents mapping async → sync or vice versa without review

### Step 453: Memory Model Translation (12 tests)
- Translate ownership/lifecycle across memory models:
  - Rust ownership → C++ smart pointers → C manual → Java/Python GC
  - @Owner(Unique) → unique_ptr → malloc+single_free → (no equivalent, GC)
  - @Owner(Shared_ARC) → shared_ptr → reference counting → GC
  - @Lifetime(Scope) → RAII → stack allocation → try-finally
- Loss of safety tracked: Rust → C loses borrow checker guarantees → @Risk(high)
- Gain of safety tracked: C → Rust gains borrow checker → @Risk(lowered)

### Step 454: Phase 21a Integration (8 tests)
- Semantic transpilation: Python data processing → Rust (idiomatic, not line-by-line)
- Algorithm recognition: Python bubble sort → Rust .sort() when @Intent("sort") present
- Type system: Java generics → Rust generics (preserved), Java → C (void* with @Risk)
- Concurrency: Python async → Rust async (different runtime acknowledged)
- Memory: C manual → Rust ownership (safety gained, annotated)
- Annotation guidance: @Intent + @Complexity + @Risk all influence translation choices

---

## Phase 21b: Verification + Confidence (Steps 455-459)

### Step 455: Behavioral Equivalence Checking (12 tests)
- For each transpiled function, generate equivalence assertions:
  - Same inputs → same outputs (for pure functions)
  - Same side effects (for stateful functions, best-effort)
  - Same error conditions (exception/error mapping)
- Express as @Contract annotations on the transpiled function
- Confidence score: how sure are we the translation preserves behavior

### Step 456: Transpilation Confidence Scoring (12 tests)
- Score each translation 0.0–1.0 based on:
  - Direct standard library mapping (0.95+)
  - Algorithm pattern match (0.85+)
  - Structural translation with @Intent (0.70+)
  - Structural translation without @Intent (0.50)
  - Unknown pattern (0.30 — flag for human review)
- Route to review gate based on confidence threshold
- Low-confidence translations get @Review(required, human) automatically

### Step 457: Translation Report (12 tests)
- Per-file and per-function translation report:
  - Source construct → target construct mapping
  - Confidence score
  - Annotations applied/changed
  - Safety delta (gained or lost)
  - Idiomatic vs literal translation choice and why
- Project-level summary: % idiomatic, % literal, % flagged for review

### Step 458: Transpilation RPC + MCP (12 tests)
- `whetstone_transpile` — transpile a function/file with semantic awareness
- `whetstone_get_translation_report` — get translation details
- `whetstone_verify_equivalence` — generate equivalence assertions
- `whetstone_get_confidence` — confidence score for a translation
- 83+ MCP tools total

### Step 459: Phase 21b Integration + Sprint Summary (8 tests)
- Full semantic transpilation: Python project → Rust with confidence scores
- Translation report shows idiomatic choices with reasoning
- Low-confidence translations auto-flagged for review
- Equivalence assertions generated and meaningful
- Sprint 21 totals: semantic transpilation operational for all 17+ languages

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 21a | 449-454 | 68 | Intent-driven, algorithm, type, concurrency, memory translation |
| 21b | 455-459 | 56 | Equivalence checking, confidence scoring, reports, MCP tools |
| **Total** | **449-459** | **~124** | 11 steps |
