# Subject 7: Meta-Programming & Homoiconicity

This document defines the annotations for code-generating-code, compile-time evaluation, and runtime introspection. In the Whetstone ecosystem, we treat the AST as a homoiconic medium where the boundary between code and data is a matter of projection policy rather than a hardcoded transformation.

## 1. Homoiconicity & Data-Code Duality

For languages like Lisp, the AST must distinguish between code intended for execution and code intended to be treated as a data structure.

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@Meta(Quoted)`**|Identifies a node as data, not for immediate execution.|Lisp `'expr` vs. standard variable reference.|
|**`@Meta(Unquoted)`**|Signals an "escape" back to execution within a quoted block.|Lisp `,expr` (comma) or `~expr`.|
|**`@Symbol(Gensym)`**|A unique, non-colliding identifier generated at compile-time.|Preventing "Macro Hygiene" issues during expansion.|
|**`@Evaluate(Phase)`**|Defines when a node should be run (Compile-time vs. Runtime).|Zig `comptime` vs. standard execution.|

## 2. Macro Systems & Expansion Options

The AST tracks the relationship between a macro "call" and its "expansion" as a set of non-destructive choices.

### A. Expansion Strategy (`@Choice`)

- **Strategy A: Preservation (Opaque):** Keep the macro call intact (if the target supports it, like Rust).
    
- **Strategy B: Static Inlining:** Inline the expanded code directly but retain the `@Expansion(Source)` metadata for auditing.
    
- **Strategy C: Dynamic Wrapper:** Wrap the expansion in a dedicated function or object to maintain a clear boundary in the target language.
    

## 3. Reflection & Introspection Policies

Instead of hardcoding a lookup table, Whetstone offers policies for how types "see" themselves at runtime.

### A. Introspection Policy (`@Policy`)

- **`@Policy(Reflect: Strip)`**: Remove all reflection metadata for maximum performance/minified size.
    
- **`@Policy(Reflect: Static_Shims)`**: Generate specific getter/setter functions for requested members only (avoids full runtime overhead).
    
- **`@Policy(Reflect: Full_Runtime)`**: Embed a comprehensive type-registry (e.g., Java-style Reflection or C# TypeInfo).
    

## 4. Templates & Generics Meta-Programming

Whetstone treats C++ templates and generic specializations as meta-programming candidates.

- **Annotation:** `@Template(Specialization)`
    
- **Modernization Options:**
    
    - **Option 1: Trait-based (Rust Style):** Project as a bounded generic with shared interfaces.
        
    - **Option 2: Monomorphization (C++ Style):** Generate distinct copies of the code for each type used.
        
    - **Option 3: Type Erasure (Java Style):** Use a single implementation with casts or Object-references.
        

## 5. Synthetic Generation & The Legacy Shim

When modernizing "string-building" code (e.g., C code using `printf` to generate script fragments):

- **Annotation:** `@Synthetic(Generator)`
    
- **Whetstone Intent:** The editor highlights these blocks as "Structural Risks." The user is offered a choice to either preserve the raw string generation or refactor the generator into an AST-native Macro that produces valid nodes rather than raw text.
    

## 6. Logic Execution & Search (Prolog Style)

- **`@Goal(Query)`**: Defines a node as a logical search.
    
- **`@Unification`**: Annotates how variables are bound during a search.
    
- **Projection Choices:** * **Candidate 1: Backtracking Engine:** Inject a recursive search algorithm.
    
    - **Candidate 2: Constraint Solver:** Map the logic to an external SMT/SAT solver library.
        
    - **Candidate 3: Iterative Search:** Flatten the goal into a series of nested loops (if the search space is finite and predictable).