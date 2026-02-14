# Sprint 14 Plan: Language Batch 1 — C, WebAssembly, Common Lisp, Scheme

## Context

Universal transpilation requires universal language coverage. Sprint 14 adds four
languages chosen for strategic reasons:

- **C** — the lingua franca of legacy code. Billions of lines of C exist that need
  modernization. C is also the bridge to assembly, OS kernels, and embedded systems.
  Critical for the legacy ingestion pipeline (Sprint 20).
- **WebAssembly (WAT)** — the universal compilation target for the web. If Whetstone
  can generate WASM text format, any Whetstone-managed project can target the browser.
  Also relevant for plugin systems and sandboxed execution.
- **Common Lisp** — the most powerful macro system in any language. Representing
  Lisp's homoiconic code-as-data in the AST tests whether our universal AST is truly
  universal. Also covers the Lisp family for AI/research codebases.
- **Scheme** — minimal Lisp dialect, heavily used in teaching and language research.
  Shares core concepts with Common Lisp but with different conventions (hygiene,
  continuations, tail-call guarantees). Tests that our Lisp representation isn't
  just "Common Lisp with a different name."

Each language follows the established pattern: parser (regex-based, no new tree-sitter
dependency), generator, pipeline integration, cross-language projection tests.

**After Sprint 14:** 14 language parsers and generators.

---

## Phase 14a: C Parser + Generator (Steps 361-366)

*C is structurally close to C++ but simpler — no classes, no templates, no namespaces
(pre-C23). The parser can share infrastructure with CppParser but must handle C-specific
constructs: raw pointers everywhere, malloc/free, function pointers, preprocessor-heavy
idioms, and typedef structs.*

### Step 361: C Parser — Functions, Structs, Enums (12 tests)
**Goal:** Parse C source into Whetstone AST.

**New file: `editor/src/ast/CParser.h`** (~450 lines)
- `parseC(source) -> unique_ptr<Module>`
- `parseCWithDiagnostics(source) -> pair<Module, vector<ParseDiagnostic>>`

**Constructs parsed:**
- `int foo(int x, char* s) { ... }` → Function
- `struct Point { int x; int y; };` → ClassDeclaration (isStruct=true)
- `typedef struct { ... } Name;` → ClassDeclaration + TypeAlias
- `enum Color { RED, GREEN, BLUE };` → EnumDeclaration
- `enum { FLAG_A = 1, FLAG_B = 2 };` → anonymous EnumDeclaration
- Variable declarations with type: `int x = 5;`, `char* name;`
- `#include`, `#define`, `#pragma` → preprocessor nodes (from Sprint 12d)
- `static`, `extern`, `const` qualifiers on functions and variables
- Function pointer types: `void (*callback)(int)` → typed parameter

**Differences from C++ parser:**
- No `class` keyword (only `struct`)
- No `namespace`, no `template`, no `virtual`
- `struct` requires typedef for type name in older C
- Default struct access is public (same as C++)

**Modifies: `editor/src/ast/Parser.h`** — include CParser.h
**Modifies: `editor/src/Pipeline.h`** — add "c" routing

**Tests:** function parsing, struct parsing, typedef struct, enum parsing,
preprocessor directives, static/extern qualifiers, function pointers,
variable with pointer type, multiple functions in file, backward compat
(C++ tests unaffected), C-style comments, header guard pattern.

### Step 362: C Generator (12 tests)
**Goal:** Generate idiomatic C from Whetstone AST.

