# Sprint Code-Health-A: Dispatch & Duplication Elimination

> **Goal:** Eliminate the worst semantic-density killers — the copy-paste dispatch chains
> and duplicated generator methods that account for ~2,000+ lines of redundant code.
>
> **Prerequisite:** None. This sprint can run independently.
> **Estimated steps:** 12–15
> **Estimated tests:** 30–40

---

## Context

Three major duplication sites inflate the codebase:

1. **ProjectionGenerator.h `dispatchGenerate()`** — 220-line if-else chain dispatching
   ~90 annotation/node types by string comparison. Every new annotation type adds 2 lines.
2. **CompactAST.h `extractSemanticSummary()`** — 340-line if-else chain doing the same
   string-match + static_cast + JSON serialization for every annotation type.
3. **Generator `emitBody()` duplication** — identical 12-line function copy-pasted into
   RustGenerator, JavaGenerator, JavaScriptGenerator, and every other generator.

---

## Steps

### Step 1–3: Annotation Self-Registration

**Goal:** Each annotation type registers itself so dispatch chains become table lookups.

**Step 1: AnnotationRegistry infrastructure**
- Create `AnnotationRegistry.h` with:
  ```cpp
  using AnnotationFactory = std::function<ASTNode*(const std::string&)>;
  using AnnotationVisitor = std::function<std::string(const ProjectionGenerator*, const ASTNode*)>;
  using AnnotationSerializer = std::function<nlohmann::json(const ASTNode*)>;

  class AnnotationRegistry {
      static auto& visitors() { static std::unordered_map<std::string, AnnotationVisitor> m; return m; }
      static auto& serializers() { static std::unordered_map<std::string, AnnotationSerializer> m; return m; }
  public:
      static void registerVisitor(const std::string& type, AnnotationVisitor v);
      static void registerSerializer(const std::string& type, AnnotationSerializer s);
      static std::string dispatch(const ProjectionGenerator* gen, const ASTNode* node);
      static nlohmann::json serialize(const ASTNode* node);
  };
  ```
- Tests: registry lookup works, unknown type returns fallback string

**Step 2: Migrate ProjectionGenerator dispatch (Part 1 — Subjects 1–5)**
- For each annotation in Subjects 1–5 (Memory, Type System, Concurrency, Scope, Shims):
  add a static registration block in the annotation's own header
- Replace corresponding if-else branches in `dispatchGenerate()` with `AnnotationRegistry::dispatch()`
- Tests: all existing step tests for these annotations still pass, dispatch returns same results

**Step 3: Migrate ProjectionGenerator dispatch (Part 2 — Subjects 6–9 + Core + AST nodes)**
- Complete the migration for remaining subjects (Optimization, Meta, Policy, Workflow, Semantic Core)
- Migrate non-annotation AST node dispatch (ClassDeclaration, EnumDeclaration, SQL nodes, etc.)
- Delete the entire 220-line if-else chain, replace with:
  ```cpp
  std::string dispatchGenerate(const ProjectionGenerator* gen, const ASTNode* node) {
      return AnnotationRegistry::dispatch(gen, node);
  }
  ```
- Tests: full dispatch regression (all 90+ types), empty node, unknown type

### Step 4–5: CompactAST Serialization Refactor

**Step 4: Add `toCompactJson()` virtual method to annotation base**
- Add virtual `nlohmann::json toCompactJson() const { return {}; }` to `ASTNode` or annotation base
- Implement for the first 20 annotation types (Subjects 1–3)
- Tests: round-trip: serialize → deserialize → compare for each type

**Step 5: Complete CompactAST migration**
- Implement `toCompactJson()` for remaining annotation types (Subjects 4–9 + Core)
- Replace `extractSemanticSummary()` 340-line chain with:
  ```cpp
  json extractSemanticSummary(const ASTNode* node) {
      json sem;
      for (auto* a : node->getChildren("annotations")) {
          json j = a->toCompactJson();
          if (!j.empty()) sem[a->shortName()] = j;
      }
      return sem;
  }
  ```
- Tests: equivalence with old output for all annotation types, empty annotations, mixed types

### Step 6–7: Generator Base Class Cleanup

**Step 6: Extract shared generator methods to ProjectionGenerator base**
- Move `emitBody()`, `appendIndented()`, `quoteIdentifier()` to `ProjectionGenerator` base
- Remove duplicates from RustGenerator, JavaGenerator, JavaScriptGenerator,
  PythonGenerator, CppGenerator, MySQLGenerator, CommonLispGenerator, WatGenerator
- Tests: each generator produces identical output before/after for sample ASTs

**Step 7: Extract shared STUB patterns to base**
- The `"// STUB: empty AST body; user implementation required"` pattern appears in every generator
- Move to `ProjectionGenerator::emitStubBody()` with language-aware comment syntax:
  - `//` for C-family, `#` for Python, `;;` for Lisp, `--` for SQL
- Tests: STUB output correct for each language, non-empty body unchanged

### Step 8–9: LSPClient Response Handler Dedup

**Step 8: Extract JSON response parsing helpers**
- LSPClient.h has repeated `result.is_array()` / `result.is_object()` / `result.contains("items")`
  patterns for completions, symbols, definitions
- Create private helpers:
  ```cpp
  std::vector<CompletionItem> parseCompletionItems(const nlohmann::json& result);
  std::vector<DocumentSymbol> parseDocumentSymbols(const nlohmann::json& result);
  DefinitionLocation parseDefinitionLocation(const nlohmann::json& result);
  ```
- Tests: parse array form, parse object-with-items form, empty result, malformed result

**Step 9: Simplify handleResponse dispatch**
- Replace the `if (id == lastCompletionRequestId_)` chain with a request-type map:
  ```cpp
  std::unordered_map<int, std::function<void(const json&)>> pendingHandlers_;
  ```
- Each `sendRequest()` registers its handler; `handleResponse()` becomes a lookup
- Tests: concurrent requests, handler cleanup after response, unknown id ignored

### Step 10–11: Headless RPC Dispatch Cleanup

**Step 10: Audit DispatchPart1–4.h for string-match duplication**
- These files dispatch RPC method strings to handler functions
- Assess: can these use a `std::unordered_map<string, handler>` like the LSP fix?
- If yes: refactor. If the dispatch is already small (~10 per file), leave it.
- Tests: all headless RPC methods still route correctly

**Step 11: Consolidate dispatch infrastructure**
- If AnnotationRegistry and RPC dispatch both use string→function maps,
  extract a shared `StringDispatch<ReturnType>` template utility
- Tests: template works for both use cases

### Step 12: Regression & Metrics

**Step 12: Full regression + line count delta**
- Run all existing tests (615 files)
- Measure: total .h lines before vs after
- Expected savings: ~1,500–2,000 lines removed
- Document in progress.md
- Tests: no regressions, line count report

---

## Success Criteria

- [ ] No if-else chain in codebase exceeds 30 lines for type dispatch
- [ ] `emitBody()` exists in exactly one place (base class)
- [ ] CompactAST `extractSemanticSummary()` is under 20 lines
- [ ] ProjectionGenerator `dispatchGenerate()` is under 10 lines
- [ ] All 615 existing tests pass
- [ ] Net line reduction: ≥1,500 lines
