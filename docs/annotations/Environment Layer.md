# Feature Request: Environment Layer (Runtime / Host Semantics)

## Summary

Add a first-class Environment Layer to the AST-first pipeline so programs can be represented and transformed with explicit runtime/host context, not just syntax and pure computation.

This is required for meaningful equivalence across languages whose semantics depend on a host runtime (browser, JVM, Python runtime, OS process model, Emacs runtime, etc.).

## Motivation

Many "languages" are inseparable from their environment:

- **JavaScript in a browser** includes event loop, microtasks/macrotasks, DOM/Web APIs, sandbox constraints.
- **Python** includes import system, module cache, object model, exceptions, refcount/GC behavior, venv packaging.
- **JVM** includes class loading, verification, reflection, JIT, GC, bytecode semantics.

If SemAnno only models AST/computation, the system will misrepresent equivalence and produce incorrect transpilation/runtime scaffolding.

## Goals

- Represent environment assumptions explicitly alongside AST/IR.
- Allow transforms/transpilation to target either:
  - a different language plus runtime scaffolding, or
  - a different environment model (e.g., "browser-like event loop" in C++).
- Provide an interface for agents to add environment-specific capabilities incrementally without rewriting core AST.

## Non-Goals

- Fully implementing a browser, JVM, or CPython.
- Building a complete OS abstraction layer.
- Perfect semantic equivalence across all edge cases (initially).
- Standardizing on any single IR format beyond "must be able to attach env info."

## Definitions

- **Environment**: The execution context providing services and semantics beyond pure expression evaluation.
- **Capability**: A named service the environment provides (I/O, timers, threads, module loading, reflection, etc.).
- **Binding Time**: Whether a name/operation resolves at compile-time, link-time, load-time, or runtime.
- **Scheduler Model**: Event loop vs threads vs fibers vs cooperative coroutines.
- **Memory Model**: Ownership/RAII, tracing GC, ref-counting, region/arena, etc.

## User Stories

- As a SemAnno user, I want to mark that a program depends on event loop semantics so a C++ target can generate appropriate scheduling code.
- As an agent generating C++ from Python-like semantics, I want to know whether imports are dynamic and how to model module caching and lookup.
- As an agent generating code for a browser target, I want to know which capabilities are required (DOM, fetch, timers) and which are forbidden (filesystem).
- As a developer debugging transpilation, I want the environment layer to explain why a transform chose threads vs event loop vs coroutines.

## Requirements

### R1. Environment Spec Object

There must be a structured "Environment Spec" attached to a program/module and accessible during transformations.

Minimum fields (names are suggestions; adapt to SemAnno conventions):

| Field | Type | Description |
|---|---|---|
| `env.id` | string | identifier, e.g. `posix_process`, `browser`, `jvm`, `python_cpython_like`, `emacs_runtime` |
| `env.version` | optional string | version of the environment spec |
| `env.capabilities` | set/list | declared required capabilities |
| `env.constraints` | set/list | forbidden or restricted behaviors |
| `env.scheduler` | enum-ish | `event_loop`, `threads`, `fibers`, `coroutines`, `single_thread` |
| `env.memory` | enum-ish | `manual`, `raii`, `refcount`, `tracing_gc`, `region` |
| `env.binding_times` | mapping | per symbol class or feature: `compile`, `link`, `load`, `runtime` |
| `env.exceptions` | model descriptor | unwind style / checked vs unchecked / typed vs untyped |
| `env.ffi` | descriptor | C ABI available? dynamic linking? host-call boundary? |

### R2. Capability Declarations

Provide a small fixed vocabulary to start (expandable):

- `io.fs`
- `io.net`
- `io.stdinout`
- `clock.time`
- `timers`
- `event_loop`
- `threads`
- `process`
- `signals`
- `modules`
- `dynamic_loading`
- `reflection`
- `gc`
- `jit`
- `ui.dom` (browser-like)
- `editor.emacs` (hosted in Emacs runtime / elisp interop)
- `host.imgui` (UI host integration)

### R3. Transform Hooks Must Receive Env

All major transforms must have read access to the environment spec:

- AST -> IR lowering
- IR -> target emission
- optimization passes (esp. those unsafe across concurrency/GC boundaries)
- runtime scaffolding generation

### R4. Environment-Aware Lowering

Certain constructs must lower differently depending on env. Examples:

| Construct | Lowering varies by... |
|---|---|
| `async/await` | event loop tasks vs threads vs coroutines |
| `imports/modules` | static linking vs dynamic loading vs runtime lookup |
| `exceptions` | unwind vs error unions vs result objects |
| `dynamic attribute access` | dictionary lookup vs vtable-like dispatch |

(Exact mapping is a later task; requirement is that the architecture supports this.)

### R5. Explicit Host Boundary Nodes (Optional but Recommended)

To avoid hidden assumptions, represent host interactions as explicit IR/AST nodes:

```
HostCall(capability="io.net", name="fetch", args=...)
ScheduleTask(queue="microtask", ...)
ModuleLoad(name=..., mode=dynamic/static, ...)
```

This enables correct transpilation/scaffolding and helps debugging.

## Implementation Notes (Low-Assumption)

- Prefer adding an annotation schema (e.g., JSON/YAML/S-expression) that can be attached to modules and referenced by UUID/path.
- The editor should surface the env spec as:
  - a module-level panel (read/write), and
  - per-node "requires capability" hints where relevant.
- Keep the initial environment models coarse:
  - "POSIX-ish native"
  - "Browser-ish event loop"
  - "Python-like dynamic runtime"
  - "JVM-like managed runtime"
  - "Emacs-hosted runtime"
- Allow partial specs; missing fields should not crash transforms (fall back to conservative defaults).

## Acceptance Criteria

- There exists a persisted Environment Spec artifact (in annotations) that is loadable with the rest of SemAnno metadata.
- A transform can query `env.scheduler` and `env.capabilities` and choose different lowering strategies.
- The editor can display and edit the Environment Spec without requiring the AST editor to be complete.
- At least one demo transformation uses env info (e.g., toggling `event_loop` vs `threads` changes generated scaffolding).

## Suggested Deliverables

1. EnvironmentSpec schema + parser/loader
2. Editor UI panel to view/edit EnvironmentSpec
3. Pass-through plumbing so all transforms receive EnvironmentSpec
4. One example backend decision driven by env (scheduler or module loading)
5. Documentation entry describing the field meanings and intended usage

## Open Questions (Intentionally Not Assumed)

- Where should env live? (program-level, module-level, per-subtree override?)
- How should capabilities be namespaced/versioned?
- What is SemAnno's canonical serialization format for annotations?
- How should "mixed environments" work (e.g., native core + embedded JS VM)?
