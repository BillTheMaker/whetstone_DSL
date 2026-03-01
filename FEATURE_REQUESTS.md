# Feature Requests

> Backlog of feature ideas to triage into future sprints (e.g., Sprint 6/7).

## Title Convention (Semantic Logging)

Feature request titles must include derivation source so provenance is explicit.

Required title prefix format:

- `[Derived:<source>] <feature title>`

Examples:

- `[Derived:DataPipeline-LoRA] Semantic Validation Gate APIs for Taskitem Promotion`
- `[Derived:UserFeedback-HiveMind] Resource Lock Constraints on TaskItems`
- `[Derived:RunLogs-NativeTools] MCP Tool-Call Completion Diagnostics`

Source tags should map to where the signal came from (run logs, user request, external project, postmortem, etc.), not just the proposed implementation area.

## Security Vulnerability Awareness (Dependencies) — IMPLEMENTED (Sprint 6, Steps 190–195)

**Goal:** Warn when a dependency has known vulnerabilities and surface safer alternatives.

**Concept:**
- Maintain a vulnerability knowledge base (local cache + optional remote sources).
- When a dependency is added/updated, show immediate warnings.
- Surface findings in the Dependencies panel and Problems list.
- Optionally block auto-upgrade to vulnerable versions.

**Potential Data Sources:**
- OSV (Open Source Vulnerabilities) API / datasets
- NVD (CVE/NVD feeds)
- GitHub Security Advisories (GHSA)
- OWASP references (for categorization)

**Candidate Data Model:**
- `VulnerabilityRecord`
  - `ecosystem` (pypi/npm/crates/maven/go/vcpkg/etc)
  - `package`
  - `affected_versions`
  - `severity`
  - `summary`
  - `references`

**UI/UX:**
- Dependencies panel: inline warning badges and �View Advisory�.
- Problems panel: security diagnostics with severity.
- Agent hints: prefer safe alternatives when available.

---

## Semantic Annotations for Library APIs — IMPLEMENTED (Sprint 6, Steps 193–194)

**Goal:** Tag library functions/types with semantic annotations (e.g., `@serialize`, `@crypto`, `@io`) so humans/agents can discover intent-driven APIs quickly.

**Concept:**
- Add annotation metadata for library symbols (by library + symbol).
- Attach annotations to `ExternalModule` / `TypeSignature` nodes.
- Use annotations to filter in Library Browser and guide agent completion.

**Candidate Storage:**
- `annotations/library_semanno.json` (or similar)
- Format: `{ library: { symbol: [annotations...] } }`

**UI/UX:**
- Library Browser: filter by annotation tag.
- Completion ranking: prioritize annotated matches for task keywords.
- Agent prompts: �Use @serialize APIs� guidance.

---

## Notes
- Treat these as separate features to schedule independently.
- Likely Sprint 6/7, after core library-aware flow is stable.

## LLM Tooling & MCP Bridge — PLANNED (Sprint 7, Steps 202–234)

**Goal:** Make the agent API easy for LLMs to use and optionally expose it via MCP.

**Status:** Full plan written in `sprint7_plan.md`. 33 steps across 6 phases:
- Phase 7a: API documentation & JSON schemas (Steps 202–206)
- Phase 7b: MCP server with tools/resources/prompts (Steps 207–213)
- Phase 7c: Synthetic trace generation for training data (Steps 214–219)
- Phase 7d: Evaluation harness for LLM tool-use accuracy (Steps 220–224)
- Phase 7e: Model-specific tool definitions — Claude, Codex, open-source (Steps 225–229)
- Phase 7f: Session recording pipeline — capture, anonymize, export (Steps 230–234)

---
## PHP Language Support (Full Pipeline) � PROPOSED

**Goal:** Add full PHP support (parse, AST, generate, project, and annotations) to Whetstone.

**Scope:**
- Tree-sitter PHP parser integration
- PHP AST mapping to SemAnno concepts
- PHP generator with annotation-aware output
- Cross-language projection to/from PHP
- Tests: parse/generate round-trip, annotation preservation, projection matrix coverage

**Notes:**
- Start with core PHP syntax (functions, classes, namespaces, arrays, exceptions, traits).
- WordPress-specific libraries on top of core PHP support.

---

## WordPress Support (Library + Semantics) � PROPOSED

**Goal:** Enable WordPress-aware tooling on top of PHP support.

