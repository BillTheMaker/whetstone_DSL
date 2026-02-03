# Sprint 1: Python ↔ C++ Dual Projection

## Sprint Goal

Enable a junior developer to write working logic in Python, a senior developer to optimize it in C++, and the junior developer to continue modifying safe areas while receiving warnings about optimization-locked regions.

---

## Scope

| In Scope | Out of Scope |
|----------|--------------|
| Python and C++ projections | Other languages |
| Tree-sitter parsing for ingestion | Custom parser implementation |
| Memory deref strategy annotations | Full SemAnno schema |
| Warning system for locked nodes | Hard locks / approval workflows |
| Basic AST nodes (functions, loops, variables, expressions) | Advanced nodes (generics, macros, templates) |
| MPS projectional editor | IDE plugins (VS Code, etc.) |

---

## User Stories

### US-1: Python Developer Writes Logic
**As a** junior Python developer
**I want to** write business logic in a Python-like projection
**So that** I can focus on correctness without worrying about memory management

### US-2: C++ Developer Optimizes
**As a** senior C++ developer
**I want to** switch to a C++ projection and add optimization annotations
**So that** the generated code is production-ready and performant

### US-3: Python Developer Sees Warnings
**As a** junior Python developer
**I want to** see warnings when I try to modify optimized nodes
**So that** I understand the impact of my changes without being blocked

### US-4: Import Existing Code
**As a** developer
**I want to** import existing Python or C++ files into the AST
**So that** I can work with legacy code in Whetstone

---

## Technical Requirements

### TR-1: Core AST Nodes

Define MPS concepts for a minimal but complete AST:

```
Module
├── Function
│   ├── name: string
│   ├── parameters: Parameter[]
│   ├── returnType: TypeReference
│   ├── body: Statement[]
│   └── annotations: SemAnno[]
├── Variable
│   ├── name: string
│   ├── type: TypeReference
│   ├── initializer: Expression?
│   └── annotations: SemAnno[]
└── annotations: SemAnno[]

Statement (abstract)
├── Assignment
├── IfStatement
├── WhileLoop
├── ForLoop
├── Return
├── ExpressionStatement
└── Block

Expression (abstract)
├── BinaryOperation
├── UnaryOperation
├── FunctionCall
├── VariableReference
├── Literal (int, float, string, bool)
├── ListLiteral
├── IndexAccess
└── MemberAccess
```

### TR-2: Memory Deref Strategy Annotation

```
DerefStrategy
├── strategy: enum {imperative, streamed, batched, content_addressed}
├── derefTime: Expression? (required if imperative)
├── derefLocation: string? (required if imperative)
└── owner: AgentReference?
```

### TR-3: Optimization Lock Annotation

```
OptimizationLock
├── lockedBy: AgentReference
├── lockReason: string
├── lockLevel: enum {warning, soft, hard}  // Sprint 1: warning only
├── affectedStrategies: string[]
└── timestamp: datetime
```

### TR-4: Language-Specific Idiom Annotation

Placeholder annotations that preserve language-specific features during import without requiring full semantic understanding.

```
LangSpecific
├── language: enum {python, cpp, rust, ...}
├── idiomType: string          // "decorator", "template", "attribute", "pragma", etc.
├── rawSyntax: string          // The original syntax as written
├── semanticHint: string?      // Optional: "memoization", "generic", "compile-hint"
└── position: enum {before, after, wrapping}  // Where it attaches to the node
```

**Examples:**

```
// Python decorator captured during import
@lang_specific(python, "decorator", "@lru_cache(maxsize=128)", hint="memoization", position=before)

// C++ template captured during import
@lang_specific(cpp, "template", "template<typename T>", hint="generic", position=before)

// C++ pragma
@lang_specific(cpp, "pragma", "#pragma omp parallel for", hint="parallelization", position=before)

// Python type hint that has no C++ equivalent
@lang_specific(python, "type_hint", "-> Generator[int, None, None]", hint="generator", position=after)

// C++ attribute
@lang_specific(cpp, "attribute", "[[nodiscard]]", hint="return_value_check", position=before)
```

