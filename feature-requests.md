# Feature Requests: C++ AST & Parser Gaps

These are specific C++ language constructs that the current parser/AST cannot represent, identified by examining what would be needed to parse Whetstone's own codebase through its own pipeline. Sprint 11c covers some of these (class, template, lambda basics). The rest are candidates for Sprint 12-13.

---

## Covered by Sprint 11c (in progress)

These are already planned but noted for completeness:

- **class/struct declarations** → ClassDeclaration node (fields, methods, inheritance)
- **templates** → GenericType + TypeParameter nodes
- **virtual methods** → MethodDeclaration with isVirtual/isOverride
- **lambdas** → LambdaExpression with capture list

---

## Sprint 12 Candidates: Core C++ Constructs

### 1. Multiple & Virtual Inheritance

Whetstone uses diamond inheritance extensively:
```cpp
class KotlinGenerator : public ProjectionGenerator,
                        public virtual AnnotationVisitorExtended,
                        public SemannoAnnotationImpl<KotlinGenerator> { ... };
```

**Needed:**
- ClassDeclaration.superClasses (vector, not single superClass)
- Inheritance qualifier: `public`, `protected`, `private`
- Virtual inheritance flag per base class
- Diamond resolution representation

### 2. CRTP (Curiously Recurring Template Pattern)

```cpp
template<typename Derived>
class SemannoAnnotationImpl : public virtual AnnotationVisitorExtended { ... };
```

The AST needs to represent:
- Template class declarations (not just template functions)
- Self-referential type parameters (`class Foo : Base<Foo>`)
- CRTP as a recognizable pattern for annotation/inference

### 3. Preprocessor Directives

Whetstone headers use:
```cpp
#pragma once
#include "ast/ASTNode.h"
#define TEST(name) { ... }
#define CHECK(cond, msg) if (!(cond)) { ... }
```

**Needed:**
- Include directive nodes (path, system vs local)
- Pragma nodes
- Macro definition nodes (name, parameters, body)
- Macro expansion tracking (`@Synthetic` annotation could mark expanded nodes)

### 4. Enum Class (Scoped Enums)

```cpp
enum class DiagnosticSeverity { Error = 1, Warning = 2, Info = 3, Hint = 4 };
```

**Needed:**
- EnumDeclaration node (name, isScoped, underlyingType)
- EnumMember node (name, value)

### 5. Namespace Declarations

```cpp
namespace whetstone { ... }
using namespace std;
```

**Needed:**
- NamespaceDeclaration node (name, children)
- UsingDirective node (namespace or specific symbol)

### 6. Type Aliases & Using Declarations

```cpp
using json = nlohmann::json;
using Diagnostic = AnnotationValidator::Diagnostic;
typedef std::vector<std::string> StringVec;
```

**Needed:**
- TypeAlias node (name, targetType)
- Nested type access representation (Foo::Bar::Baz)

### 7. Static Members & Methods

```cpp
class Foo {
    static bool isPowerOf2(int v);
    static const int MAX_WIDTH = 128;
};
```

**Needed:**
- MethodDeclaration.isStatic (exists in Sprint 11c plan)
- Static data member representation
- Static method call vs instance method call distinction

### 8. Const & Constexpr

```cpp
const std::string& getMessage() const;
constexpr int MAX = 256;
static constexpr bool isPowerOf2(int v) { ... }
```

**Needed:**
- Method const qualifier (method doesn't modify `this`)
- constexpr function/variable distinction
- const reference parameter types

### 9. Smart Pointers & RAII Patterns

```cpp
std::unique_ptr<Module> ast;
std::make_unique<BitWidthAnnotation>();
auto* raw = new Function();
```

**Needed:**
- Template type instantiation in expressions (not just declarations)
- Smart pointer construction/move as recognizable patterns
- `new`/`delete` expression nodes

---

## Sprint 13 Candidates: Advanced C++

### 10. `auto` Type Deduction

```cpp
auto entry = SemannoParser::parse(emitted);
auto* a = static_cast<const BitWidthAnnotation*>(anno);
const auto& ct = anno->conceptType;
```

**Needed:**
- AutoType node (deferred type resolution)
- `auto*`, `auto&`, `const auto&` variants

### 11. static_cast / dynamic_cast / reinterpret_cast

```cpp
auto* a = static_cast<const BitWidthAnnotation*>(anno);
```

**Needed:**
- CastExpression node (kind: static/dynamic/reinterpret/const, targetType, operand)
- This is critical for Whetstone's own downcast-heavy validation pattern

### 12. Range-Based For Loops

```cpp
for (const auto& role : node->childRoles()) { ... }
for (auto* child : node->allChildren()) { ... }
```

**Needed:**
- RangeForStatement node (variable, iterable, body)
- Distinguish from C-style for loops

### 13. Structured Bindings (C++17)

```cpp
auto [key, value] = pair;
```

**Needed:**
- StructuredBinding node (names[], initializer)

### 14. std::initializer_list

```cpp
bool isOneOf(const std::string& val, std::initializer_list<const char*> options);
```

**Needed:**
- Initializer list expression node
- Brace-enclosed initializer `{"a", "b", "c"}`

### 15. Operator Overloading

```cpp
bool operator<(const DiagnosticKey& other) const;
friend std::ostream& operator<<(std::ostream& os, const Foo& f);
```

**Needed:**
- OperatorDeclaration node (operator symbol, parameters)
- Friend declaration support

### 16. Exception Handling

```cpp
try { ... } catch (const std::exception& e) { ... }
throw std::runtime_error("error");
noexcept specifier
```

**Needed:**
- TryCatchStatement node
- ThrowExpression node
- Noexcept specifier on functions

### 17. STL Container Usage Patterns

```cpp
std::vector<std::string> items;
std::map<std::string, const ASTNode*> present;
std::set<std::string> seen;
```

**Needed:**
- Generic type instantiation in variable declarations
- Container method calls as recognizable patterns (push_back, insert, find, count, etc.)
- Iterator patterns

---

## Self-Hosting Milestone

When items 1-11 are implemented, Whetstone should be able to parse its own header files through its own pipeline. This would be the ultimate integration test: annotate Whetstone's source with its own annotation taxonomy, validate it with its own validator, and generate projections of itself.

Key files to target first (simplest to most complex):
1. `AnnotationConflictExtended.h` — simple structs and free functions
2. `AnnotationValidatorExtended.h` — single class, static_cast pattern
3. `TransformEngineExtended.h` — inheritance, virtual methods
4. `SemannoAnnotationImpl.h` — CRTP, virtual inheritance (hardest)
