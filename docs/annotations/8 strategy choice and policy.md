# Subject 8: Strategic Choice & Policy

This document defines the annotations for "Decision Support." Instead of dictating a modernization strategy, the universal AST tracks multiple valid transformation paths, allowing the "pro" to select the optimal projection based on specific project constraints (e.g., Real-time requirements vs. Memory safety).

## 1. Divergent Projection Strategies

When a legacy pattern is identified, the engine generates a "Strategy Menu" rather than a single output. This menu is surfaced in the Whetstone IDE as a set of interactive choices.

|   |   |   |   |
|---|---|---|---|
|**Legacy Node**|**Strategy A: Conservative (Preservation)**|**Strategy B: Progressive (Safe)**|**Strategy C: Aggressive (Modern)**|
|**Raw Pointer**|Raw Pointer (C-style)|Smart Pointer (C++11/14)|Owned/Borrowed (Rust)|
|**Manual Loop**|Preserve exact structure|Auto-vectorize (SIMD)|Functional (map/filter)|
|**Global State**|Preserve as Global|Wrap in Singleton/Namespace|Dependency Injection|
|**Goto/Jumps**|Direct Jump mapping|Structured Loop/Match|State Machine / Continuation|

## 2. Policy Annotations (`@Policy`)

These annotations allow the engineer to set "Guardrails" for the projection engine. Policies can be applied globally, per-module, or per-node.

- **`@Policy(Strictness: High)`**: Rejects any projection that results in unsafe memory access or undefined behavior.
    
- **`@Policy(Perf: Critical)`**: Prioritizes zero-cost abstractions over safety wrappers (e.g., preferring raw pointers over `shared_ptr`).
    
- **`@Policy(Binary: Stable)`**: Ensures that the projected code maintains exact ABI compatibility with existing binary libraries.
    
- **`@Policy(Style: Idiomatic)`**: Instructs the engine to favor the target language's "best practices" even if it requires a larger semantic leap from the source.
    

## 3. Conflict & Ambiguity Resolution

In complex legacy code, the "intent" is often ambiguous. We use these annotations to flag areas requiring human intervention in the Whetstone UI.

|   |   |   |
|---|---|---|
|**Annotation**|**Semantic Purpose**|**Human Action Required**|
|**`@Ambiguity(Intent)`**|Patterns that could be either an optimization or a bug.|Choose: "Preserve Hack" or "Refactor to Identity".|
|**`@Candidate(Type)`**|A `void*` that could map to several possible types.|Select the correct semantic type from an inferred list.|
|**`@Tradeoff(Reason)`**|Highlights where safety and speed are in direct opposition.|Authorize the specific safety-violation or accept the latency.|

## 4. The "Pro-Assistant" Workflow (The Whetstone Method)

Whetstone functions as an expert advisor rather than a black-box transpiler. The workflow follows a "Sharpening" cycle:

1. **Survey:** Ingest legacy code (C++/Fortran) and tag patterns with `@Policy(TBD)`.
    
2. **Telemetry:** The engine runs data-flow analysis to show the "Cost of Safety" (e.g., "Applying `@Owner(Single)` here will add 4% latency").
    
3. **Selection:** The engineer reviews the "Strategy Menu" and commits to a choice. This creates a `@Decision` node.
    
4. **Audit:** The `@Original` metadata (Subject 5) is displayed side-by-side with the projection, allowing the pro to verify that their intent hasn't been mangled.
    

## 5. Non-Deterministic Projections

- **`@Choice(ID, Options[])`**: A meta-annotation that stores a list of possible valid AST transformations. In Whetstone, this is rendered as a "Lens" that can toggle the view between different strategies.
    
- **`@Decision(ID, Selection, Author)`**: Records which option was chosen, why it was chosen, and who made the call. This turns the codebase into a living "Modernization Log."
    

## 6. Tooling & IDE Interaction (Whetstone Intent)

The Whetstone IDE uses these annotations to provide a specialized UI:

- **"Modernization Lenses":** Toggling the editor view between the legacy "Source Intent" and the modern "Target Projection."
    
- **"Safety Heatmaps":** Visualizing nodes tagged with `@Policy(Strictness: Low)` or `@Ambiguity`.
    
- **"Homoiconic Refactoring":** Allowing the user to manipulate the AST metadata directly through a Lisp-based command layer.