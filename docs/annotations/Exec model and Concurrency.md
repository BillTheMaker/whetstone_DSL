# Subject 3: Execution Model & Concurrency

This document defines the annotations for the universal AST to capture how code executes, interacts with hardware threads, and manages asynchronous state. This is essential for converting legacy synchronous code (like old C++ or Fortran) into modern non-blocking paradigms.

## 1. Concurrency Primitives & Memory Models

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Example Conflict**|
|**`@Atomic(Consistency)`**|Defines memory visibility (SeqCst, Relaxed, Acquire/Release).|C++ `std::atomic` vs. Java `volatile`.|
|**`@Sync(Monitor|Spin|Semaphore)`**|
|**`@ThreadModel(Green|OS|Fiber)`**|
|**`@MemoryBarrier`**|Explicit hardware-level memory fencing.|Essential for legacy C/Assembly transpilation to prevent reordering.|

## 2. Asynchronous State & Control Flow

Mapping between "Blocking" and "Non-blocking" logic requires tracking the state machine of the execution.

### A. Async/Await Projection (`@Exec(Async)`)

- **Source:** JS, Python, Rust, C#.
    
- **Definition:** Functions that yield execution and resume via a task runner.
    
- **Projection Logic:**
    
    - **Into C (No runtime):** Must be projected into a manual `struct`-based state machine with function pointers.
        
    - **Into Java (Project Loom):** Maps directly to Virtual Threads.
        

### B. Reactive/Event-Driven (`@Exec(Event)`)

- **Source:** Node.js, Erlang, Prolog (backtracking).
    
- **Definition:** Execution triggered by signals or goal-matching.
    
- **Projection Logic:** In imperative languages, this requires injecting a central event loop or a "Trampoline" function to prevent stack overflow.
    

## 3. The "Legacy Sync-to-Async" Upgrade (The Karpathy Goal)

For modernizing 40 trillion lines of C++, the AST identifies "Blocking IO" and annotates it for transformation.

- **Annotation:** `@Blocking(IO | Compute)`
    
- **Transformation:** If the target is a modern safe language (Rust/Go), the projection engine replaces `@Blocking(IO)` calls with their non-blocking equivalents, wrapping the call-site in an `async` context.
    

## 4. Parallelism & Vectorization

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Target Action**|
|**`@Parallel(Data)`**|Safe for SIMD/Vectorization.|Inject `pragma omp parallel` (C++) or use `Intrinsics`.|
|**`@Parallel(Task)`**|Safe for multi-core distribution.|Project to `ForkJoinPool` (Java) or `Ray` (Python).|
|**`@Pure`**|No side effects; execution order is irrelevant.|Allows aggressive reordering and memoization.|

## 5. Error Handling & Interrupts

- **`@Trap(Signal)`**: Handling hardware-level interrupts (C/Fortran).
    
- **`@Exception(Checked | Unchecked)`**: Software-level error propagation.
    
- **`@Panic(Abort | Unwind)`**: Defining what happens during an unrecoverable state.
    

**Projection Conflict:** When projecting C++ (Exceptions) to Go (Return Values), the engine must wrap every `@Exception` call-site in an `if err != nil` block, "exploding" the single line of C++ into the multi-line Go pattern.