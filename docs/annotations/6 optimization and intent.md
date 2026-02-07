# Subject 6: Optimization & Intent Hints

This document defines the annotations for capturing programmer intent regarding performance, layout, and compiler behavior. These hints allow the projection engine to respect the original developer's optimizations even when moving between radically different execution environments.

## 1. Function & Execution Hints

These annotations signal how a specific block of logic should be treated by the target compiler or runtime.

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@Inline(Always|Never|Hint)`**|
|**`@Pure`**|Signals no side effects (referential transparency).|Essential for Haskell-to-imperative transpilation.|
|**`@TailCall`**|Marks a call as eligible for TCO.|Preserves recursion safety when projecting to languages without native TCO.|
|**`@Cold / @Hot`**|Profile-guided hints for branch prediction.|C++ `[[likely]]` / `[[unlikely]]` vs. profile-blind targets.|

## 2. Loop & Data Parallelism Hints

For modernizing legacy Fortran or C++, capturing the "shape" of iterative logic is key to generating efficient modern code.

### A. Loop Transformation (`@Loop`)

- **`@Loop(Unroll, N)`**: Intent to expand the loop body $N$ times.
    
- **`@Loop(Vectorize)`**: Assertion that iterations are independent (safe for SIMD).
    
- **`@Loop(Fuse)`**: Hint to combine adjacent loops for better cache locality.
    

### B. Memory Locality (`@Data`)

- **`@Data(Prefetch)`**: Intent to load data into cache before use.
    
- **`@Data(Restrict)`**: Assertion that a pointer is not aliased (C `restrict`).
    

## 3. The "Legacy Performance" Preservation

In 40 trillion lines of C++, optimizations are often "baked in" to the syntax (e.g., bit-shifting instead of multiplication).

- **Annotation:** `@Intent(Identity)`
    
- **Definition:** Identifies a pattern that is an optimization of a simpler mathematical identity.
    
- **Projection Logic:**
    
    - **Raising:** If the engine sees a bit-shift `x << 1` annotated as `@Intent(Multiply)`, it can project it as `x * 2` in a high-level language, allowing the target compiler to choose the best implementation.
        
    - **Lowering:** Injects the specific low-level trick into C/Assembly to match the original performance profile.
        

## 4. Hardware Mapping & Layout

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Target Action**|
|**`@Align(N)`**|Force memory alignment to $N$ bytes.|Project to `alignas(N)` in C++ or `__attribute__((aligned(N)))`.|
|**`@Pack`**|Remove all padding between struct members.|Maps to `#pragma pack` in C/C++.|
|**`@ConstExpr`**|Enforce compile-time evaluation.|Project to `constexpr` (C++) or `static final` (Java) if possible.|

## 5. Security & Safety Overrides

While modernizing, we often need to trade "blind speed" for safety.

- **`@BoundsCheck(Disable)`**: Used in legacy code for performance; flagged for review in safe-target projection.
    
- **`@Overflow(Wrap | Saturation | Panic)`**: Defines how to handle integer wrap-around, ensuring the target language mimics the source's arithmetic behavior.
    

**Projection Conflict:** If C++ code relies on `unsigned` wrap-around behavior and is projected to a language that panics on overflow, the engine must inject explicit modular arithmetic to remain "lossless."