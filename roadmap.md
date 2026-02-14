# Whetstone DSL — Sprint Roadmap (Sprints 11-25)

## Sprint 11: Dial It In (in progress)
Finish the annotation foundation: validation for all 67 types, parser deepening for
class/async/lambda, Kotlin + C# languages, and generalized annotation inference.
Replaces training data export with workflow-relevant annotation types.

## Sprint 12: Workflow Model + C++ Depth
Introduce WorkItem, TaskAnnotation, and routing annotation types (@ContextWidth,
@ReviewRequired, @DeferToHuman, @Automatable). Add C++ constructs from feature-requests
items 1-5 (multiple inheritance, CRTP, preprocessor, enum class, namespaces).

## Sprint 13: GUI Overhaul Phase 1
Dock all floating windows, implement key combination symbols, professional color scheme,
clean navigation. The human-in-the-loop thesis requires a GUI that makes the system
intuitive to understand. Should look like a finished product.

## Sprint 14: Language Batch 1 — C, WebAssembly, Common Lisp, Scheme
C is critical for legacy ingestion. WASM for web deployment targets. Lisp/Scheme for
functional paradigm coverage and macro system representation in the AST.

## Sprint 15: Orchestration Engine
Routing logic: annotation → dispatch decision (deterministic/SLM/LLM/human). Task queue
with priority and dependency management. Context size estimation from annotations.
Review gates and approval workflows.

## Sprint 16: C++ Depth + Self-Hosting Phase 1
Feature-requests items 6-11 (type aliases, static members, const/constexpr, smart
pointers, auto type deduction, casts). Begin self-hosting: parse simplest Whetstone
headers through our own pipeline.

## Sprint 17: Language Batch 2 — .NET Family + SQL
F#, VB.NET for server-side .NET coverage. SQL dialect support (PostgreSQL, T-SQL, MySQL)
for full-stack transpilation scenarios.

## Sprint 18: Claude Code Plugin + MCP Workflow Tools
Workflow-aware MCP tools that understand task queues and routing. Claude Code plugin that
an engineer can install and immediately use for annotation-driven project workflows.
End-to-end: describe project → annotate → route → execute.

## Sprint 19: GUI Phase 2 — Workflow Visualization
Task queue visualization, annotation overlay on code, workflow status dashboard, human
review interface. The architect should see the project as a structured flow, not a
wall of text.

## Sprint 20: Legacy Code Ingestion
Parse legacy code (old C, old C++, old Java), infer annotations, suggest modernization
paths. Safe transpilation to modern equivalents with human review at decision points.

## Sprint 21: Cross-Language Transpilation Engine
Beyond syntactic projection: algorithm-level translation guided by semantic annotations.
Safety and efficiency annotations steer the translation. Verify behavioral equivalence
where possible.

## Sprint 22: Language Batch 3 — Assembly + Remaining Gaps
x86 and ARM assembly representation. C++ feature-requests items 12-17 (range-for,
structured bindings, operator overloading, exception handling, STL patterns).
Fill remaining language gaps identified during real-world use.

## Sprint 23: Architect Mode + Tech Stack Selection
Describe a problem in natural language → system suggests tech stack, project structure,
and annotation skeleton. Architecture-level templates. Project scaffolding from
annotated descriptions.

## Sprint 24: Security + Static Analysis
Security annotations and validation. Vulnerability pattern detection (OWASP top 10).
Dependency audit annotations. Security review as a routing category in the
orchestration engine.

## Sprint 25: Integration, Self-Hosting, Polish
Full self-hosting: Whetstone parses, annotates, and transpiles its own source.
End-to-end workflow testing across all supported languages. Performance optimization.
Release preparation.

---

## Post-25: Training Data Harvest + Project Modeling Intelligence

### Training Data from Real Workflows
By Sprint 25, the system has accumulated real-world signal across every layer:
- **Architect modeling decisions**: skeleton ASTs, annotation choices, how humans decompose
  problems into annotated task queues. This is the rarest and most valuable data — how
  experienced engineers *think about* projects before code exists.
- **Routing outcomes**: which annotation patterns led to correct dispatch (deterministic vs
  SLM vs LLM vs human), and which routing decisions had to be corrected by humans.
- **Transpilation pairs**: verified source→target translations across 20+ languages, validated
  by the system and reviewed by humans. Not synthetic — real production transpilations.
- **Human review decisions**: what architects approved, rejected, modified. The diff between
  what the system proposed and what the human accepted is pure training signal.
- **Orchestration traces**: full workflow recordings from project description through
  completion, including where the system deferred to humans and why.

### Project Modeling as a Learnable Skill
The architect phase (skeleton AST + annotations = project specification) generates a
unique dataset: how humans model projects *before implementation begins*. This includes:
- How a problem description maps to module decomposition
- Which annotations an experienced architect applies to which skeleton nodes
- How dependency structure is decided
- Where the architect marks "this needs human review" vs "this is automatable"
- How the architect estimates complexity, risk, and context requirements

This is the data needed to eventually train a model that can propose project structures
from problem descriptions (Sprint 23's "Tech Stack Selection"), but grounded in real
human decisions rather than synthetic examples.

### Sprint 26+: Fine-Tuning Pipeline
- Export accumulated workflow decisions, routing outcomes, and transpilation pairs
- Build fine-tuning datasets for routing model improvement
- Build fine-tuning datasets for project modeling (skeleton generation from descriptions)
- Feedback loop: improved models → better routing → better outcomes → better training data
- The system improves its own judgment from historical human decisions
