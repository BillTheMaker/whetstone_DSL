# Feature Requests

> Backlog of feature ideas to triage into future sprints (e.g., Sprint 6/7).

## Security Vulnerability Awareness (Dependencies)

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
- Dependencies panel: inline warning badges and “View Advisory”.
- Problems panel: security diagnostics with severity.
- Agent hints: prefer safe alternatives when available.

---

## Semantic Annotations for Library APIs

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
- Agent prompts: “Use @serialize APIs” guidance.

---

## Notes
- Treat these as separate features to schedule independently.
- Likely Sprint 6/7, after core library-aware flow is stable.

## LLM Tooling & MCP Bridge

**Goal:** Make the agent API easy for LLMs to use and optionally expose it via MCP.

**Concept:**
- Document JSON-RPC API with schemas and examples.
- Generate synthetic interaction traces for fine-tuning / tool-use evaluation.
- Add an MCP server wrapper that exposes current agent methods as MCP tools/resources.

**Candidate Deliverables:**
- `docs/AGENT_API.md` with request/response schemas
- Example flows: read AST ? mutate ? verify
- MCP bridge module (optional): maps JSON-RPC to MCP tools

---