**Projection Behavior:**

| In Native Projection | In Foreign Projection |
|---------------------|----------------------|
| Rendered as original syntax | Rendered as comment with semantic hint |

**Python projection of a C++ template function:**
```python
# cpp_template: template<typename T> (generic)
# NOTE: Python version uses dynamic typing instead
def compute(value):
    ...
```

**C++ projection of a Python decorated function:**
```cpp
// python_decorator: @lru_cache(maxsize=128) (memoization)
// NOTE: Implement memoization manually or use std::map cache
int compute(int value) {
    ...
}
```

### TR-5: Tree-sitter Integration

- Python grammar: `tree-sitter-python`
- C++ grammar: `tree-sitter-cpp`
- Map tree-sitter CST nodes to Whetstone AST concepts
- Preserve source locations for round-trip fidelity

### TR-6: Generators

**Python Generator:**
- Emit idiomatic Python from AST
- Ignore deref strategies (Python is GC'd)
- Emit annotations as comments or type hints where applicable

**C++ Generator:**
- Emit idiomatic C++ from AST
- Translate deref strategies to appropriate constructs
- Inject required includes and memory management code

---

## Memory Deref Strategy Examples

The following examples show the **same logical function** with different memory strategies, demonstrating how the C++ projection changes while Python remains constant.

### Example 1: Processing a List of Records

#### Whetstone AST (Canonical Form)
```
Function: processRecords
  @deref(???)  // Strategy applied here
  Parameter: records -> List<Record>
  Body:
    ForLoop:
      iterator: record in records
      body:
        Call: record.validate()
        Call: record.transform()
    Return: records
```

#### Python Projection (Same for all strategies)
```python
def process_records(records: list[Record]) -> list[Record]:
    """
    @deref: {strategy}  # Annotation visible but doesn't affect code
    """
    for record in records:
        record.validate()
        record.transform()
    return records
```

#### C++ Projection: @deref(batched) - Garbage Collected
```cpp
// Memory managed by shared_ptr reference counting
#include <memory>
#include <vector>

std::vector<std::shared_ptr<Record>> process_records(
    std::vector<std::shared_ptr<Record>> records
) {
    for (auto& record : records) {
        record->validate();
        record->transform();
    }
    return records;
}
```

#### C++ Projection: @deref(streamed) - Ownership-Based (Rust-like)
```cpp
// Ownership transferred, no copies, RAII cleanup
#include <vector>
#include <memory>

std::vector<std::unique_ptr<Record>> process_records(
    std::vector<std::unique_ptr<Record>> records  // Takes ownership
) {
    for (auto& record : records) {
        record->validate();
        record->transform();
    }
    return records;  // Transfers ownership to caller
}
```

#### C++ Projection: @deref(imperative) - Manual Control
```cpp
// Developer explicitly controls allocation and deallocation
// @deref-time: end_of_function
// @deref-location: caller_responsibility
#include <vector>

std::vector<Record*> process_records(
    std::vector<Record*> records,  // Raw pointers - caller owns
    size_t count
) {
    for (size_t i = 0; i < count; ++i) {
        records[i]->validate();
        records[i]->transform();
    }
    return records;
    // NOTE: No deallocation here - @deref-location specifies caller handles it
}
```

#### C++ Projection: @deref(content-addressed) - Immutable
```cpp
// Immutable data, identity by content hash, can be freely shared
#include <vector>
#include <functional>

struct ImmutableRecord {
    const std::string data;
    const size_t hash;

    ImmutableRecord transform() const {
        // Returns NEW record, original unchanged
        return ImmutableRecord{transformed_data, new_hash};
    }
};

std::vector<ImmutableRecord> process_records(
    const std::vector<ImmutableRecord>& records  // Immutable reference
) {
    std::vector<ImmutableRecord> results;
    results.reserve(records.size());
    for (const auto& record : records) {
        record.validate();  // Throws if invalid, doesn't mutate
        results.push_back(record.transform());  // New record
    }
    return results;
}
```

---

### Example 2: Building a Cache

#### Whetstone AST (Canonical Form)
```
Function: getOrCreate
  @deref(???)
  Parameter: cache -> Map<string, Widget>
  Parameter: key -> string
  Body:
    IfStatement:
      condition: key in cache
      then: Return cache[key]
      else:
        Assignment: widget = Widget.create(key)
        Assignment: cache[key] = widget
        Return: widget
```

#### Python Projection
```python
def get_or_create(cache: dict[str, Widget], key: str) -> Widget:
    """
    @deref: {strategy}
    """
    if key in cache:
        return cache[key]
    else:
        widget = Widget.create(key)
        cache[key] = widget
        return widget
```

#### C++ Projection: @deref(batched)
```cpp
std::shared_ptr<Widget> get_or_create(
    std::unordered_map<std::string, std::shared_ptr<Widget>>& cache,
    const std::string& key
) {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    auto widget = std::make_shared<Widget>(Widget::create(key));
    cache[key] = widget;
    return widget;
}
```

#### C++ Projection: @deref(streamed)
```cpp
// Note: Unique ownership makes caching tricky - must use reference
Widget& get_or_create(
    std::unordered_map<std::string, std::unique_ptr<Widget>>& cache,
    const std::string& key
) {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return *it->second;
    }
    auto [inserted_it, _] = cache.emplace(
        key,
        std::make_unique<Widget>(Widget::create(key))
    );
    return *inserted_it->second;
}
```

#### C++ Projection: @deref(imperative)
```cpp
// @deref-time: cache_destruction
// @deref-location: CacheManager::cleanup()
Widget* get_or_create(
    std::unordered_map<std::string, Widget*>& cache,
    const std::string& key
) {
    auto it = cache.find(key);
    if (it != cache.end()) {
        return it->second;
    }
    Widget* widget = new Widget(Widget::create(key));  // Manual allocation
    cache[key] = widget;
    return widget;
    // Caller note: Deallocation handled by CacheManager::cleanup()
}
```

---

## Warning System Specification

### Warning Triggers
A warning appears when a node has an `OptimizationLock` annotation and the current user's tier is below the lock level.

### Warning Display
```
┌─────────────────────────────────────────────────────────────┐
│ ⚠ OPTIMIZATION WARNING                                      │
├─────────────────────────────────────────────────────────────┤
│ This node was optimized by: senior_dev_alice                │
│ Optimization: @deref(imperative) with SIMD vectorization    │
│                                                             │
│ Modifying this code will:                                   │
│   • Invalidate the manual memory management strategy        │
│   • Disable the 4x SIMD optimization                        │
│                                                             │
│ [Proceed Anyway]  [View C++ Projection]  [Cancel]           │
└─────────────────────────────────────────────────────────────┘
```

### Warning Metadata
When user proceeds despite warning:
1. Original optimization is **shadowed** (preserved but inactive)
2. `@provenance` updated with modification chain
3. Notification queued for original optimizer

---

## File Structure for Sprint 1

```
languages/
├── SemAnno/
│   └── models/
│       └── SemAnno.structure.mps      # Update with new concepts
├── WhetstoneCore/                      # NEW: Core AST language
│   ├── models/
│   │   ├── WhetstoneCore.structure.mps
│   │   ├── WhetstoneCore.editor.mps
│   │   ├── WhetstoneCore.constraints.mps
│   │   └── WhetstoneCore.typesystem.mps
│   └── WhetstoneCore.mpl
├── PythonProjection/                   # NEW: Python view
│   ├── models/
│   │   ├── PythonProjection.editor.mps
│   │   └── PythonProjection.textGen.mps
│   └── PythonProjection.mpl
├── CppProjection/                      # NEW: C++ view
│   ├── models/
│   │   ├── CppProjection.editor.mps
│   │   └── CppProjection.textGen.mps
│   └── CppProjection.mpl
└── TreeSitterImport/                   # NEW: Import from source
    ├── models/
    │   ├── TreeSitterImport.behavior.mps
    │   └── TreeSitterImport.plugin.mps
    └── TreeSitterImport.mpl
```

---

## Acceptance Criteria

### AC-1: Round-Trip Parsing
- [ ] Parse Python file with tree-sitter → Whetstone AST
- [ ] Parse C++ file with tree-sitter → Whetstone AST
- [ ] Generate Python from AST that is functionally equivalent
- [ ] Generate C++ from AST that compiles and runs

### AC-2: Deref Strategy Application
- [ ] Apply `@deref(batched)` to a function → C++ uses shared_ptr
- [ ] Apply `@deref(streamed)` to a function → C++ uses unique_ptr
- [ ] Apply `@deref(imperative)` to a function → C++ uses raw pointers
- [ ] Python projection shows annotation but code unchanged

### AC-3: Warning System
- [ ] Add optimization lock to node
- [ ] Junior user attempts edit → warning appears
- [ ] Warning shows what will be invalidated
- [ ] User can proceed (warning only, not blocking)
- [ ] Provenance updated after edit

### AC-4: Dual Projection Editing
- [ ] Open AST in Python projection → edit logic
- [ ] Switch to C++ projection → same logic, different syntax
- [ ] Edit in C++ projection → Python projection updates
- [ ] Annotations visible in both projections

### AC-5: Language-Specific Idiom Preservation
- [ ] Import Python file with decorators → `@lang_specific` annotations created
- [ ] Import C++ file with templates → `@lang_specific` annotations created
- [ ] View Python decorator in C++ projection → shows as comment with hint
- [ ] View C++ template in Python projection → shows as comment with hint
- [ ] Re-export to original language → idiom syntax restored exactly
- [ ] Semantic hints populated for known patterns (lru_cache → "memoization")

---

## Implementation Order

1. **Week 1-2: Core AST Nodes**
   - Define WhetstoneCore language structure
   - Implement basic editors for each node type
   - Manual AST creation works in MPS

2. **Week 3-4: Python Projection**
   - Python-syntax editor for AST nodes
   - Python text generator
   - Manual round-trip: type Python-like → see generated .py

3. **Week 5-6: C++ Projection**
   - C++ syntax editor for AST nodes
   - C++ text generator with deref strategy translation
   - Deref strategy annotations affect generated code

4. **Week 7-8: Tree-sitter Import**
   - Integrate tree-sitter-python
   - Integrate tree-sitter-cpp
   - Parse source files → populate AST

5. **Week 9-10: Warning System**
   - OptimizationLock annotation
   - Warning UI in editor
   - Provenance tracking

---

## Open Questions

1. **Deref inference:** Should the system suggest deref strategies based on usage patterns, or always require explicit annotation?

2. **Partial optimization:** Can a senior optimize just one function while leaving others with default `@deref(batched)`?

3. **Conflict granularity:** If a junior modifies a loop inside an optimized function, does that invalidate the whole function or just the loop?

4. **Tree-sitter fidelity:** How do we handle Python/C++ features that don't map cleanly to each other (e.g., Python decorators, C++ templates)?

---

## Dependencies

| Dependency | Purpose | Source |
|------------|---------|--------|
| JetBrains MPS 2023.2+ | Language workbench | jetbrains.com/mps |
| tree-sitter | Parsing | github.com/tree-sitter |
| tree-sitter-python | Python grammar | github.com/tree-sitter/tree-sitter-python |
| tree-sitter-cpp | C++ grammar | github.com/tree-sitter/tree-sitter-cpp |

---

## Success Metrics

| Metric | Target |
|--------|--------|
| Parse success rate (Python) | >95% of valid Python files |
| Parse success rate (C++) | >90% of valid C++ files |
| Round-trip fidelity | Semantically equivalent output |
| Warning accuracy | 100% of locked nodes trigger warnings |
| Generator correctness | Generated C++ compiles without errors |