**Scope:**
- Library symbol stubs for WordPress core APIs
- Semantic tags for WP concepts (hooks, filters, actions)
- Agent suggestions for safe WP patterns

---

## Rust Plugin Packaging (Conflict Resolver) � PROPOSED

**Goal:** Package a Rust-based topological conflict resolver as a Whetstone plugin.

**Scope:**
- Define plugin interface for external tools
- Packaging format and plugin discovery/loading
- Example integration with a Rust binary or shared library

## Julia Language Support (Full Pipeline) � PROPOSED

**Goal:** Add full Julia support (parse, AST, generate, project, and annotations) to Whetstone.

**Scope:**
- Tree-sitter Julia parser integration
- Julia AST mapping to SemAnno concepts
- Julia generator with annotation-aware output
- Cross-language projection to/from Julia
- Tests: parse/generate round-trip, annotation preservation, projection matrix coverage

---

## Julia ML Projection Layer � PROPOSED

**Goal:** Map common Python ML/Numerical APIs to Julia equivalents while preserving optimization intent.

**Scope:**
- API mapping table (NumPy/Pandas/Torch core calls ? Julia equivalents)
- Semantic tags for numerical/tensor operations
- Fallback interop for unmapped calls (PyCall/JuliaCall)
- Dual projections: clean surface Julia + preserved optimization annotations


## Julia Packaging Strategy � PROPOSED

**Goal:** Define a reliable packaging path for Julia-based artifacts.

**Scope:**
- Document options: runtime install, PackageCompiler.jl, embedding Julia as a library
- Provide recommended defaults for CLI tools vs GUI apps
- Include guidance for minimizing startup latency and bundle size

## [Derived:PersonalVaultTool-MultiPlatform] Platform-Agnostic Projection via SemAnno Adapter Boundaries — PROPOSED

**Filed:** 2026-02-27
**Filed by:** personal_vault_tool session (2026-02-27)
**Priority:** High — required for any real-world cross-platform projection (Python → Android/Kotlin, Python → Swift/iOS, etc.)
**Source context:** `personal_vault_tool/vault_core.py` — clean business logic and platform I/O are entangled in the same class with no annotation boundary.

### Problem

Whetstone's projector today treats a source file as a unit. When projecting `VaultStore`
to Kotlin, it faithfully projects `sqlite3.connect()` — which doesn't exist on Android.
The projector has no way to distinguish:

- **Pure logic nodes** — `share_evaluate()` selector scoring, audit hash chain computation,
  grant TTL enforcement — zero platform dependency, projectable to any target.
- **Platform adapter nodes** — `sqlite3.connect()`, `Fernet.encrypt()`, `socket.bind()` —
  completely platform-specific, must be substituted per-target, not projected literally.

This is not a generator quality issue (GR-022/023/024). It is an architectural gap:
the AST has no annotation that marks where the platform boundary is.

### The Correct Architecture (already implied by SemAnno's design goals)

```
Source file with SemAnno annotations
│
├── @pure.logic nodes     → project as-is to any language
│     share_evaluate()
│     audit_hash_chain()
│     grant_scope_check()
│
└── @platform.storage     → generate abstract interface + per-target impl
│     _db() / read_records() / write_record()     → Python: sqlite3
│                                                  → Android: Room DAO
│                                                  → iOS: CoreData
│
├── @platform.crypto      → generate abstract interface + per-target impl
│     FernetEncryptor                              → Python: cryptography.Fernet
│                                                  → Android: Android Keystore AES
│
└── @platform.network     → omit on mobile (caller drives transport)
      ThreadingHTTPServer
```

The platform adapters become **generated abstract interfaces** with per-target
implementations, not projected concrete calls.

### Feature Request A: SemAnno Platform Boundary Annotations

**New annotation tags:**
- `@pure.logic` — no I/O, no platform imports; freely projectable
- `@platform.storage` — interacts with a persistence layer; requires adapter
- `@platform.crypto` — interacts with a key/encryption service; requires adapter
- `@platform.network` — interacts with sockets/HTTP; usually omitted on mobile
- `@platform.os` — interacts with filesystem, env vars, signals

**Generator behavior:** annotated nodes are flagged before projection; the projector
emits an `AdapterInterface` node instead of projecting the implementation.

**Validation gate:** `whetstone_validate_taskitem` or a new `whetstone_check_platform_purity`
tool should verify that `@pure.logic` nodes have zero `@platform.*` imports in their
transitive dependency graph.

