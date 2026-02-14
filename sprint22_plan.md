# Sprint 22 Plan: Language Batch 3 — Assembly + Remaining Gaps

## Context

Assembly language is the floor. If Whetstone can represent assembly, it can
represent anything. This sprint adds x86 and ARM assembly, fills remaining
C++ gaps from feature-requests.md (items 12-17), and addresses any language
coverage holes discovered during real-world use in Sprints 18-21.

**After Sprint 22:** 19+ language parsers and generators, C++ self-hosting
coverage at 90%+.

---

## Phase 22a: Assembly Language Support (Steps 460-465)

Assembly is not like other languages. There's no abstraction — every instruction
is an explicit machine operation. The AST representation must capture: registers,
instructions, addressing modes, labels, directives, and sections.

### Step 460: Assembly AST Nodes (12 tests)
- New node types (all in `editor/src/ast/AssemblyNodes.h`):
  - `AssemblyInstruction` — opcode, operands (register/immediate/memory)
  - `AssemblyLabel` — name, isGlobal
  - `AssemblyDirective` — type (.data, .text, .global, .section, .byte, .word)
  - `AssemblyRegister` — name, size (8/16/32/64-bit)
  - `AssemblyMemoryOperand` — base register, offset, index, scale
- These live inside Function nodes (a function = label + instructions + ret)
- Serialization for all new types

### Step 461: x86 Assembly Parser (12 tests)
- Intel and AT&T syntax support (mode flag)
- Parse: instructions (mov, add, sub, cmp, jmp, call, ret, push, pop),
  labels, directives (.section, .global, .data, .text), comments (;)
- Register recognition: rax/eax/ax/al, rbx, rcx, ..., rsp, rbp, rip
- Memory addressing: [rax], [rbp-8], [rax+rbx*4+8]
- Sections (.text, .data, .bss) as NamespaceDeclaration equivalents

### Step 462: ARM Assembly Parser (12 tests)
- ARM/AArch64 instruction set
- Parse: MOV, ADD, SUB, LDR, STR, B, BL, CMP, branch conditions
- Registers: r0-r15 (ARM), x0-x30 (AArch64), sp, lr, pc
- Addressing modes: [r0], [r0, #4], [r0, r1, LSL #2]
- Directives: .global, .text, .data, .word, .align

### Step 463: Assembly Generator — x86 + ARM (12 tests)
- Generate valid assembly from AST
- x86 generator: Intel syntax (default) or AT&T syntax (flag)
- ARM generator: standard ARM syntax
- Cross-architecture: x86 → ARM translation for simple instruction sequences
- `commentPrefix() -> "; "` (x86) or `"@ "` (ARM)

### Step 464: Assembly Annotation Mapping (12 tests)
- @Exec(native) on all assembly functions
- @Target(x86) or @Target(arm) — architecture annotation
- @Align from .align directives
- @Risk(high) on all assembly (inherently unsafe)
- @Complexity from instruction count and branch density
- Register pressure analysis → @Complexity annotation
- Cross-language: C function → assembly (compiler-like lowering)
- Cross-language: assembly → C (decompilation-like lifting)

### Step 465: Phase 22a Integration (8 tests)
- Parse real x86 assembly → valid AST
- Parse real ARM assembly → valid AST
- Cross-architecture: simple x86 → ARM translation
- Assembly → C lifting for simple functions
- C → assembly lowering annotations
- Annotations meaningful for assembly
- 19+ languages total
- Assembly AST round-trips through Serialization

---

## Phase 22b: C++ Remaining Gaps (Steps 466-470)

Feature-requests.md items 12-17: the constructs that come up in real C++ code
but aren't architecturally complex — they just need parser/generator support.

### Step 466: Range-Based For + Structured Bindings (12 tests)
- `for (const auto& x : container)` → RangeForStatement node
- `auto [key, value] = pair;` → StructuredBinding node
- Parse, serialize, generate for both
- Cross-language: range-for → Python for-in, Rust for-in, Java for-each

### Step 467: Exception Handling (12 tests)
- TryCatchStatement node: try block, catch clauses (each with type + variable), optional finally
- ThrowExpression node
- Noexcept specifier on functions
- Parse `try { } catch (const exception& e) { } catch (...) { }`
- Cross-language: C++ try/catch → Rust Result/match, Python try/except, Go if err

### Step 468: Operator Overloading + Friend (12 tests)
- OperatorOverload node: operator symbol, parameters, return type, isFriend
- Parse `bool operator<(const Foo&) const;`
- Parse `friend ostream& operator<<(ostream&, const Foo&);`
- Cross-language: operators → named methods in other languages (Java compareTo, Python __lt__)

### Step 469: Initializer Lists + STL Patterns (12 tests)
- Initializer list expressions: `{1, 2, 3}`
- STL container recognition: vector, map, set, string → annotated with type info
- Iterator pattern detection → @Loop annotation
- Parse `std::vector<int> v = {1, 2, 3};`

### Step 470: Phase 22b Integration + Sprint Summary (8 tests)
- C++ self-hosting coverage: parse TransformEngineExtended.h (inheritance, virtual)
- All feature-requests items 1-17 now covered
- Remaining self-hosting gaps identified (for Sprint 25 polish)
- Sprint 22 totals: 19+ languages, C++ at 90%+ coverage

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 22a | 460-465 | 68 | Assembly AST nodes, x86 parser, ARM parser, generators, annotations |
| 22b | 466-470 | 56 | Range-for, exceptions, operators, initializer lists, STL |
| **Total** | **460-470** | **~124** | 11 steps |
