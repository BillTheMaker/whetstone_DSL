# Whetstone DSL — Architect Notes

## Core Thesis

AI+Human teams will always outperform AI alone. Millions of years of cognitive evolution
means humans belong on the team — not as button-pushers on an assembly line, but as
architects, reviewers, and decision-makers where ambiguity, judgment, and reflection matter.

## The Three Pillars

### 1. Semantic Annotations as Workflow Routing
Annotations are not just metadata. They are **routing signals** that determine:
- Who handles a task (human architect, human engineer, LLM, SLM, deterministic template)
- What context is needed (local, file-wide, project-wide, cross-project)
- What kind of thinking is required (mechanical, analytical, creative, architectural)
- When to defer (ambiguity too high, risk too high, human review required)

The annotation taxonomy makes the cost/complexity of each task **knowable before spending
tokens or compute**. This is the core optimization: don't send a PhD to push buttons on
the assembly line.

### 2. Universal Transpilation
Whetstone's AST is language-agnostic. The vision:
- Ingest legacy code in any supported language
- Annotate it with semantic meaning (intent, complexity, risk, contracts)
- Transpile to modern, safer, more efficient implementations in any target language
- Eventually: describe a problem, the system chooses the tech stack

This requires broad language coverage. Languages to support (in rough priority):
- **Done:** Python, C++, JavaScript, TypeScript, Java, Rust, Go, Elisp
- **Sprint 11:** Kotlin, C#
- **Near-term:** C, WebAssembly, Common Lisp, Scheme
- **Mid-term:** F#, VB.NET, SQL stacks (PostgreSQL, T-SQL, MySQL)
- **Later:** x86 assembly, ARM assembly, additional as needed

### 3. Professional GUI for Human-in-the-Loop
The GUI is not an afterthought. If the thesis is that humans must be part of the team,
then the interface must make what's happening **intuitive to understand**. It should look
like a finished product from a professional company.

Current issues:
- Floating windows need to be docked
- Key combination symbols not implemented
- Color scheme needs work
- Navigation needs to be clean and discoverable

## The Skeleton AST — How Architects Model Projects

Before code exists, the architect (human or agent) creates a **skeleton AST**: modules,
functions, and classes with names, signatures, and annotations — but no implementations.
The annotations on skeleton nodes describe:
- **What** it should do: @Intent("parse HTTP headers and extract auth tokens")
- **How hard** it is: @Complexity(high), @Ambiguity(medium)
- **Who should do it**: @Automatability(llm), @Review(required, human)
- **What context is needed**: @ContextWidth(project)
- **What order**: @Priority(high, blockedBy=["auth_module"])
- **What state it's in**: @ImplementationStatus(skeleton)

The skeleton + annotations = project specification = task queue. This is the bridge
between "describe a problem" and "execute the solution." The architect doesn't write
code — they write the structure and intent. The orchestration engine reads the
annotations and routes each skeleton to the right worker.

This also means every project modeling decision is captured as structured data (which
annotations the architect chose, how they decomposed the problem, what they marked
for human review). Post-Sprint 25, this becomes training data for teaching AI how
experienced engineers think about projects before implementation begins.

## Architecture Invariants

- **Header-only C++** — all implementation in .h files, no .cpp (except mcp_main.cpp and test files)
- **600-line file limit** — split when approaching, use Extended/Impl pattern
- **Test plans generated per-step from complexity analysis** — test count and type emerge from what's being built (unit, negative, integration, contract, boundary, property, smoke, performance, regression), not from a fixed template. Architect approves the plan before workers execute. Prior sprints used a fixed "12 tests per step" scaffold; Sprint 23c replaces this with the test plan generator.
- **MCP-first** — every capability exposed as an MCP tool
- **Headless-first** — all functionality works without GUI
- **No external dependencies beyond tree-sitter** — regex parsers for languages without tree-sitter grammars

## Key Metrics

- Current: 38+ MCP tools, 67+ annotation types, 8 languages, ~296 steps, ~3000+ tests
- Sprint 11 target: 40+ tools, 10 languages, validation complete
- Sprint 15 target: Workflow orchestration operational, 14+ languages
- Sprint 20 target: Claude Code plugin usable end-to-end, professional GUI
- Sprint 25 target: Self-hosting, legacy code ingestion, tech stack selection

## What "Done" Looks Like

A developer says: "I have a legacy C++ codebase. I want to modernize it, add safety
guarantees, and port the networking layer to Rust."

Whetstone:
1. Ingests the C++ code, builds annotated AST
2. Architect (human or agent) reviews annotations, marks routing decisions
3. Task queue populates: deterministic renames go to templates, straightforward ports
   go to SLM, complex architectural decisions surface for human review
4. Engineer approves the queue, work executes in parallel
5. Cross-language transpilation produces the Rust networking layer
6. Validation catches conflicts, diagnostics guide fixes
7. Human reviews the 5% that needed judgment, not the 95% that didn't

## Sprint Philosophy

- Quality over speed. Build what's right, not what's fast.
- Languages are additive and quick — keep adding them throughout.
- The orchestration layer is the hard architectural work — get it right.
- The GUI is load-bearing for the thesis — it must be professional.
- Self-hosting is the ultimate proof: Whetstone annotates and transpiles itself.
