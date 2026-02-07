# Subject 4: Scope & Namespace Resolution

This document defines the annotations required to map identifier lookup, isolation, and visibility across disparate language environments. To ensure a lossless transition, the AST must track not just the name of a symbol, but the exact "binding site" and "reachability" rules of that symbol.

## 1. Symbol Binding & Lookup Semantics

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@Binding(Static)`**|Resolved at compile time based on text.|C, Java, Rust.|
|**`@Binding(Dynamic)`**|Resolved at runtime based on call stack.|Lisp (Emacs Lisp), Perl (local), early TeX.|
|**`@Lookup(Lexical)`**|Scope is defined by where code is written.|Most modern languages (JS, Python, C++).|
|**`@Lookup(Hoisted)`**|Declarations are moved to the top of scope.|JavaScript `var`, Python function definitions.|

## 2. Closure Captures & Environment Mapping

When a function "remembers" its environment, we must annotate how it accesses those external variables to prevent memory leaks or data races.

### A. Capture Strategies (`@Capture`)

- **`@Capture(Value)`**: A copy of the variable at the time of creation (C++ `[x]`).
    
- **`@Capture(Ref)`**: A reference to the live variable (JS, Python, C++ `[&x]`).
    
- **`@Capture(Move)`**: Transfer of ownership into the closure (Rust `move ||`).
    

**Projection Logic:** * **Into C:** The engine must generate a "Context Struct" containing all captured values/pointers and pass it as a hidden first argument to the projected function.

## 3. Visibility & Access Control

|   |   |   |   |
|---|---|---|---|
|**Annotation**|**Level**|**Target Action (into Java)**|**Target Action (into Python)**|
|**`@Visibility(Private)`**|Strictly local to the owner.|`private` keyword.|`__name` (double underscore) mangling.|
|**`@Visibility(Internal)`**|Visible within the same module/binary.|`internal` / `package-private`.|Standard naming; logic enforced via linter.|
|**`@Visibility(Friend)`**|Visible to specific other entities.|C++ `friend` class/func.|Requires interface projection or reflection.|

## 4. Namespace & Module Resolution

Handling how names are qualified and imported.

- **`@Namespace(Qual)`**: The symbol requires a prefix (e.g., `std::vector`).
    
- **`@Namespace(Flat)`**: The symbol exists in a global or current flat scope.
    
- **`@Conflict(Shadow)`**: Annotates where a local variable "hides" an outer one.
    

**Projection Logic:**

If projecting from a **Qualified** language (C++) to a **Flat** one (Global-heavy JS), the engine must "mangle" names (e.g., `std_vector`) to avoid collisions.

## 5. The "Legacy Symbol" Problem

For modernizing C++ or Fortran, we often encounter "Global State" or "Header Bloat."

- **Annotation:** `@Scope(Global_Leaked)`
    
- **Transformation:** Identify globals that are only used in one logical unit and "demote" them to `@Scope(Local)` or wrap them in a `@Singleton` class during projection to a safe language like Rust.