### Feature Request B: Platform Profile System

**New concept:** a named platform profile describes what substitutions exist for each
`@platform.*` category.

```json
{
  "profile": "android-kotlin",
  "substitutions": {
    "platform.storage": "androidx.room.RoomDatabase",
    "platform.crypto":  "android.security.keystore.KeyGenParameterSpec",
    "platform.network": null
  }
}
```

Built-in profiles to start:
- `python-stdlib` (current, implicit)
- `android-kotlin` (Room + Android Keystore)
- `swift-ios` (CoreData + CryptoKit)
- `node-js` (better-sqlite3 + node:crypto)

**Generator behavior:** when projecting with a profile active, `@platform.*` nodes
are replaced by the profile's adapter interface stub rather than the source
implementation. The developer fills the stub; the logic layer is complete.

### Feature Request C: Multi-Target Projection Command

**Tool:** extend `whetstone_run_pipeline` or add `whetstone_project_multiplatform`

**Input:** annotated source file + list of target profiles
**Output:** one generated artifact per target, plus a shared pure-logic module
that all targets import unchanged.

**Acceptance criteria:**
- `personal_vault_tool/vault_core.py` (fully annotated) projects to:
  - `out/python/vault_logic.py` + `out/python/adapters/sqlite_adapter.py`
  - `out/kotlin/VaultLogic.kt` + `out/kotlin/adapters/RoomAdapter.kt` (stub)
- All `@pure.logic` methods are byte-identical in content between targets (modulo syntax).
- `@platform.*` methods emit only the interface signature + a `TODO` body in each target.

### Relationship to existing gaps

| Gap | Relationship |
|-----|-------------|
| GR-006 | Complex class generation — same root; multi-method class projection needed for VaultStore |
| GR-007 | Cross-language class node emission — must be closed before this works |
| GR-022/023/024 | Python generator string/import/comment bugs — must fix before Python→Python round-trip is clean enough to annotate |

### Suggested sprint scope

- Sprint N: SemAnno tags `@pure.logic`, `@platform.storage`, `@platform.crypto` (schema + parser + editor UI)
- Sprint N+1: Platform profile schema + substitution registry (2 built-in profiles: python-stdlib, android-kotlin)
- Sprint N+2: Projection gate — `whetstone_check_platform_purity` tool
- Sprint N+3: Multi-target pipeline output with stub generation

---

## Python/C++/JS-TS Support Enhancements � PROPOSED

**Goal:** Expand first-class support for Python, C++, and JS/TS beyond current parse/generate.

**Scope:**
- Improve language-specific projection fidelity and diagnostics
- Expand standard library and ecosystem stubs (pip/npm/vcpkg)
- Add language-specific refactor recipes and annotation guidance
- Strengthen test corpus and projection round-trip coverage

---

## Plugin System Enhancements � PROPOSED

**Goal:** Expand plugin support for external tools and language packs.

**Scope:**
- Standardize plugin packaging format
- Document plugin API and lifecycle hooks
- Add plugin discovery and version compatibility checks
- Provide example plugins for language packs and external analyzers

---

## Resource Lock Constraints on TaskItems — SPRINT 46 CANDIDATE

**Filed by:** HiveMind project agent (2026-02-21)
**Priority:** High — blocks correct parallel scheduling
**Source spec:** `hivemind/docs/WHETSTONE_FEATURE_REQUEST_RESOURCE_LOCKS.md`
**Components:** `AnnotatedTaskitem` schema, `whetstone_generate_taskitems`, `whetstone_queue_ready`, `whetstone_validate_taskitem`

**Problem:**
`dependencyTaskIds` models causal ordering (DAG edges). It does not model mutual
exclusion — two tasks that cannot run concurrently because they share a resource,
even though neither depends on the other's output. Without this, the scheduler must
either serialize everything or rely on humans to catch conflicts. Both fail at scale.

**Proposed schema addition:**
```typescript
interface AnnotatedTaskitem {
  // ... existing fields ...

  // NEW
  resourceLocks?: string[];  // Named resources held exclusively during execution.
                             // Tasks with overlapping locks are serialized by scheduler.
                             // Node-local by default. Prefix "global:" for swarm-wide mutex.
                             // Examples: "drone-process", "sqlite-db", "file:/path/to/f",
                             //           "global:nexus-jobs-queue"
}
```

