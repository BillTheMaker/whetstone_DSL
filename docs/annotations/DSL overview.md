# Universal AST Annotation Taxonomy for Lossless Transpilation

This document outlines the semantic metadata required to create a language-agnostic Abstract Syntax Tree (AST). These annotations bridge the "Semantic Gap" between high-level abstractions (Python, Java) and low-level imperatives (C++, Rust).

## Subject 1: Memory Management & Lifecycle (Priority Sprint)

_This section is reserved for the initial implementation phase. It will cover Allocation, Deallocation, Ownership, and Lifetime semantics._

## Subject 2: Type System & Variance

To map types across languages, we must go beyond "Int" or "String" and annotate the underlying constraints.

- **Memory Layout:** Padding, alignment requirements, and bit-width.
    
- **Mutability:** Deep vs. shallow immutability (e.g., `const` in C++ vs. `readonly` in TS).
    
- **Nullability & Optionals:** Defining how "nothingness" is handled (Pointers, Option types, or Sentinel values).
    
- **Type Variance:** Covariance and contravariance constraints for generics and collections.
    
- **Nominal vs. Structural Identity:** Does the type match by name or by shape?
    

## Subject 3: Execution Model & Concurrency

Capturing how code flows and interacts with the processor or runtime.

- **Evaluation Strategy:** Eager vs. Lazy evaluation nodes.
    
- **Concurrency Primitives:** Shared memory vs. Message passing (Actors).
    
- **Atomicity:** Annotating blocks that must be thread-safe or non-interruptible.
    
- **Async/Await Semantics:** Capturing the state machine logic of asynchronous tasks.
    
- **Exception Handling:** Distinguishing between recoverable errors and fatal panics.
    

## Subject 4: Scope & Namespace Resolution

Handling how identifiers are looked up and isolated.

- **Closure Captures:** Explicitly marking which variables are moved, copied, or referenced by a closure.
    
- **Shadowing Rules:** Defining behavior when identifiers overlap across scopes.
    
- **Visibility & Access:** Beyond `public/private`; defining "friendship" and module-level internal access.
    
- **Symbol Binding:** Static vs. Dynamic linking requirements.
    

## Subject 5: Target-Specific "Shims" (The Escape Hatch)

Handling features that cannot be expressed purely through universal semantics.

- **Inline Assembly/Intrinsics:** Preserving raw hardware instructions.
    
- **FFI Boundaries:** Annotating nodes that interact with foreign binary interfaces.
    
- **Platform-Specific Conditional Logic:** Semantic tags for OS-specific or Hardware-specific implementations.
    

## Subject 6: Optimization & Intent Hints

Metadata that guides the projection engine to produce "idiomatic" target code.

- **Inlining Hints:** Programmer intent regarding function expansion.
    
- **Vectorization Potential:** Marking loops that are safe for SIMD instructions.
    
- **Purity & Side Effects:** Annotating functions as "Pure" to allow for aggressive caching/reordering during projection.
    

## Subject 7: Meta-Programming & Macros

How the AST handles code that generates more code.

- **Expansion State:** Tracking if a node is the result of a macro expansion.
    
- **Reflection Requirements:** Marking objects that need metadata available at runtime.