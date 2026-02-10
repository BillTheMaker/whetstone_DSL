# Language Support Roadmap

This document proposes an order of language support additions based on
semantic fit for AST-driven workflows, ecosystem impact, and projection
complexity. It is intended for future sprint planning (not tied to Sprint 8).

## Principles
- Semantic Fit: Languages with clean, well-defined ASTs and predictable semantics.
- Ecosystem Impact: Practical value from large ecosystems and real-world usage.
- Projection Complexity: How hard it is to preserve semantics across languages.

## Proposed Phases

### Phase A: High-Impact Ecosystem Coverage
1. PHP (core language)
   - Rationale: Large installed base, web dominance, WordPress ecosystem.
   - Complexity: Medium (dynamic typing, mixed paradigms).
   - Notes: Add WordPress stubs and semantic tags after core PHP support.

2. C#
   - Rationale: Enterprise usage, strong typing, tooling clarity.
   - Complexity: Medium (runtime / reflection surface).

3. Kotlin
   - Rationale: JVM + Android, modern semantics, good tooling.
   - Complexity: Medium (interop with Java).

### Phase B: Semantic-First Languages (Best AST Fit)
4. Common Lisp / Scheme
   - Rationale: Homoiconic AST, macros, direct mapping to transformations.
   - Complexity: Medium (macro system and evaluation model).

5. OCaml / F#
   - Rationale: Algebraic data types, pattern matching, strong typing.
   - Complexity: Medium (type system mapping).

6. Haskell
   - Rationale: Pure FP, strong types; great for semantic transformations.
   - Complexity: High (typeclass system, laziness).

### Phase C: Concurrency/Logic Models
7. Erlang / Elixir
   - Rationale: Actor model, supervision trees; great for explicit execution annotations.
   - Complexity: High (message passing semantics).

8. Prolog
   - Rationale: Declarative semantics, unification; fits AST reasoning.
   - Complexity: High (search/backtracking model).

### Phase D: Long-Tail, Specialized
9. Swift
   - Rationale: ARC semantics align with annotations; Apple ecosystem.
   - Complexity: High (toolchain + platform APIs).

10. Ruby / Lua
   - Rationale: Dynamic languages with distinct semantics.
   - Complexity: Medium.

11. Dart
   - Rationale: Flutter ecosystem.
   - Complexity: Medium.

## WordPress Layer (Post-PHP)
- Add WordPress core API stubs.
- Introduce semantic tags for hooks, filters, actions.
- Provide guidance for safe/idiomatic WP patterns.

## Notes on Projection Strategy
- Use LangSpecific annotations to preserve constructs that do not map cleanly.
- Define a minimal “semantic core” for each language and expand iteratively.
- Validate with projection matrix tests as language count grows.
