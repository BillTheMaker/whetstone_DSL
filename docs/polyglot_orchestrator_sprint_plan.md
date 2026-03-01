# Polyglot Orchestrator — Sprint Plan
> Created: 2026-02-28
> Follows: Sprint 270 (all 26 generator readiness gaps closed)
> Goal: True lossless polyglot code generation with unified editing and debugging

---

## What We Are Building

A system where:
1. A single spec drives code generation into multiple languages simultaneously
2. Each language gets the component it is best suited for (fitness-scored)
3. FFI boundaries between languages are generated, not written by hand
4. A single editor session understands all languages via an LSP orchestrator
5. A single debugger session can step across language boundaries via a DAP orchestrator
6. A test harness proves behavioral parity across all language variants

The theoretical endpoint: a project with components in every supported Whetstone
generation language, each component calling the next, with no hand-written glue,
unified editing, and unified debugging. This proves lossless polyglot transpiling
as a construction, not a claim.

---

## Phase 1 — Language Fitness Scorer (Sprint 271-272)
**Steps 1883-1892**

The ability to analyze an AST subtree and recommend a ranked list of target languages
based on computational shape. Prerequisite for everything else — you can't route
generation to the right language without knowing what "right" means.

### Sprint 271: LanguageFitnessScorer core (Steps 1883-1887)

| Step | Component | What |
|------|-----------|------|
| 1883 | `ASTFeatureExtractor` | Extract scalar features from AST subtrees: mutation ratio, recursion shape, concurrency primitives, I/O pattern, type complexity |
| 1884 | `LanguageIdiomProfile` | Static profiles for each supported language: ideal feature vector per language |
| 1885 | `LanguageFitnessScorer` | Score AST features against profiles, return ranked list with rationale |
| 1886 | `whetstone_score_language_fitness` | MCP tool wiring |
| 1887 | Sprint 271 integration | Cross-component test: spec section → features → scores → ranked list |

### Sprint 272: Fitness-routed 2-language test projects (Steps 1888-1892)

| Step | Component | What |
|------|-----------|------|
| 1888 | `PolyglotProjectSpec` | Spec format extension: per-section language hints + fitness override |
| 1889 | Test project: `poly-sort` | Rust (sort core) + Python (data gen/validation) — fitness-routed |
| 1890 | Test project: `poly-api` | Go (HTTP server) + TypeScript (client) — fitness-routed |
| 1891 | Test project: `poly-parse` | C++ (lexer/parser) + Haskell (AST transformation) — fitness-routed |
| 1892 | Sprint 272 integration | All 3 test projects: generate → compile → run → output matches expected |

---

## Phase 2 — FFI Glue + Symbol Index (Sprint 273-275)
**Steps 1893-1907**

Generate the boundaries between languages from the shared AST. This is what makes
the boundaries type-safe and eliminates hand-written glue code.

### Sprint 273: PolyglotFFIGlueGenerator (Steps 1893-1897)

| Step | Component | What |
|------|-----------|------|
| 1893 | `ABIBoundaryExtractor` | Identify AST nodes that cross language boundaries (exported functions, shared structs) |
| 1894 | `CHeaderEmitter` | Emit C ABI header as neutral intermediary for any language pair |
| 1895 | `RustPythonBindingEmitter` | Rust `extern "C"` + Python ctypes binding from same AST node |
| 1896 | `GoCppBindingEmitter` | CGo + C++ extern from same AST node |
| 1897 | Sprint 273 integration | Rust↔Python round-trip: Python calls Rust via generated binding |

### Sprint 274: More binding pairs + DWARF annotations (Steps 1898-1902)

| Step | Component | What |
|------|-----------|------|
| 1898 | `CppJSBindingEmitter` | C++ N-API binding from AST node |
| 1899 | `RustGoBindingEmitter` | Rust FFI + CGo binding from AST node |
| 1900 | `DWARFBoundaryAnnotator` | Annotate generated DWARF with cross-language AST node IDs (needed for DAP orchestrator) |
| 1901 | `whetstone_generate_ffi_glue` | MCP tool wiring |
| 1902 | Sprint 274 integration | Go↔C++ round-trip: Go calls C++ via generated binding |

### Sprint 275: Cross-Language Symbol Index (Steps 1903-1907)

