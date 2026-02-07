# Subject 2: Type System & Variance

This document defines the annotations required to map disparate type systems into a universal AST. To achieve lossless transpilation, we must capture the "underlying truth" of a type—its size, its flexibility, and its identity—rather than just its high-level name.

## 1. Core Type Annotations

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@BitWidth(N)`**|Precise size of the primitive.|C `int` (platform dependent) vs. Java `int` (32-bit).|
|**`@Endian(Big|Little)`**|Byte order for multi-byte types.|
|**`@Layout(Packed|Aligned)`**|Memory padding constraints.|
|**`@Nullability(Strict|Nullable)`**|Whether the type can represent "None".|
|**`@Variance(Co|Contra|In)`**|

## 2. Identity & Structural Mapping

Languages differ on what makes two types "the same." We use these annotations to tell the projection engine how to handle comparisons and assignments.

### A. Nominal Identity (`@Identity(Nominal)`)

- **Source:** Java, C++, Swift.
    
- **Definition:** Types are different even if they look the same (e.g., `class User` vs `class Admin` with identical fields).
    
- **Projection Logic:** In TypeScript (structural), the engine must inject a "brand" or "tag" to prevent them from being interchangeable.
    

### B. Structural Identity (`@Identity(Structural)`)

- **Source:** TypeScript, Go, OCaml.
    
- **Definition:** If it walks like a duck and quacks like a duck, it is a duck.
    
- **Projection Logic:** In Java (nominal), the engine might need to generate an `Interface` that both classes implement to satisfy the compiler.
    

## 3. Immutability & Constancy

|   |   |   |   |
|---|---|---|---|
|**Annotation**|**Level**|**Target Action (into C++)**|**Target Action (into JS)**|
|**`@Mut(Shallow)`**|Top-level is immutable.|`Type * const ptr`|`Object.freeze()`|
|**`@Mut(Deep)`**|Everything reachable is immutable.|`const Type * const ptr`|Recursive `Object.freeze` or conversion to `ReadonlyArray`.|
|**`@Mut(Interior)`**|Immutable container, mutable data.|`std::cell` or `mutable` keyword.|Standard variable with access controls.|

## 4. Variance & Generics Projection

Handling how `List<String>` relates to `List<Object>` is a classic transpilation failure point.

- **Lowering (High** $\to$ **Low):** When projecting Java's wildcards (`List<? extends T>`) into C++ templates, the engine must generate specific template constraints or use `std::variant` to preserve the flexible logic.
    
- **Raising (Low** $\to$ **High):** When projecting C++'s template specialization into C#, the engine may need to use `Reflection` or generate multiple distinct classes if the generics don't map 1:1.
    

## 5. Type Erasure vs. Reification

- **`@TypeState(Erased)`**: Metadata exists only at compile time (Java/TS).
    
- **`@TypeState(Reified)`**: Metadata exists at runtime (C#/C++).
    

**Projection Conflict:** If source is `@TypeState(Reified)` (like C# `typeof(T)`) and target is `@TypeState(Erased)` (like Java), the projection engine **must** inject an extra `Class<T>` parameter into constructors/methods to carry that type information forward.