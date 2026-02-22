# Feature Requests

> Backlog of feature ideas to triage into future sprints (e.g., Sprint 6/7).

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