| Step | Component | What |
|------|-----------|------|
| 1903 | `SCIPEmitter` | Emit SCIP-format symbol index from shared AST after multi-language generation |
| 1904 | `CrossLanguageSymbolTable` | In-memory symbol table linking same-origin symbols across all generated languages |
| 1905 | `SymbolIndexUpdater` | Incremental update on re-generation (don't rebuild from scratch on every change) |
| 1906 | `whetstone_emit_symbol_index` | MCP tool wiring |
| 1907 | Sprint 275 integration | poly-sort: generate Rust+Python → emit index → verify cross-language symbol links |

---

## Phase 3 — LSP Orchestration (Sprint 276-278)
**Steps 1908-1922**

An LSP proxy that routes editor queries through the symbol index. The editor
sees one language server; the orchestrator fans out to the right per-language
server or resolves via the symbol index for cross-language queries.

### Sprint 276: LSP proxy core (Steps 1908-1912)

| Step | Component | What |
|------|-----------|------|
| 1908 | `LSPProxyServer` | LSP protocol handler that forwards to per-language servers |
| 1909 | `LanguageServerRouter` | Route by file extension / detected language |
| 1910 | `CrossLanguageBoundaryDetector` | Identify request targets that are cross-language boundary symbols |
| 1911 | `CrossLanguageDefinitionResolver` | Resolve goto-definition across language seams via symbol index |
| 1912 | Sprint 276 integration | poly-sort: goto-definition from Python call site lands on Rust source |

### Sprint 277: Hover + references + rename (Steps 1913-1917)

| Step | Component | What |
|------|-----------|------|
| 1913 | `CrossLanguageHoverProvider` | Hover at boundary shows both-side types + C ABI type |
| 1914 | `CrossLanguageReferencesProvider` | Find all references across all language targets |
| 1915 | `CrossLanguageRenameOrchestrator` | Rename propagates to all LSPs + updates symbol index |
| 1916 | `LSPProxyConfig` | Config file: which languages, which LSP servers, which port |
| 1917 | Sprint 277 integration | poly-api: rename shared type in Go+TS simultaneously |

### Sprint 278: LSP proxy hardening (Steps 1918-1922)

| Step | Component | What |
|------|-----------|------|
| 1918 | `LSPRequestCache` | Cache intra-language responses to reduce round-trip latency |
| 1919 | `LSPServerHealthMonitor` | Detect crashed language servers, route to degraded mode |
| 1920 | `LSPProxyDiagnosticAggregator` | Merge per-language diagnostics into unified list (no duplicates at boundaries) |
| 1921 | LSP proxy test suite | End-to-end: proxy running, 3 language servers, cross-language queries correct |
| 1922 | Sprint 278 integration | All 3 Phase 1 test projects: LSP proxy running, cross-language navigation verified |

---

## Phase 4 — DAP Orchestration (Sprint 279-281)
**Steps 1923-1937**

A DAP proxy that presents a unified debugger session across all language runtimes.
Stack traces are annotated, stepping across language boundaries works, breakpoints
can be set on AST nodes (firing in any language target).

### Sprint 279: DAP proxy core + stack annotation (Steps 1923-1927)

| Step | Component | What |
|------|-----------|------|
| 1923 | `DAPProxyServer` | DAP protocol handler forwarding to per-language debug adapters |
| 1924 | `DebugAdapterRouter` | Route by active frame language (detected from DWARF annotations) |
| 1925 | `PolyglotStackTraceDecoder` | Annotate mixed-language stack with origin language + AST node ID per frame |
| 1926 | `LanguageSeamFrameDetector` | Identify frames that sit at a language boundary (from DWARF annotations) |
| 1927 | Sprint 279 integration | poly-sort: unified stack trace shows Rust + Python frames labeled correctly |

### Sprint 280: Cross-language step + breakpoints (Steps 1928-1932)

| Step | Component | What |
|------|-----------|------|
| 1928 | `CrossLanguageStepOverHandler` | Step-over at a boundary call executes foreign call atomically, pauses in caller |
| 1929 | `CrossLanguageStepIntoHandler` | Step-into at a boundary call switches active debug adapter to foreign language |
| 1930 | `ASTNodeBreakpointMapper` | Set breakpoint by AST node ID, fires in all language targets that implement that node |
| 1931 | `BreakpointSyncOrchestrator` | Keep breakpoints in sync across all active debug adapters |
| 1932 | Sprint 280 integration | poly-api: breakpoint on Go handler fires; step-into crosses into TS call |

### Sprint 281: DAP hardening + multi-adapter coordination (Steps 1933-1937)

| Step | Component | What |
|------|-----------|------|
| 1933 | `DAPSessionLifecycleManager` | Launch/attach/detach per-language adapters as the debug session progresses |
| 1934 | `UnifiedVariableInspector` | Inspect variables across the boundary (translate types to editor representation) |
| 1935 | `DAPHealthMonitor` | Handle adapter crashes, fall back to single-language mode gracefully |
| 1936 | DAP proxy test suite | Step-into, step-over, breakpoint, variable inspect — all cross-language |
| 1937 | Sprint 281 integration | poly-parse: C++↔Haskell debug session — step across AST transformation boundary |

---

## Phase 5 — The Proof (Sprint 282-285)
**Steps 1938-1957**

The full polyglot test harness and the all-language test projects. This is the
demonstration that the system is real — not a claim but a construction.

### Sprint 282: Polyglot test harness (Steps 1938-1942)

| Step | Component | What |
|------|-----------|------|
| 1938 | `PolyglotSpecRunner` | Generate N language variants from one spec, compile and run all |
| 1939 | `BehavioralParityChecker` | Compare outputs across variants for same inputs, flag divergence |
| 1940 | `ParityRegressionGuard` | Block new language additions that break existing parity |
| 1941 | `whetstone_run_polyglot_suite` | MCP tool wiring |
| 1942 | Sprint 282 integration | poly-sort: Python+Rust+Go+Haskell — all produce identical output for 1000 inputs |

### Sprint 283: Wacky project 1 — poly-pipeline (Steps 1943-1947)

A data pipeline where each stage is a different language:
- Python (CSV ingestion + schema validation)
- Rust (data normalization + outlier detection, perf-critical)
- Go (fan-out routing to multiple sinks, concurrency)
- Haskell (statistical transformation, pure function)
- JavaScript/TypeScript (JSON serialization + HTTP push to consumer)

| Step | What |
|------|------|
| 1943 | Spec: poly-pipeline — all 5 components, interfaces defined in shared AST |
| 1944 | Generate all 5 language targets + FFI glue at each boundary |
| 1945 | Integration: end-to-end CSV in → HTTP push out, all 5 stages running |
| 1946 | LSP + DAP: unified editing and debugging across all 5 languages |
| 1947 | Parity check: replace each stage with a reference Python implementation, verify same output |

### Sprint 284: Wacky project 2 — poly-compiler (Steps 1948-1952)

A toy compiler where each phase is the language most suited to it:
- C++ (lexer — character-level performance, no GC pressure)
- Haskell (parser — recursive descent, algebraic data types)
- Rust (type checker — borrow-safe, zero-cost ownership tracking)
- Lisp/Scheme (optimizer — s-expression AST manipulation, homoiconicity)
- Go (code emitter — goroutine-parallel emission, simple concurrency)

| Step | What |
|------|------|
| 1948 | Spec: poly-compiler — 5 phases, shared IR type defined in AST |
| 1949 | Generate all 5 language targets + FFI glue |
| 1950 | Integration: source string in → bytecode out, all 5 phases connected |
| 1951 | LSP + DAP: cross-language navigation and debugging in the compiler itself |
| 1952 | Parity check: each phase replaceable with Python reference implementation |

### Sprint 285: Wacky project 3 — poly-everything (Steps 1953-1957)

One project, every supported generation language. Each language owns one function.
The chain: Language A calls B, B calls C, ... last language returns to A.
Fitness scorer assigns each language to the function most suited to it.
All FFI glue generated. All boundaries in the symbol index. Full LSP + DAP.

This is the theoretical proof: a single shared AST produced working code in every
supported language, connected at runtime, navigable in a single editor session,
debuggable in a single debugger session.

| Step | What |
|------|------|
| 1953 | Spec: poly-everything — one function per language, fitness-assigned |
| 1954 | Generate all N language targets + all FFI glue in one pipeline run |
| 1955 | Integration: full chain executes, output correct |
| 1956 | LSP + DAP: navigate and debug across all N language boundaries |
| 1957 | Parity report: all N language functions individually replaceable, parity confirmed |

---

## Success Definition

The sprint plan is complete when:

1. A spec with no target language hint produces a fitness-ranked recommendation
2. A 2-language project (any pair) generates with correct FFI glue — no hand-written bindings
3. Goto-definition from language A lands on the definition in language B
4. A single debugger session steps across a language A → language B call
5. poly-pipeline (5 languages) passes end-to-end with all stages running
6. poly-compiler (5 languages) compiles and runs a "hello world" source string
7. poly-everything (all languages) executes the full chain and parity is confirmed

At step 7, lossless polyglot transpiling is proven as a construction.

---

## Notes on Scope

- LSP and DAP orchestration layers are editor integrations, not MCP tools.
  They run as separate processes alongside the whetstone_mcp binary.
- The SCIP symbol index is the shared data structure. It must be written to
  a known path on generation and read by both the LSP and DAP orchestrators.
- Phase 5 test projects are not production software. They are proofs of concept
  designed to stress the boundaries of the orchestration system.
- The practical value is not the test projects themselves but the integration
  layer capability they prove: any project that needs FFI between two environments
  can use Whetstone to generate the boundary from a shared spec.
