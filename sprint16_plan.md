# Sprint 16 Plan: C++ Depth + Self-Hosting Phase 1

## Context

Sprints 12d gave us preprocessor, enum, namespace, and multiple inheritance.
Sprint 16 adds the remaining constructs needed to parse Whetstone's own simpler
headers: type aliases, static members, const/constexpr, smart pointers, auto
type deduction, and cast expressions (feature-requests items 6-11).

The self-hosting milestone: parse `AnnotationConflictExtended.h` (simple structs
and free functions) through our own pipeline, get a valid AST, annotate it, and
generate equivalent code.

---

## Phase 16a: C++ Type System Depth (Steps 394-399)

### Step 394: TypeAlias + Nested Type Access (12 tests)
- Parse `using json = nlohmann::json;` and `typedef vector<string> StringVec;`
- Represent nested type access: `Foo::Bar::Baz` as qualified name
- Already have TypeAlias node from Sprint 12d — deepen parser recognition

### Step 395: Static Members + Const/Constexpr (12 tests)
- MethodDeclaration gains `isConst` (method doesn't modify `this`)
- Variable gains `isConstexpr`, `isStatic` flags
- Parse `static constexpr int MAX = 256;` and `const string& getName() const;`
- Generator outputs const and constexpr qualifiers

### Step 396: Smart Pointer Patterns (12 tests)
- Recognize `unique_ptr<T>`, `shared_ptr<T>`, `make_unique<T>()` as patterns
- Map to ownership annotations: unique_ptr → @Owner(Unique), shared_ptr → @Owner(Shared_ARC)
- Parse `new`/`delete` expressions as FunctionCall nodes with @Reclaim annotations
- Cross-language: unique_ptr → Rust Box<T>, shared_ptr → Rust Arc<T>

### Step 397: Auto Type Deduction (12 tests)
- New node or annotation: `auto` as deferred type
- Parse `auto x = compute();`, `auto* p = &x;`, `const auto& ref = vec;`
- Type inference hint via @TypeState annotation when deducible from context
- Generator outputs `auto` for C++, inferred types for other languages

### Step 398: Cast Expressions (12 tests)
- CastExpression node: kind (static/dynamic/reinterpret/const), targetType, operand
- Parse `static_cast<const Foo*>(bar)` and `dynamic_cast<Derived*>(base)`
- Critical for Whetstone's own downcast-heavy validation pattern
- Annotation: @Risk(medium) on reinterpret_cast, @Risk(low) on static_cast

### Step 399: Phase 16a Integration (8 tests)
- Parse a Whetstone-style header fragment with all new constructs
- Roundtrip: parse → generate → parse → equivalent AST
- Cross-language projection for smart pointers and casts

---

## Phase 16b: Self-Hosting Phase 1 (Steps 400-404)

### Step 400: Self-Hosting Test Harness (12 tests)
- Infrastructure to feed actual Whetstone .h files through the pipeline
- Compare parsed AST against expected structure
- Track coverage: which constructs parsed vs skipped
- Graceful degradation: unparseable sections become opaque blocks, not errors

### Step 401: Parse AnnotationConflictExtended.h (12 tests)
- The simplest Whetstone header: structs + free functions
- Verify: all struct definitions found, all function signatures captured
- Expected: some template-heavy code still opaque, but basic structure correct

### Step 402: Parse AnnotationValidatorExtended.h (12 tests)
- Single class with static methods and static_cast patterns
- Verify: class structure, method signatures, static_cast recognized
- Annotation inference: @Complexity scores for validation methods

### Step 403: Annotate + Generate from Self-Hosted AST (12 tests)
- Take parsed AST from step 401-402, run inference
- Generate C++ output, compare structural similarity
- Semanno annotations on Whetstone's own code

### Step 404: Self-Hosting Progress Report + Phase Integration (8 tests)
- Coverage metrics: % of constructs in target files successfully parsed
- Gap analysis: what still can't parse (for Sprint 22)
- Sprint 16 totals verification

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 16a | 394-399 | 68 | Type aliases, const/constexpr, smart pointers, auto, casts |
| 16b | 400-404 | 56 | Self-hosting harness, parse own headers, annotate, generate |
| **Total** | **394-404** | **~124** | 11 steps |