**Key distinction:**
| Field | Meaning | Relationship |
|-------|---------|--------------|
| `dependencyTaskIds: [A]` | A must finish before I start | Directed — A → B |
| `resourceLocks: ["db"]` | I need exclusive access to "db" while running | Undirected — A ↔ B |

**`exclusionGroup` as sugar:** `exclusionGroup: "X"` expands to `resourceLocks: ["group:X"]`.
One model, two syntaxes.

**Lock scope convention (addition to original spec):**
All locks are node-local by default. Prefix `global:` for swarm-wide mutual exclusion.
HiveMind's scheduler knows the dispatch target node and enforces accordingly.

**Sprint 46 plan (5 steps):**
1. Schema: add `resourceLocks?: string[]` to `AnnotatedTaskitem`
2. Generator: infer locks in `TaskitemGeneratorV2` from spec signals ("both tests spin up a drone" → `drone-process`, "shared SQLite DB" → `sqlite-db`, "same port" → `nexus-port-N`)
3. MCP: surface locks in `whetstone_generate_taskitems` output
4. Queue: add `resource_conflict_unresolved` warning class to `whetstone_queue_ready` (warn on serialization, don't block)
5. Validation: `whetstone_validate_taskitem` checks lock name format, flags node-local vs global ambiguity

**Precedent:** GNU Make `.NOTPARALLEL`, Bazel `tags=["exclusive"]`, GitHub Actions `concurrency:`, Kubernetes resource quotas, POSIX `flock(2)`.

---

## Deterministic MCP Debugging Workflow for SLM Agents — PROPOSED

## [Derived:DataPipeline-LoRA] Semantic + Recursive Taskitem Validation APIs — PROPOSED

**Goal:** Expose first-class MCP/editor APIs to score task completion beyond test-pass and feed recursive quality signals back into taskitem generation.

**Why derived:** Current dataset curation needs semantic validity checks and recursive re-validation loops to avoid local minima in training data.

**Scope:**
- Semantic completion validator API (invariant checks, intent/diff consistency, equivalence hooks).
- Recursive taskitem quality scorer (self-containment, dependency correctness, blocker detectability).
- Promotion-grade result packet (`execution_pass`, `semantic_pass`, `recursive_pass`, confidence).
- MCP endpoints for batch scoring and audit export.

**Data-pipeline impact:**
- Reduces script-only heuristics.
- Enables consistent promotion gates across projects.
- Improves good/bad labeling quality for LoRA datasets.

**Filed by:** Whetstone execution session (2026-02-21)
**Priority:** Critical for self-improving low-context agents
**Goal:** Make generation + debugging robust enough that small/local models can complete tasks without expert intuition.

### Why this matters

Current flow is strong for generation and validation, but debugging still relies heavily on manual compile/test interpretation. Strong agents can compensate; weaker agents cannot. The system needs deterministic failure triage and patch guidance, not just raw logs.

### Feature Request A: Structured Failure Packet Tool

**Tool name:** `whetstone_capture_failure_packet`

**Problem:**
Compile/test output is noisy and non-deterministic across environments. SLMs need compact, stable failure packets.

**Requirements:**
- Input: command, cwd, optional target (`step707_test`, etc.)
- Output packet fields:
  - `failureClass` (`compile_error`, `test_assertion`, `schema_error`, `tool_contract_error`, `runtime_error`)
  - `primaryFile`, `primaryLine`, `primarySymbol`
  - `normalizedMessage` (deduped, canonicalized)
  - `reproCommand`
  - `firstFailingTest` (if test run)
  - `confidence`
- Deterministic normalization rules (same stderr -> same packet).

**Acceptance criteria:**
- Same failing command produces byte-stable JSON packet across repeated runs.
- Packet size bounded (e.g., <= 8 KB) with deterministic truncation.

### Feature Request B: Root-Cause Clustering Tool

**Tool name:** `whetstone_cluster_failures`

**Problem:**
One broken include can yield dozens of compiler errors. Small models need one root cause, not 50 symptoms.

**Requirements:**
- Group errors by root cause signature (`missing_symbol`, `type_mismatch`, `include_cycle`, `schema_shape_mismatch`).
- Return ordered clusters with:
  - `clusterId`
  - `rootCauseCandidate`
  - `blastRadius`
  - `fixFirst` boolean
- Deterministic ordering by severity + source location.

**Acceptance criteria:**
- At least 80% reduction in symptom-level errors shown to agent for common compile breaks.

### Feature Request C: Patch Proposal Tool

**Tool name:** `whetstone_propose_patch_for_failure`

**Problem:**
Weak agents can identify a failure but cannot reliably synthesize a minimal fix.

**Requirements:**
- Input: failure packet + repository context slice.
- Output:
  - `patchPlan` (ordered edit intents)
  - `candidatePatch` (unified diff)
  - `riskLevel`
  - `expectedTestsToRun`
- Must include explicit invariants ("do not alter unrelated files", "preserve JSON schema keys").

**Acceptance criteria:**
- Patch can be applied automatically in dry-run mode.
- For known failure fixtures, first proposal passes targeted test in >=60% cases.

### Feature Request D: Deterministic Debug Loop Orchestrator

**Tool name:** `whetstone_debug_until_green`

**Problem:**
Current workflows require manual sequencing: run test, parse output, patch, rerun.

**Requirements:**
- Single loop with explicit phases:
  1. reproduce
  2. capture failure packet
  3. cluster root cause
  4. propose patch
  5. apply patch (optional gated)
  6. rerun targeted tests
  7. emit outcome report
- Hard iteration cap (`maxIterations`) and stop reasons.
- Full state machine emitted as JSON transitions.

**Acceptance criteria:**
- Re-running with same seed/state yields same action sequence.
- No hidden state outside packet history.

### Feature Request E: Repro Packet Archive & Replay

**Tool names:** `whetstone_save_repro_packet`, `whetstone_replay_repro_packet`

**Problem:**
Debugging quality is hard to improve without stable training/eval fixtures.

**Requirements:**
- Save packet: failing command, env snapshot, file hashes, failure packet, patch attempt history.
- Replay packet in sandbox and verify same failure class before patching.
- Export format compatible with LoRA/SLM fine-tuning datasets.

**Acceptance criteria:**
- Replay reproduces same `failureClass` and `primaryFile` for >=95% captured packets.

### Feature Request F: Minimal Context Assembler for Fixing

**Tool name:** `whetstone_assemble_fix_context`

**Problem:**
Small models degrade when given broad context.

**Requirements:**
- Input: failure packet/cluster.
- Output: minimal file slices needed to fix:
  - failing file span
  - relevant headers
  - nearest tests
  - related schema/tool contract
- Token budget modes: `tiny`, `small`, `medium`.

**Acceptance criteria:**
- Produces <=2k token context in `tiny` mode while preserving fix success on benchmark fixtures.

### Feature Request G: Regression Guard Tool

**Tool name:** `whetstone_regression_guard`

**Problem:**
A fix may pass one test but silently break prior working behavior.

**Requirements:**
- Given touched files + step ID, compute deterministic smoke set:
  - failing target
  - neighboring step tests
  - MCP tool registration checks
- Emit `mustPass`, `optional`, and `deferred` buckets.

**Acceptance criteria:**
- Any patch merged by autonomous loop includes a guard report with pass/fail matrix.

### Feature Request H: Debugging Metrics for SLM Readiness

**Metric packet fields:**
- `timeToFirstGreen`
- `iterationsToGreen`
- `tokenCostPerFix`
- `symptomToRootCauseCompression`
- `patchAcceptanceRate`
- `regressionEscapeRate`

**Requirement:**
- Metrics available per run and aggregated by failure class.
- Used to decide when to switch work from large model -> SLM.

### Proposed rollout

1. Phase 1: Failure packet + clustering (`A`, `B`)
2. Phase 2: Patch proposal + fix context (`C`, `F`)
3. Phase 3: Loop orchestrator + regression guard (`D`, `G`)
4. Phase 4: Replay archive + readiness metrics (`E`, `H`)

### Done definition for “SLM-ready debugging”

- A small model can take a single failing test target and autonomously produce a patch that passes:
  - target test,
  - deterministic regression guard set,
  - MCP tool schema checks,
  - with full repro packet trace and no manual intervention.

---

## Polyglot Orchestrator — PROPOSED

> Origin: design session 2026-02-28. Goal: true lossless polyglot code generation —
> every component of a project written in the language best suited to it, with
> type-safe boundaries, unified editing (LSP), and unified debugging (DAP),
> all driven from a single shared AST.
>
> This is a multi-phase capability that extends Whetstone from a single-target
> code generator into a polyglot orchestration platform. The theoretical proof
> is a project where every supported generation language participates, each
> responsible for the component it is most fit for, with no manual FFI code.

### Feature Request I: Language Fitness Scorer

**Tool name:** `whetstone_score_language_fitness`

**Derived:** `[Derived:DesignSession-Polyglot] Language Fitness Scorer`

**Problem:**
Language selection today is implicit — the caller picks a target language. There is no
mechanism to analyze an AST subtree and recommend which language best fits its
computational shape. This means the caller must already know the answer, which defeats
the purpose of language-agnostic AST representation.

**Requirements:**
- Extract structural features from an AST node or spec section:
  - mutation ratio (writes/reads per node)
  - recursion shape (tail-recursive, tree-recursive, flat loop)
  - concurrency primitives present (channels, shared memory, actors)
  - memory lifetime pattern (arena, GC, borrow-shaped)
  - type complexity (simple generics, dependent types, none)
  - I/O pattern (blocking, async, event-driven)
- Score each supported generation language against a language idiom profile.
- Return ranked list with per-language score and feature explanation.

**Acceptance criteria:**
- Given an AST node representing a data-parallel reduction loop, scores Julia/Rust
  above Python/Lisp.
- Given an AST node representing a supervision tree with restart policies, scores
  Erlang/Elixir above C++/Go.
- Ranking rationale is human-readable (per-feature delta from ideal profile).
- Tool runs in < 50ms on AST subtrees up to 500 nodes.

---

### Feature Request J: Polyglot FFI Glue Generator

**Tool name:** `whetstone_generate_ffi_glue`

**Derived:** `[Derived:DesignSession-Polyglot] Polyglot FFI Glue Generator`

**Problem:**
When a project uses multiple languages, the boundary between them (FFI, C ABI,
ctypes, JNI, pybind11, etc.) is written by hand. This is the primary source of
polyglot integration failures — type mismatches, lifetime errors, and ABI drift
are all caught only at runtime. The shared AST already encodes the type information
needed to generate these boundaries correctly.

**Requirements:**
- Given an AST boundary node (a function or struct visible to two language targets):
  - Emit the correct binding on both sides (e.g. Rust `extern "C"` + Python ctypes struct)
  - Emit a C ABI header as the neutral intermediary when needed
  - Annotate the boundary with source AST node ID for cross-language debug symbol linking
- Support at minimum: Rust↔Python, Go↔C++, C++↔JavaScript (N-API), Rust↔Go (CGo)
- Generated glue must compile and pass a round-trip type check

**Acceptance criteria:**
- A Rust function callable from Python via generated ctypes binding, where the
  binding was produced entirely from the shared AST — no hand-written glue.
- Round-trip test: Python calls Rust, Rust returns value, Python receives correct type.
- Boundary annotations present in generated DWARF (for DAP orchestrator).

---

### Feature Request K: Cross-Language Symbol Index Emitter

**Tool name:** `whetstone_emit_symbol_index`

**Derived:** `[Derived:DesignSession-Polyglot] Cross-Language Symbol Index`

**Problem:**
LSPs are language-specific and share no state. Cross-language goto-definition,
rename, and find-references are broken in all polyglot projects. The shared AST
already contains the symbol graph — it just isn't being emitted in a queryable format.

**Requirements:**
- Emit a SCIP-format (Sourcegraph Code Intelligence Protocol) symbol index from
  the shared AST after multi-language generation.
- Index must include:
  - Definition sites (per-language file + line)
  - Reference sites across all generated languages
  - Type information at language boundaries
  - Cross-language "same symbol" links (Rust `my_fn` ↔ Python `my_fn` ↔ C ABI `my_fn`)
- Index is updated incrementally on re-generation of any language target.

**Acceptance criteria:**
- A symbol defined in Rust and called from Python: goto-definition from Python
  resolves to Rust source file and line.
- Rename in the shared AST propagates to all language targets and updates index.
- Index generation adds < 200ms to a full polyglot generation run.

---

### Feature Request L: LSP Orchestration Layer

**Tool name:** (editor integration, no standalone MCP tool)

**Derived:** `[Derived:DesignSession-Polyglot] LSP Orchestrator`

**Problem:**
Editors run one LSP per language. Cross-language queries (goto-definition, hover,
rename) are silently broken at language seams. There is no mechanism to route an
LSP request through the shared AST when the answer crosses a language boundary.

**Requirements:**
- A thin LSP proxy that sits between the editor and all per-language LSP servers.
- Intercepts requests at known cross-language boundary symbols (identified via symbol index).
- For intra-language requests: forwards directly to the appropriate language server.
- For cross-language requests: resolves via the SCIP symbol index and returns a
  synthesized response to the editor.
- Supports: textDocument/definition, textDocument/references, textDocument/rename,
  textDocument/hover (shows both-side type information at boundaries).

**Acceptance criteria:**
- Goto-definition from a Python call site lands on Rust function definition.
- Rename of a boundary symbol in the orchestrator propagates rename requests to
  both the Rust LSP and the Python LSP simultaneously.
- Hover at a Rust↔Python boundary shows the C ABI type as well as both native types.

---

### Feature Request M: DAP Orchestration Layer

**Tool name:** (debugger integration, no standalone MCP tool)

**Derived:** `[Derived:DesignSession-Polyglot] DAP Orchestrator`

**Problem:**
Debuggers are language-specific. A polyglot stack trace (Rust → Go → Lisp) is
unreadable — native frames, goroutine scheduler frames, and Lisp continuations
are interleaved with no cross-language annotation. Stepping across a language
boundary is not possible.

**Requirements:**
- A DAP proxy sitting above per-language debug adapters (CodeLLDB for Rust,
  Delve for Go, etc.).
- Uses DWARF boundary annotations (emitted by FFI Glue Generator) to identify
  language seam frames in a mixed stack.
- Presents a unified, annotated stack trace: each frame labeled with its origin
  language, AST node ID, and source location.
- Supports cross-language step-over: stepping past a boundary call executes
  the foreign call and pauses at the next source line in the calling language.
- Breakpoints set by AST node ID fire in all generated language targets
  simultaneously.

**Acceptance criteria:**
- Single debugger session, stack trace shows Rust frames and Python frames
  correctly interleaved and labeled.
- Step-over at a Rust→Python call boundary pauses at the next Rust line
  (Python call executes atomically).
- Breakpoint on AST boundary node fires when either the Rust or Python side
  of the call is entered.

---

### Feature Request N: Polyglot Project Test Harness

**Tool name:** `whetstone_run_polyglot_suite`

**Derived:** `[Derived:DesignSession-Polyglot] Polyglot Test Harness`

**Problem:**
There is no systematic way to verify that a multi-language generation round-trip
is semantically correct — that the same intent expressed in N languages produces
equivalent observable behavior. Without this, "lossless polyglot transpiling"
is a claim, not a proof.

**Requirements:**
- Given a spec and a set of target languages, generate all language variants.
- Run each variant against the same black-box test suite (input/output pairs).
- Compare outputs across all variants; report any semantic divergence.
- Track parity over time: if adding a new language breaks an existing variant's
  parity, block the generation.

**Acceptance criteria:**
- A sorting algorithm spec generates Python, Rust, Go, and Haskell variants.
- All four variants produce identical output for the same 1000 random inputs.
- Introducing a deliberate bug in the Rust generator causes the harness to flag
  Rust as divergent without affecting the Python/Go/Haskell parity report.

---

### Rollout Plan (Polyglot Orchestrator)

1. **Phase 1 — Language Fitness** (Sprint 271-272): `LanguageFitnessScorer` + 2-language test projects
2. **Phase 2 — FFI + Symbol Index** (Sprint 273-275): `PolyglotFFIGlueGenerator` + `CrossLanguageSymbolIndexEmitter`
3. **Phase 3 — LSP Orchestration** (Sprint 276-278): LSP proxy + cross-language goto/rename/hover
4. **Phase 4 — DAP Orchestration** (Sprint 279-281): DAP proxy + unified stack traces + boundary breakpoints
5. **Phase 5 — Proof** (Sprint 282-285): Full polyglot test harness + all-language test projects

### Done definition for "lossless polyglot transpiling"

- A single spec generates N language variants (N = all supported Whetstone generators).
- All variants pass an identical black-box test suite.
- Cross-language calls work at runtime via generated FFI glue (no hand-written bindings).
- Goto-definition, rename, and hover work across all language seams in the editor.
- A unified debugger session can step through a call that crosses three or more language boundaries.
- The only hand-written artifact is the spec.
