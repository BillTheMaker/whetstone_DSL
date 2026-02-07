# Subject 5: Target-Specific Shims & Escape Hatches

This document defines the annotations required to preserve platform-specific or language-specific features that cannot be mapped to universal semantics. These "Shims" ensure that even when a feature is "inexpressible" in the target language, the intent is preserved through emulation or raw embedding.

## 1. Hardware & Platform Interop

When code interacts directly with the environment (OS, CPU, or specific hardware), we use these annotations to flag non-portable nodes.

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@Intrinsic`**|Maps to a specific CPU instruction (SIMD, AES, etc.).|Intel AVX instructions in C++ vs. high-level Go.|
|**`@Raw(LangID)`**|Literal code string in a specific language.|Inline Assembly in C or `__asm__`.|
|**`@CallingConv`**|Specific binary interface (stdcall, cdecl, fastcall).|Essential for FFI (Foreign Function Interface) stability.|
|**`@Link(Name)`**|Linkage requirement for external binary symbols.|Referencing a `.lib` or `.so` file.|

## 2. Emulation of "Inexpressible" Concepts

If the source language has a feature that the target lacks (e.g., C++ Multiple Inheritance into Java), we use these shims.

### A. Feature Emulation (`@Shim(Strategy)`)

- **`@Shim(VTable)`**: Manual virtual table generation for languages without polymorphism.
    
- **`@Shim(Trampoline)`**: Used for deep recursion in languages without Tail-Call Optimization (TCO).
    
- **`@Shim(Union_Tag)`**: Injecting a discriminator into raw C unions to make them "Safe Sum Types" in Rust/Swift.
    

### B. Pointer Arithmetic Shim (`@Pointer(Arithmetic)`)

- **Source:** C, C++, Assembly.
    
- **Target Action (into Java):** The engine must project this into a `ByteBuffer` or `Unsafe` offset calculation.
    
- **Target Action (into Rust):** Maps to an `unsafe { ptr.offset(n) }` block.
    

## 3. The "Legacy Escape Hatch"

Modernizing 40 trillion lines of code requires handling "The Unknown."

- **Annotation:** `@Opaque(Reason)`
    
- **Definition:** Identifies code that the transpiler cannot safely interpret but must carry forward.
    
- **Projection Logic:**
    
    - The engine wraps the code in a "Foreign Block" (e.g., `extern "C"` in C++ or `Native` methods in Java).
        
    - It triggers a "Review Required" flag in the MPS editor, signaling that a human or specialized AI needs to provide a semantic mapping.
        

## 4. Conditional Compilation (The `#ifdef` Problem)

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Target Action**|
|**`@Target(Platform)`**|Code only valid for specific OS/Arch.|Maps to `#[cfg]` in Rust or `#if` in C#.|
|**`@Feature(Flag)`**|Code enabled by build-time flags.|Maps to build tags or conditional constants.|

## 5. Metadata Preservation (The "Lossless" Guarantee)

- **`@Original(SourceCode)`**: Stores the original raw string of the ingested legacy code.
    
- **`@Mapping(History)`**: Tracks every transformation the node has undergone (e.g., "From C pointer to Rust Reference").
    

**Projection Conflict:** If a legacy C function uses a `void*` for generic data, and we are projecting to a strictly typed language, the `@Shim(Cast)` annotation tracks the intended type discovered through data-flow analysis, allowing the engine to "Force" a type-safe cast in the target.