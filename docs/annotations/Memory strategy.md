# Subject 1: Memory Strategy Mapping (Top 20 Languages)

This document maps the memory management strategies of the top 20 languages and defines the annotation-based "projection" strategy used when the target language does not natively support the source paradigm.

## 1. Universal Memory Paradigms

|   |   |   |
|---|---|---|
|**Paradigm**|**Description**|**Key Languages**|
|**Manual (MM)**|Explicit allocation/deallocation.|C, C++, Assembly, Zig|
|**Garbage Collection (GC)**|Automatic runtime reclamation.|Java, Python, JS, Go, C#, Ruby, PHP|
|**ARC (Automatic Ref Counting)**|Deterministic reference tracking.|Swift, Objective-C|
|**Ownership/Borrowing (OB)**|Compile-time lifetime tracking.|Rust|

## 2. Language-Specific Strategy & Projection Mapping

|   |   |   |   |   |
|---|---|---|---|---|
|**Language**|**Primary Strategy**|**Annotation for Non-Supporting Target**|**Projection Action (into Low-Level/C)**|**Projection Action (into High-Level/Java)**|
|**1. Python**|GC (Ref Counting + Cycle)|`@Reclaim(Tracing)`|Inject ref-count logic; use `free` on zero.|Treat as native object; let JVM handle.|
|**2. Java**|GC (Generational)|`@Reclaim(Tracing)`|Inject tracking hooks; require runtime GC lib.|Native mapping.|
|**3. JavaScript**|GC (Mark-and-Sweep)|`@Reclaim(Tracing)`|Transform closures into heap-allocated structs.|Map to native classes/objects.|
|**4. C++**|RAII / Manual|`@Lifetime(RAII)`|Inject `destructor` calls at scope end.|Wrap in `AutoCloseable` or `finalize`.|
|**5. C**|Manual|`@Deallocate(Explicit)`|Direct `free()` mapping.|Requires manual `.close()` or `cleaner`.|
|**6. Rust**|Ownership/Borrowing|`@Owner(Single)`|Zero-cost; inject `free` at end-of-life.|Emulate with single-reference patterns.|
|**7. TypeScript**|GC (via JS)|`@Reclaim(Tracing)`|Same as JavaScript.|Same as JavaScript.|
|**8. C#**|GC (Generational)|`@Reclaim(Tracing)`|Inject `IDisposable` pattern logic.|Native mapping.|
|**9. Go**|GC (Concurrent)|`@Reclaim(Escape)`|Perform escape analysis; inject heap vs stack.|Map to native objects.|
|**10. Swift**|ARC|`@Owner(Shared_ARC)`|Inject `increment`/`decrement` calls.|Map to native GC objects.|
|**11. PHP**|GC (Ref Counting)|`@Reclaim(Cycle)`|Inject cycle-detection logic.|Map to native objects.|
|**12. Ruby**|GC (Mark-and-Sweep)|`@Reclaim(Tracing)`|Inject tracking for all object refs.|Map to native objects.|
|**13. Kotlin**|GC (via JVM/Native)|`@Reclaim(Tracing)`|Same as Java (or ARC if Kotlin/Native).|Native mapping.|
|**14. Go**|GC|`@Reclaim(Tracing)`|See #9.|Native mapping.|
|**15. Lua**|GC (Incremental)|`@Reclaim(Tracing)`|Inject minimal state-machine GC.|Map to native objects.|
|**16. Fortran**|Static / Manual|`@Allocate(Static)`|Fixed memory buffers.|Map to static arrays.|
|**17. Assembly**|Register / Raw|`@Allocate(Register)`|Direct register mapping or stack spill.|Use local variables (JVM locals).|
|**18. Dart**|GC (Generational)|`@Reclaim(Tracing)`|Inject hook for "Isolates" memory.|Native mapping.|
|**19. Objective-C**|ARC / Manual (MRR)|`@Owner(Shared_ARC)`|See #10.|Map to native objects.|
|**20. Zig**|Manual (Explicit)|`@Allocate(Allocator)`|Pass allocator pointers to all functions.|Wrap in resource manager.|

## 3. The "Lossless" Projection Logic

When the transpiler encounters a node with a memory annotation that the target language cannot express, it applies these transformations:

### A. High-Level $\to$ Low-Level (e.g., Python to C)

- **Rule:** If `@Reclaim(Tracing)` is found, the engine must implement a **Reference Counting Shim**.
    
- **Action:** Every variable assignment is wrapped in an `INC_REF` function, and every scope exit or reassignment triggers `DEC_REF`. If count == 0, `free()` is injected.
    

### B. Low-Level $\to$ High-Level (e.g., C to Java)

- **Rule:** If `@Deallocate(Explicit)` is found, the engine must ensure the resource is not leaked by the high-level runtime's lack of "free".
    
- **Action:** The node is projected into a `try-with-resources` block or the object is wrapped in a `Cleaner` API to ensure the underlying buffer is released even if the user forgets.
    

### C. Strict $\to$ Permissive (e.g., Rust to Python)

- **Rule:** If `@Owner(Single)` is found, the engine can "relax" the constraint.
    
- **Action:** Python doesn't care about single ownership; the annotation is preserved in the AST metadata for analysis but doesn't affect the generated Python code beyond standard object creation.