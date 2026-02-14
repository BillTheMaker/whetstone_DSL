# Sprint 24 Plan: Security + Static Analysis

## Context

Security isn't a feature you bolt on at the end — but it is a domain that
benefits from having the full annotation taxonomy, transpilation engine,
and orchestration system in place first. Sprint 24 adds security as a
first-class concern: security annotations, vulnerability detection, dependency
audit, and security review as a routing category.

When the system encounters `@Risk(security)` or detects a vulnerable pattern,
it knows to route that work item to human review with full security context.
The annotation system makes security findings *actionable* — not just warnings
in a log, but structured routing decisions.

---

## Phase 24a: Security Annotations + Detection (Steps 482-487)

### Step 482: Security Annotation Types — Subject 10 (12 tests)
- New annotation subject: Security
  - `TrustBoundaryAnnotation` — marks where trusted/untrusted data crosses
  - `InputValidationAnnotation` — validation: "sanitized" | "raw" | "escaped"
  - `AuthAnnotation` — authRequired: bool, roles: vector, method: "jwt" | "session" | "api-key"
  - `EncryptionAnnotation` — algorithm, keySize, mode (at-rest, in-transit)
  - `SecretsAnnotation` — marks code that handles secrets, rotation policy
  - `CORSAnnotation` — origins, methods, credentials
- Serialization, CompactAST, SidecarPersistence for all new types

### Step 483: OWASP Pattern Detection (12 tests)
- Detect OWASP Top 10 vulnerability patterns via AST analysis:
  - **Injection:** string concatenation in SQL queries, unsanitized user input in commands
  - **Broken Auth:** hardcoded credentials, missing auth checks on routes
  - **Sensitive Data:** secrets in source code, unencrypted storage
  - **XXE:** XML parsing without entity restriction
  - **Broken Access Control:** missing role checks, direct object references
  - **Security Misconfiguration:** debug mode in production annotations, permissive CORS
  - **XSS:** unsanitized output in HTML generation
  - **Deserialization:** untrusted data deserialization
  - **Known Vulnerabilities:** deprecated crypto, known-insecure functions
  - **Logging:** sensitive data in log statements
- Each detection produces structured diagnostic (E1300+ range)
- Confidence level per finding (some are heuristic, not certain)

### Step 484: Dependency Audit Annotations (12 tests)
- `DependencyAnnotation` — package name, version, known CVEs, license
- Parse dependency files: requirements.txt, Cargo.toml, package.json, pom.xml
- Flag known-vulnerable versions from annotation data
- License compatibility checking (GPL vs MIT vs Apache in same project)
- Not a real-time CVE database — annotation-based tracking that the
  orchestration engine can route for periodic review

### Step 485: Security-Aware Routing (12 tests)
- Security findings automatically set routing annotations:
  - OWASP finding → @Risk(security) + @Review(required, human)
  - Trust boundary crossing → @ContextWidth(project) (need full context)
  - Crypto code → @Automatability(human) (never auto-generate crypto)
  - Auth logic → @Review(required, human) + @Ambiguity(high)
- Routing engine recognizes security annotations as highest-priority review triggers
- Security review items get enhanced context: threat model, adjacent trust boundaries

### Step 486: Security Report Generation (12 tests)
- Structured security report for a project:
  - Executive summary: risk level, critical findings count
  - Per-finding: OWASP category, CWE code, affected code, confidence, remediation
  - Trust boundary map: which modules trust which
  - Annotation coverage: % of code with security annotations
- Export formats: JSON (for orchestration), Markdown (for human review)

### Step 487: Phase 24a Integration (8 tests)
- Scan a Python web app → detect SQL injection + missing auth → security report
- Security annotations route sensitive code to human review
- OWASP findings produce structured diagnostics
- Dependency audit flags known-vulnerable packages
- Security report includes trust boundary analysis
- MCP tools: whetstone_security_scan, whetstone_security_report, whetstone_audit_dependencies
- 90+ MCP tools total

---

## Phase 24b: Secure Transpilation (Steps 488-492)

### Step 488: Security-Preserving Translation (12 tests)
- When transpiling code with security annotations:
  - @InputValidation preserved in target language (not silently dropped)
  - @TrustBoundary maintained at module boundaries
  - @Auth annotations mapped to target-language auth patterns
  - @Encryption mapped to target-language crypto libraries
- Security annotations NEVER auto-removed during transpilation
- Any security annotation that can't map to target → @Review(required, human)

### Step 489: Secure-by-Default Code Generation (12 tests)
- Generated code follows secure defaults:
  - SQL queries use parameterized queries, never string concatenation
  - User input always has @InputValidation(raw) until explicitly sanitized
  - File operations include path traversal checks
  - Network calls include timeout and TLS verification
  - Crypto uses current best practices (no MD5, no SHA1 for security)
- Deterministic/template workers enforce these defaults
- LLM context bundles include security requirements

### Step 490: Threat Model Integration (12 tests)
- Annotations can express a threat model:
  - @TrustBoundary(name, level) — "external-api", "database", "user-input"
  - Data flow through trust boundaries → automatic @InputValidation requirements
  - Missing validation at boundary → E1300 diagnostic
- Threat model as a project-level annotation (on Module)
- Visualization data for Sprint 19's GUI (trust boundary overlay)

### Step 491: Security Testing Skeleton Generation (12 tests)
- Generate security test skeletons alongside production code:
  - Input validation tests (fuzzing boundaries)
  - Auth bypass tests
  - SQL injection probe tests
  - XSS payload tests
  - Access control matrix tests
- Skeletons annotated with @Intent describing what to test
- Routed: simple validation tests → SLM, complex penetration → human

### Step 492: Phase 24b Integration + Sprint Summary (8 tests)
- Transpile Python → Rust: security annotations preserved and strengthened
- Generated code uses parameterized SQL, validated input
- Threat model produces trust boundary diagnostics
- Security test skeletons generated and meaningful
- Sprint 24 totals: security is a first-class orchestration concern

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 24a | 482-487 | 68 | Security annotations, OWASP detection, dependency audit, routing |
| 24b | 488-492 | 56 | Secure transpilation, defaults, threat models, security tests |
| **Total** | **482-492** | **~124** | 11 steps |