**New file: `editor/src/ast/CGenerator.h`** (~400 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<CGenerator>`
- C syntax: no classes (structs only), function declarations, typedef
- Type mapping: int, float, double, char, void, size_t, pointer types
- Memory: explicit malloc/free patterns (guided by @Owner annotations)
- String handling: char* with strlen/strcpy patterns
- Array syntax: fixed-size arrays, pointer arithmetic
- `commentPrefix() -> "// "` (C99+) or `"/* "` for pre-C99

**Cross-language mapping:**
- Python class → C struct + associated functions
- Java/C++ class → C struct + function pointers (vtable pattern) for methods
- Python list → C array with size parameter
- Exceptions → error return codes + errno pattern
- Smart pointers → raw pointers with @Owner annotation to track ownership

**Modifies: `editor/src/ast/Generator.h`** — include CGenerator.h
**Modifies: `editor/src/Pipeline.h`** — add "c" generator routing
**Modifies: `editor/src/CrossLanguageProjector.h`** — C adaptations

**Tests:** function generation, struct output, typedef struct output,
enum output, preprocessor output, pointer types, malloc/free for @Owner(Manual),
cross-language: Python → C, cross-language: C++ class → C struct + functions,
Semanno annotation comments, commentPrefix, C-style string handling.

### Step 363: C Memory Annotation Mapping (12 tests)
**Goal:** C has explicit manual memory management. The annotation system should
map cleanly to C patterns and provide meaningful guidance.

**Modifies: `editor/src/MemoryStrategyInference.h`**
- C language defaults: Owner(Manual), Lifetime(Scope), Reclaim(Explicit)
- Inference rules for C:
  - Function with malloc → @Owner(Manual) + @Reclaim(Explicit)
  - Local variable → @Lifetime(Scope)
  - Function parameter pointer → @Owner(Borrowed)
  - Return pointer → @Owner(Transferred) or @Owner(Borrowed) based on convention
  - Static variable → @Lifetime(Static)

**Modifies: `editor/src/AnnotationInference.h`**
- C-specific inference patterns:
  - `goto` usage → @Complexity(high) + @Risk(medium)
  - No bounds checking on arrays → @BoundsCheck(unchecked)
  - Void pointer casts → @Risk(high) + @Ambiguity(medium)
  - Header guard → @Synthetic(guard)

**Tests:** C defaults applied, malloc → Owner(Manual), local → Lifetime(Scope),
parameter pointer → Owner(Borrowed), goto detected as complexity signal,
void pointer cast flagged, static variable lifetime, header guard pattern,
inference confidence reasonable, cross-language: C Owner(Manual) →
Rust Owner(Box) projection, C Owner(Borrowed) → Rust &T.

### Step 364: C Integration + Self-Hosting Prep (8 tests)
**Goal:** Full C pipeline and cross-language validation.

**Tests:**
1. Parse C source → generate C → roundtrip produces equivalent output
2. Cross-language: C struct → Python class with __init__
3. Cross-language: Python class → C struct + constructor function
4. Cross-language: C with manual memory → Rust with ownership annotations preserved
5. Pipeline.run() with C source + target C++ → struct becomes class
6. Annotation inference on C code: malloc, arrays, function pointers all annotated
7. Parse simplified C header from Whetstone dependencies → verify AST
8. MCP tools recognize "c" language, tools/list unchanged (language is a parameter)

---

## Phase 14b: WebAssembly Text Format (Steps 365-369)

*WASM text format (WAT) is an S-expression syntax representing a stack-based VM.
This is structurally different from all previous languages — it's lower-level than C,
stack-based rather than expression-based, and has explicit type annotations on every
value. Representing WAT in the AST proves the AST can handle compilation targets,
not just source languages.*

### Step 365: WAT Parser (12 tests)
**Goal:** Parse WebAssembly text format into Whetstone AST.

**New file: `editor/src/ast/WatParser.h`** (~400 lines)
- `parseWat(source) -> unique_ptr<Module>`
- `parseWatWithDiagnostics(source) -> pair<Module, vector<ParseDiagnostic>>`

**WAT constructs → AST mapping:**
- `(module ...)` → Module
- `(func $name (param $x i32) (result i32) ...)` → Function
  - WAT types: i32, i64, f32, f64, funcref, externref
  - Parameters have explicit types, always
  - Result type is explicit
- `(local $name i32)` → Variable
- `(export "name" (func $name))` → Export annotation
- `(import "module" "name" (func ...))` → Import node
- `(global $name (mut i32) ...)` → Variable (isGlobal=true)
- `(memory 1)` → module-level memory declaration (custom annotation)
- `(table ...)` → module-level table declaration
- Instructions as statements: `i32.add`, `local.get`, `call`, `if`, `block`, `loop`

**Design decision:** WAT instructions map to existing AST nodes where possible
(BinaryOperation for i32.add, FunctionCall for call, IfStatement for if/else).
Stack semantics are represented via annotations (@Exec(stack)) rather than
trying to model the stack in the AST.

**Modifies: `editor/src/ast/Parser.h`** — include WatParser.h
**Modifies: `editor/src/Pipeline.h`** — add "wat" / "wasm" routing

**Tests:** module parsing, function with params and result, local variables,
export declarations, import declarations, global variables, i32.add → BinaryOperation,
call → FunctionCall, if/else → IfStatement, nested blocks, empty module,
WAT-specific types preserved in annotations.

### Step 366: WAT Generator (12 tests)
**Goal:** Generate valid WAT from Whetstone AST.

**New file: `editor/src/ast/WatGenerator.h`** (~400 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<WatGenerator>`
- S-expression output format
- Type mapping to WASM types: int→i32, float→f32, double→f64, bool→i32
- Function output: `(func $name (param $x i32) (result i32) ...)`
- Expressions → stack instructions: `a + b` → `local.get $a local.get $b i32.add`
- Control flow: if/else → `(if (result i32) ... (then ...) (else ...))`
- Memory operations guided by annotations
- `commentPrefix() -> ";; "`

**Cross-language mapping:**
- Any language → WAT: flatten expressions to stack operations
- Classes → function tables + struct-in-memory layout
- Strings → memory offset + length (WAT has no native strings)
- Exceptions → trap or error code return

**Modifies: `editor/src/ast/Generator.h`** — include WatGenerator.h
**Modifies: `editor/src/Pipeline.h`** — add "wat" generator routing

**Tests:** function generation, S-expression formatting, type mapping,
arithmetic → stack instructions, if/else → WAT if, function call → call,
export declaration, import declaration, Semanno comments with ;; prefix,
cross-language: Python function → WAT function, round-trip parse → generate,
valid WAT structure (parentheses balanced).

### Step 367: WAT Annotation Mapping (12 tests)
**Goal:** Meaningful annotations for WASM's unique characteristics.

**Annotation mappings for WAT:**
- All functions → @Exec(stack) (stack-based execution model)
- Memory operations → @Owner(Manual) (WASM has linear memory, no GC by default)
- Exported functions → @Visibility(public)
- Imported functions → @Link(import, module="name")
- Table-based dispatch → @Shim(vtable)
- Memory size declarations → @Align + @Layout annotations

**Modifies: `editor/src/MemoryStrategyInference.h`** — WAT defaults
**Modifies: `editor/src/AnnotationInference.h`** — WAT-specific patterns

**Tests:** WAT function gets @Exec(stack), memory access gets @Owner(Manual),
export → @Visibility(public), import → @Link, defaults applied correctly,
inference confidence, cross-language: WAT annotations preserved when
projecting to C, annotation roundtrip through Semanno format.

### Step 368: WAT Cross-Language Projection (12 tests)
**Goal:** Validate that WAT works as both source and target in cross-language
projection scenarios.

**Tests:**
1. Python function → WAT: arithmetic expression flattened to stack ops
2. C function → WAT: direct mapping (both low-level, closest match)
3. WAT → Python: stack ops reconstructed to expression tree
4. WAT → C: near-direct mapping
5. WAT → Rust: memory annotations guide ownership
6. Java class → WAT: method table + memory layout
7. Round-trip: Python → WAT → Python produces equivalent logic
8. Memory annotations preserved across WAT ↔ C projection
9. WAT with imports → any language: imports become function declarations
10. Complex: multi-function WAT module → C output compiles conceptually
11. Semanno roundtrip on WAT source
12. Pipeline.run() with WAT source succeeds

### Step 369: Phase 14b Integration (8 tests)
**Goal:** WAT fully integrated into the transpilation pipeline.

**Tests:**
1. Parse WAT → generate WAT → output is valid WAT
2. Cross-language: C → WAT → C roundtrip preserves logic
3. 12 languages all project to WAT as target
4. WAT annotations meaningful and complete
5. Compact AST for WAT module has correct node names
6. Diagnostics work on WAT (parse errors detected)
7. Semanno sidecar save/load for WAT files
8. MCP tools handle "wat" language parameter

---

## Phase 14c: Common Lisp Parser + Generator (Steps 370-373)

*Common Lisp is homoiconic: code is data, data is code. The S-expression syntax means
the AST representation is almost 1:1 with the source. This is both simpler (no complex
parsing) and harder (macros, multiple return values, condition system, CLOS). This phase
tests whether our universal AST handles a fundamentally different programming paradigm.*

### Step 370: Common Lisp Parser (12 tests)
**Goal:** Parse Common Lisp source into Whetstone AST.

**New file: `editor/src/ast/CommonLispParser.h`** (~400 lines)
- `parseCommonLisp(source) -> unique_ptr<Module>`
- `parseCommonLispWithDiagnostics(source) -> pair<Module, vector<ParseDiagnostic>>`

**CL constructs → AST mapping:**
- `(defun name (params) body)` → Function
- `(defmethod name ((x class)) body)` → MethodDeclaration
- `(defclass name (supers) (slots))` → ClassDeclaration (CLOS)
- `(defvar *name* value)` / `(defparameter *name* value)` → Variable (isGlobal=true)
- `(let ((x 1) (y 2)) body)` → Variable declarations + scope
- `(lambda (params) body)` → LambdaExpression
- `(if test then else)` → IfStatement
- `(cond (test1 result1) ...)` → chain of IfStatements
- `(loop ...)` / `(do ...)` / `(dotimes ...)` → ForStatement variants
- `(funcall fn args)` / `(apply fn args)` → FunctionCall
- `(defmacro name (params) body)` → MacroDefinition (from Sprint 12d)
- `(quote expr)` / `'expr` → annotated expression (@Meta(quoted))
- `(values x y z)` → multi-value return (annotated with @Contract)

**Conventions handled:**
- `*earmuffs*` for dynamic/special variables
- `+constants+` for constants
- `predicate-p` naming convention
- `(declare (type fixnum x))` → type annotations
- Multiple package prefixes: `cl:defun`, `my-package:foo`

**Tests:** defun parsing, defclass parsing, lambda, let bindings, if/cond,
loop variants, defmacro, quote/quasiquote, earmuff detection, nested
S-expressions (deep nesting), mixed definitions in one file, CLOS method.

### Step 371: Common Lisp Generator (12 tests)
**Goal:** Generate idiomatic Common Lisp from Whetstone AST.

**New file: `editor/src/ast/CommonLispGenerator.h`** (~400 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<CommonLispGenerator>`
- S-expression output: proper indentation, one form per line
- Type mapping: fixnum, single-float, double-float, string, t, nil, list, vector
- Memory: garbage collected (CL has GC), @Owner defaults to Shared_GC
- Convention adherence: earmuffs for specials, `-p` for predicates
- `commentPrefix() -> ";; "`

**Cross-language mapping:**
- Python class → CLOS defclass with defmethod
- C++ class → CLOS class + condition system for exceptions
- Java interface → CLOS generic function protocol
- Python list → CL list
- Static typing → (declare ...) type annotations
- Exceptions → CL condition/restart system

**Tests:** defun generation, defclass output, lambda output, let bindings,
S-expression formatting (correct indentation), type declarations from
annotations, cross-language: Python → Common Lisp, cross-language: Java →
Common Lisp, Semanno comments with ;; prefix, round-trip parse → generate,
CLOS method dispatch, condition system from exception annotations.

### Step 372: Lisp Annotation Mapping (12 tests)
**Goal:** Map Lisp's unique features to the annotation taxonomy.

**CL-specific annotations:**
- Macros → @Meta(quoted) + @Synthetic(macro)
- Dynamic variables → @Binding(dynamic) (vs @Binding(static) for lexical)
- Multiple return values → @Contract(returns=["x", "y"])
- Tail-recursive functions → @TailCall (CL doesn't guarantee TCO but hints are useful)
- CLOS methods → @Visibility + method combination annotations
- Special declarations → @Complexity(declared-type)
- `(optimize (speed 3) (safety 0))` → @Policy(perf=critical, style=literal)

**Modifies: `editor/src/MemoryStrategyInference.h`** — CL defaults (GC)
**Modifies: `editor/src/AnnotationInference.h`** — CL-specific patterns

**Tests:** macro → @Meta(quoted), dynamic var → @Binding(dynamic), multiple
values → @Contract, tail recursion detected, CLOS class → @Visibility,
optimize declaration → @Policy mapping, GC defaults, inference confidence,
cross-language: CL @Binding(dynamic) → Python (no direct equivalent, comment),
CL @Binding(dynamic) → C++ (thread_local hint).

### Step 373: Common Lisp Integration (8 tests)
**Goal:** Full CL pipeline and cross-language validation.

**Tests:**
1. Parse CL → generate CL → roundtrip equivalent
2. Cross-language: Python → Common Lisp (class → CLOS)
3. Cross-language: Common Lisp → Python (defclass → class)
4. Cross-language: Common Lisp → Java (CLOS → class hierarchy)
5. Annotation inference on CL code: macros, dynamic vars, tail calls
6. Semanno roundtrip on CL source
7. Pipeline.run() with CL source succeeds
8. Compact AST for Lisp module has meaningful node names

---

## Phase 14d: Scheme Parser + Generator (Steps 374-377)

*Scheme shares Lisp syntax but has different conventions: hygiene, first-class
continuations, guaranteed tail calls, and a minimalist standard library. The AST
must distinguish Scheme from Common Lisp despite surface similarity.*

### Step 374: Scheme Parser (12 tests)
**Goal:** Parse Scheme source into Whetstone AST.

**New file: `editor/src/ast/SchemeParser.h`** (~350 lines)
- `parseScheme(source) -> unique_ptr<Module>`
- `parseSchemeWithDiagnostics(source) -> pair<Module, vector<ParseDiagnostic>>`

**Scheme constructs → AST mapping:**
- `(define (name params) body)` → Function
- `(define name value)` → Variable
- `(lambda (params) body)` → LambdaExpression
- `(let ((x 1)) body)` / `(let* ...)` / `(letrec ...)` → scoped Variables
- `(if test then else)` → IfStatement
- `(cond ...)` → chained IfStatements
- `(do ((var init step) ...) (test result) body)` → ForStatement
- `(call-with-current-continuation proc)` / `(call/cc proc)` → FunctionCall
  with @Exec(continuation) annotation
- `(define-syntax name ...)` → MacroDefinition with @Meta(hygienic)
- `(syntax-rules ...)` → macro body
- `(values x y)` → multi-value return
- `(begin ...)` → statement sequence

**Differences from Common Lisp parser:**
- `define` instead of `defun`/`defvar`
- No CLOS (use records or closures for objects)
- Hygienic macros (define-syntax) instead of defmacro
- Guaranteed tail call optimization
- `#t`/`#f` instead of `t`/`nil` for booleans
- `'()` for empty list instead of `nil`

**Tests:** define function, define variable, lambda, let/let*/letrec,
if/cond, call/cc recognized, define-syntax, #t/#f booleans, nested
S-expressions, mixed definitions, tail-call pattern, Scheme-specific
comment `;` handling.

### Step 375: Scheme Generator (12 tests)
**Goal:** Generate idiomatic Scheme from Whetstone AST.

**New file: `editor/src/ast/SchemeGenerator.h`** (~350 lines)
- Extends `ProjectionGenerator` and `SemannoAnnotationImpl<SchemeGenerator>`
- S-expression output with Scheme conventions
- Type mapping: minimal (Scheme is dynamically typed) — number, string, boolean,
  pair, list, vector, procedure
- Booleans: #t / #f
- Empty list: '()
- `commentPrefix() -> "; "`

**Cross-language mapping:**
- Python class → Scheme record type + procedures
- C struct → Scheme vector with accessor procedures
- Java interface → Scheme protocol (procedure signatures)
- Exceptions → Scheme `guard` / `with-exception-handler`
- Tail calls preserved where source had @TailCall
- Continuations: unique to Scheme, generated from @Exec(continuation) annotations

**Tests:** define generation, lambda output, let bindings, boolean #t/#f,
S-expression formatting, cross-language: Python → Scheme, cross-language:
Common Lisp → Scheme (closest mapping), cross-language: Scheme → Python,
Semanno comments with ; prefix, round-trip, tail-call preservation,
continuation annotations.

### Step 376: Scheme-Specific Annotations + CL Differentiation (12 tests)
**Goal:** Ensure Scheme and Common Lisp are properly differentiated in
annotations and cross-language projection.

**Scheme-specific annotations:**
- All functions checked for tail position → @TailCall where applicable
  (Scheme guarantees TCO, so @TailCall is a correctness annotation, not just a hint)
- call/cc → @Exec(continuation)
- define-syntax → @Meta(hygienic) (vs CL's @Meta(quoted) for unhygienic macros)
- Proper tail recursion → @Loop(tail-recursive) (Scheme idiom for iteration)

**CL vs Scheme differentiation:**
- CL `defmacro` → @Meta(quoted), Scheme `define-syntax` → @Meta(hygienic)
- CL `*specials*` → @Binding(dynamic), Scheme uses `make-parameter` → @Binding(parameter)
- CL CLOS → ClassDeclaration, Scheme records → ClassDeclaration (isStruct=true)
- CL condition system → @Exception(condition), Scheme guard → @Exception(guard)

**Modifies: `editor/src/MemoryStrategyInference.h`** — Scheme defaults (GC)
**Modifies: `editor/src/AnnotationInference.h`** — Scheme-specific patterns

**Tests:** tail call detected in Scheme (guarantee vs hint), call/cc annotation,
hygienic macro annotation, CL→Scheme projection preserves semantics but changes
idiom, Scheme→CL projection, both languages GC defaults, differentiated
@Meta types, @Binding types differ, @Exception types differ, cross-language
annotation fidelity.

### Step 377: Phase 14d Integration + Sprint 14 Summary (8 tests)
**Goal:** All 4 new languages fully integrated.

**Tests:**
1. All 14 languages parse and generate without error
2. Cross-language matrix: C ↔ WAT ↔ Common Lisp ↔ Scheme all project correctly
3. Lisp → WAT: S-expression source → stack-based target
4. C → Common Lisp: manual memory → GC with annotation translation
5. Semanno format works for all 4 new languages with correct comment prefixes
6. Annotation inference produces language-appropriate defaults for each new language
7. Pipeline.run() succeeds for all 4 new languages
8. Sprint 14 totals: 14 parsers, 14 generators, all cross-language projections valid

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 14a | 361-364 | 44 | C parser, generator, memory mapping, integration |
| 14b | 365-369 | 56 | WAT parser, generator, annotations, cross-language, integration |
| 14c | 370-373 | 44 | Common Lisp parser, generator, annotations, integration |
| 14d | 374-377 | 44 | Scheme parser, generator, differentiation, integration |
| **Total** | **361-377** | **~188** | 17 steps |

---

## Key Files

**New files:**
- `editor/src/ast/CParser.h` — C parser (regex-based)
- `editor/src/ast/CGenerator.h` — C code generator
- `editor/src/ast/WatParser.h` — WebAssembly text format parser
- `editor/src/ast/WatGenerator.h` — WAT code generator
- `editor/src/ast/CommonLispParser.h` — Common Lisp parser
- `editor/src/ast/CommonLispGenerator.h` — Common Lisp code generator
- `editor/src/ast/SchemeParser.h` — Scheme parser
- `editor/src/ast/SchemeGenerator.h` — Scheme code generator
- `editor/tests/step361_test.cpp` through `editor/tests/step377_test.cpp`

**Modified files:**
- `editor/src/ast/Parser.h` — include 4 new parsers
- `editor/src/ast/Generator.h` — include 4 new generators
- `editor/src/Pipeline.h` — routing for "c", "wat", "common-lisp", "scheme"
- `editor/src/CrossLanguageProjector.h` — adaptations for all 4 languages
- `editor/src/MemoryStrategyInference.h` — defaults for C (Manual), WAT (Manual), CL (GC), Scheme (GC)
- `editor/src/AnnotationInference.h` — language-specific inference patterns
- `editor/CMakeLists.txt` — test targets

---

## Language Design Notes

### Why regex-based parsers (no tree-sitter)?
Tree-sitter grammars exist for C and Scheme, but:
1. The architecture invariant says no new external dependencies beyond tree-sitter
2. The existing tree-sitter integration handles Python/C++/Java/Rust/Go/JS/TS
3. Adding tree-sitter grammars for 4 more languages bloats the build
4. Regex-based parsers for Kotlin/C# (Sprint 11d) proved the pattern works
5. For transpilation purposes, we need structural (not syntactic) fidelity —
   regex parsers capture the structural skeleton, which is what matters for
   the AST

### Lisp parser approach
Both Lisp parsers are S-expression readers at core:
1. Tokenize: atoms, strings, numbers, parentheses, quotes
2. Build S-expression tree
3. Walk tree, pattern-match on car of each form: `defun`→Function, `define`→Function, etc.
4. Unrecognized forms become generic FunctionCall nodes

This is simpler than regex parsing of C-like languages because S-expressions
have uniform syntax. The complexity is in semantic mapping, not parsing.

### WAT as compilation target
WAT is unique among our supported languages: it's primarily a target, not a source.
Most workflows will be "source language → WAT" rather than "WAT → source language."
The WAT→source direction is decompilation, which we support but with lower fidelity
(stack reconstruction to expression trees is lossy for complex code).
