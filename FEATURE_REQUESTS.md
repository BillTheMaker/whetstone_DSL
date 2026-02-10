# Feature Requests

> Backlog of feature ideas to triage into future sprints (e.g., Sprint 6/7).

## Security Vulnerability Awareness (Dependencies) â€” IMPLEMENTED (Sprint 6, Steps 190â€“195)

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
- Dependencies panel: inline warning badges and ï¿½View Advisoryï¿½.
- Problems panel: security diagnostics with severity.
- Agent hints: prefer safe alternatives when available.

---

## Semantic Annotations for Library APIs â€” IMPLEMENTED (Sprint 6, Steps 193â€“194)

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
- Agent prompts: ï¿½Use @serialize APIsï¿½ guidance.

---

## Notes
- Treat these as separate features to schedule independently.
- Likely Sprint 6/7, after core library-aware flow is stable.

## LLM Tooling & MCP Bridge â€” PLANNED (Sprint 7, Steps 202â€“234)

**Goal:** Make the agent API easy for LLMs to use and optionally expose it via MCP.

**Status:** Full plan written in `sprint7_plan.md`. 33 steps across 6 phases:
- Phase 7a: API documentation & JSON schemas (Steps 202â€“206)
- Phase 7b: MCP server with tools/resources/prompts (Steps 207â€“213)
- Phase 7c: Synthetic trace generation for training data (Steps 214â€“219)
- Phase 7d: Evaluation harness for LLM tool-use accuracy (Steps 220â€“224)
- Phase 7e: Model-specific tool definitions â€” Claude, Codex, open-source (Steps 225â€“229)
- Phase 7f: Session recording pipeline â€” capture, anonymize, export (Steps 230â€“234)

---
## PHP Language Support (Full Pipeline) — PROPOSED

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

## WordPress Support (Library + Semantics) — PROPOSED

**Goal:** Enable WordPress-aware tooling on top of PHP support.

**Scope:**
- Library symbol stubs for WordPress core APIs
- Semantic tags for WP concepts (hooks, filters, actions)
- Agent suggestions for safe WP patterns

---

## Rust Plugin Packaging (Conflict Resolver) — PROPOSED

**Goal:** Package a Rust-based topological conflict resolver as a Whetstone plugin.

**Scope:**
- Define plugin interface for external tools
- Packaging format and plugin discovery/loading
- Example integration with a Rust binary or shared library

## Julia Language Support (Full Pipeline) — PROPOSED

**Goal:** Add full Julia support (parse, AST, generate, project, and annotations) to Whetstone.

**Scope:**
- Tree-sitter Julia parser integration
- Julia AST mapping to SemAnno concepts
- Julia generator with annotation-aware output
- Cross-language projection to/from Julia
- Tests: parse/generate round-trip, annotation preservation, projection matrix coverage

---

## Julia ML Projection Layer — PROPOSED

**Goal:** Map common Python ML/Numerical APIs to Julia equivalents while preserving optimization intent.

**Scope:**
- API mapping table (NumPy/Pandas/Torch core calls ? Julia equivalents)
- Semantic tags for numerical/tensor operations
- Fallback interop for unmapped calls (PyCall/JuliaCall)
- Dual projections: clean surface Julia + preserved optimization annotations


## Julia Packaging Strategy — PROPOSED

**Goal:** Define a reliable packaging path for Julia-based artifacts.

**Scope:**
- Document options: runtime install, PackageCompiler.jl, embedding Julia as a library
- Provide recommended defaults for CLI tools vs GUI apps
- Include guidance for minimizing startup latency and bundle size

## Python/C++/JS-TS Support Enhancements — PROPOSED

**Goal:** Expand first-class support for Python, C++, and JS/TS beyond current parse/generate.

**Scope:**
- Improve language-specific projection fidelity and diagnostics
- Expand standard library and ecosystem stubs (pip/npm/vcpkg)
- Add language-specific refactor recipes and annotation guidance
- Strengthen test corpus and projection round-trip coverage

---

## Plugin System Enhancements — PROPOSED

**Goal:** Expand plugin support for external tools and language packs.

**Scope:**
- Standardize plugin packaging format
- Document plugin API and lifecycle hooks
- Add plugin discovery and version compatibility checks
- Provide example plugins for language packs and external analyzers
