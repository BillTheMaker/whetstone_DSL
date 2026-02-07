# Sprint 2 Vision: Whetstone Editor & Structured Editing

## Vision Statement

Build a next-generation code editor that combines Emacs's powerful libraries with a modern, symbol-rich UI, and introduce **structured editing** where humans and AI agents construct code by choosing from legal primitives rather than typing syntax.

---

## Goals

1. **Hybrid Editor Architecture** - Dear ImGui shell + headless Emacs servers
2. **Structured Editing UI** - Choice-based code construction, syntax errors impossible
3. **Unified Human/Agent Interface** - Same API for clicking and scripting
4. **Elisp ↔ C++ Projection** - Dogfood Whetstone by building the editor with it
5. **Proof of Concept** - Demonstrate AST-native development accelerates iteration

---

## Architecture: Dear ImGui + Headless Emacs

### Why Hybrid?

| Approach | Pros | Cons |
|----------|------|------|
| Fork Emacs | Full compatibility | Massive codebase, GUI baggage |
| Pure Dear ImGui | Clean slate, full control | Lose Emacs libraries |
| **Hybrid** | Best of both | Complexity in coordination |

The hybrid approach gives us:
- **Emacs libraries** without Emacs's GUI limitations
- **Dear ImGui flexibility** without reimplementing text processing
- **Parallelism** via multiple Emacs server instances
- **Upstream compatibility** - Emacs updates just work

### System Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         Whetstone Editor                                │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    Dear ImGui Shell (C++)                        │   │
│  │                                                                  │   │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────────────┐   │   │
│  │  │ File     │ │ Editor   │ │ Terminal │ │ Structured       │   │   │
│  │  │ Tree     │ │ Tabs     │ │ Panel    │ │ Edit Panel       │   │   │
│  │  │          │ │          │ │          │ │                  │   │   │
│  │  │ 📁 src   │ │ [foo.py] │ │ $ _      │ │ [Function ▼]     │   │   │
│  │  │ 📁 lib   │ │ [bar.cpp]│ │          │ │ [Variable ▼]     │   │   │
│  │  │ 📄 main  │ │          │ │          │ │ [Expression ▼]   │   │   │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────────────┘   │   │
│  │                                                                  │   │
│  │  ┌──────────────────────────────────────────────────────────┐   │   │
│  │  │ Command Palette (Emacs-style, keyboard-driven)           │   │   │
│  │  │ > find-file  > magit-status  > consult-ripgrep           │   │   │
│  │  └──────────────────────────────────────────────────────────┘   │   │
│  │                                                                  │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                    │                                    │
│                                    │ IPC (JSON-RPC / Shared Memory)    │
│                                    ▼                                    │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    Management Node (C++)                         │   │
│  │                                                                  │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐ │   │
│  │  │ Whetstone   │  │ Server      │  │ Agent                   │ │   │
│  │  │ AST Store   │  │ Pool Mgr    │  │ Queue                   │ │   │
│  │  │             │  │             │  │                         │ │   │
│  │  │ [AST Nodes] │  │ spawn()     │  │ [Choice sequences from  │ │   │
│  │  │ [Annotations│  │ route()     │  │  human clicks or        │ │   │
│  │  │ [Projections│  │ sync()      │  │  agent submissions]     │ │   │
│  │  └─────────────┘  └─────────────┘  └─────────────────────────┘ │   │
│  │                                                                  │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                    │                                    │
│                                    │ emacsclient protocol               │
│                                    ▼                                    │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                 Headless Emacs Server Pool                       │   │
│  │                                                                  │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │   │
│  │  │ Server 1    │  │ Server 2    │  │ Server N    │              │   │
│  │  │             │  │             │  │             │              │   │
│  │  │ File I/O    │  │ Search      │  │ Git (Magit) │              │   │
│  │  │ TRAMP       │  │ Completion  │  │ Org-mode    │              │   │
│  │  │ Dired       │  │ Helm/Vertico│  │ LSP bridge  │              │   │
│  │  └─────────────┘  └─────────────┘  └─────────────┘              │   │
│  │                                                                  │   │
│  │  (Each runs: emacs --daemon=serverN --load whetstone-bridge.el) │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### Component Responsibilities

| Component | Language | Responsibility |
|-----------|----------|----------------|
| **Dear ImGui Shell** | C++ | All rendering, keyboard/mouse input, widgets |
| **Management Node** | C++ | AST storage, server coordination, agent queue |
| **Emacs Servers** | Elisp | Text processing, file ops, search, completion |

**Future Goal:** Once Whetstone is mature, rewrite the Management Node in Whetstone AST and generate C++ from it. This is the ultimate dogfooding - the editor's core logic defined in the system it's built to create.

### Communication Protocol

```
┌────────────┐         JSON-RPC          ┌─────────────┐
│ ImGui Shell│ ◄─────────────────────────► │ Mgmt Node   │
└────────────┘                            └─────────────┘
                                                │
                                                │ emacsclient -e '(elisp-expr)'
                                                ▼
                                          ┌─────────────┐
                                          │ Emacs Server│
                                          └─────────────┘
```

**Example flow - User searches for file:**

1. User presses `Ctrl+P` in ImGui shell
2. ImGui sends `{ "method": "find-file", "query": "" }` to Management Node
3. Management Node routes to Search server: `emacsclient -s search -e '(consult-find "")'`
4. Emacs returns candidates
5. Management Node sends candidates to ImGui
6. ImGui renders searchable dropdown
7. User selects file
8. Management Node loads file into AST, sends to ImGui for rendering

---

## Structured Editing: The Core Innovation

### The Problem with Text-Based Coding

```
Traditional flow:
  Human/Agent generates text → Parser validates → Errors → Fix → Repeat

Problems:
  - Syntax errors waste time
  - Agents hallucinate invalid syntax
  - Autocompletion is probabilistic (might suggest wrong things)
  - Context is implicit (what's in scope? what types are valid?)
```

### The Solution: Choice-Based Construction

**You never type code. You choose from valid options at each step.**

```
Structured flow:
  Choose construct → Fill constrained fields → Choose next construct → Done

Benefits:
  - Syntax errors IMPOSSIBLE
  - Every option shown is LEGAL
  - Context is EXPLICIT (only in-scope items offered)
  - Agents submit choice sequences (no parsing, no errors)
```

### Structured Editing UI Flow

#### Step 1: Choose Construct Type
```
┌─────────────────────────────────────────────────────────────────┐
│  What do you want to create?                                    │
│                                                                 │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐          │
│  │ Function │ │  Class   │ │ Variable │ │  Import  │          │
│  │    λ     │ │    ◇     │ │   x=     │ │    ↓     │          │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘          │
│                                                                 │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐          │
│  │    If    │ │   For    │ │  While   │ │  Return  │          │
│  │    ?     │ │    ↻     │ │    ↺     │ │    ←     │          │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘          │
│                                                                 │
│  [Keyboard: f=Function, c=Class, v=Variable, i=Import...]      │
└─────────────────────────────────────────────────────────────────┘
```

#### Step 2: Fill Constrained Fields (Function selected)
```
┌─────────────────────────────────────────────────────────────────┐
│  Define Function                                                │
│                                                                 │
│  Name: [process_data________]     ← Text input, identifier rules│
│                                                                 │
│  Parameters:                                                    │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Name        Type           Default                      │   │
│  │  [items    ] [List[int] ▼]  [None ▼]                    │   │
│  │  [threshold] [int       ▼]  [10    ]                    │   │
│  │                                                          │   │
│  │  [+ Add Parameter]                                       │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  Return Type: [List[int] ▼]  [None] [Auto-infer]               │
│                                                                 │
│  Annotations:                                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ [@Reclaim(Tracing) ▼]  [@complexity: ___]  [+ Add]       │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  [Continue to Body →]                                          │
└─────────────────────────────────────────────────────────────────┘
```

#### Step 3: Build Function Body
```
┌─────────────────────────────────────────────────────────────────┐
│  Function: process_data(items: List[int], threshold: int)       │
│                                                                 │
│  Body:  (Click to add statement)                               │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ 1. [+ Add Statement]                                     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  ─────────────────────────────────────────────────────────────  │
│  Choose statement type:                                         │
│                                                                 │
│  [Variable]  [If]  [For]  [While]  [Return]  [Call]  [Assign]  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### Step 4: Add Variable (Variable selected)
```
┌─────────────────────────────────────────────────────────────────┐
│  Create Variable                                                │
│                                                                 │
│  Name: [result_________]                                        │
│                                                                 │
│  Initialize with:                                               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐          │
│  │ Literal  │ │ Variable │ │   Call   │ │ BinaryOp │          │
│  │   42     │ │    x     │ │   f()    │ │   a+b    │          │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

#### Step 5: Build Expression (Call selected)
```
┌─────────────────────────────────────────────────────────────────┐
│  Function Call                                                  │
│                                                                 │
│  Function: [filter        ▼]    ← Dropdown: in-scope functions │
│            ┌─────────────────┐                                  │
│            │ filter          │  ← Built-in                     │
│            │ map             │  ← Built-in                     │
│            │ process_data    │  ← Current function (recursion) │
│            │ helper_func     │  ← Defined above                │
│            └─────────────────┘                                  │
│                                                                 │
│  Arguments:                                                     │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  Arg 1 (callable): [lambda ▼]  →  [x > threshold]       │   │
│  │  Arg 2 (iterable): [items ▼]   ← Only iterables shown   │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  [Done - Add to Body]                                          │
└─────────────────────────────────────────────────────────────────┘
```

#### Result: Complete Function
```
┌─────────────────────────────────────────────────────────────────┐
│  Function: process_data                                         │
│  ├── params: [(items, List[int]), (threshold, int, default=10)]│
│  ├── returns: List[int]                                        │
│  ├── @Reclaim(Tracing)                                         │
│  └── body:                                                      │
│       └── Return                                                │
│            └── Call: list                                       │
│                 └── Call: filter                                │
│                      ├── Lambda: x > threshold                  │
│                      └── Var: items                             │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ Preview (Python):                                        │   │
│  │                                                          │   │
│  │ def process_data(items: List[int], threshold: int = 10): │   │
│  │     return list(filter(lambda x: x > threshold, items))  │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                 │
│  [Edit] [View as C++] [Add Annotation] [Done]                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Agent API: Scripted Choices

The structured editing UI has a direct JSON equivalent. Agents don't generate code text - they submit choice sequences.

### Human Interaction (UI)
```
Click: Function → Type: "process_data" → Click: Add Param → ...
```

### Agent Interaction (JSON)
```json
{
  "construct": "Function",
  "name": "process_data",
  "params": [
    { "name": "items", "type": "List[int]" },
    { "name": "threshold", "type": "int", "default": "10" }
  ],
  "return_type": "List[int]",
  "annotations": [
    { "type": "deref", "strategy": "batched" }
  ],
  "body": [
    {
      "statement": "Return",
      "expression": {
        "type": "Call",
        "function": "list",
        "args": [
          {
            "type": "Call",
            "function": "filter",
            "args": [
              {
                "type": "Lambda",
                "params": ["x"],
                "body": { "type": "BinaryOp", "left": "x", "op": ">", "right": "threshold" }
              },
              { "type": "VariableRef", "name": "items" }
            ]
          }
        ]
      }
    }
  ]
}
```

### Validation Guarantees

The Management Node validates each choice:

```
Agent submits: { "function": "nonexistent_func", ... }
System responds: {
  "error": "InvalidChoice",
  "field": "function",
  "valid_options": ["filter", "map", "process_data", "helper_func"],
  "reason": "Function 'nonexistent_func' not in scope"
}
```

**No parsing errors. No syntax errors. Only semantic validation.**

### Agent Workflow: Context-Aware Queuing

A local agent with project context can pre-compute likely choices:

```
┌─────────────────────────────────────────────────────────────────┐
│  Agent Context                                                  │
│  ├── Current file: data_processor.py                           │
│  ├── Cursor: inside class DataProcessor                        │
│  ├── In scope: self, items, config, logger                     │
│  └── Recent pattern: user adding validation methods            │
│                                                                 │
│  Agent predicts user wants: validation method                   │
│  Pre-queued choices:                                           │
│    1. Function (method)                                         │
│    2. Name: "validate_items" (inferred from context)           │
│    3. Param: items (already in scope)                          │
│    4. Return: bool (validation pattern)                        │
│    5. Body: If → condition → Return True/False                 │
│                                                                 │
│  User sees: "Create validate_items method? [Yes] [Modify] [No]"│
└─────────────────────────────────────────────────────────────────┘
```

Instead of:
```
Agent: "Here's the code I generated..."
Human: "Almost, but change X"
Agent: "Here's the updated code..."
Human: "Now there's a syntax error"
Agent: "Sorry, here's the fix..."
```

It becomes:
```
Agent: "I suggest these choices: [validate_items, bool, If pattern]"
Human: Clicks [Modify] → changes return type to Optional[str]
Agent: Updates downstream choices automatically
Human: Clicks [Accept]
Done. No back-and-forth. No errors.
```

---

## Elisp ↔ C++ Projection

### Why These Languages?

| Language | Role in Editor |
|----------|----------------|
| **Elisp** | Emacs server scripting, configuration, extensions |
| **C++** | Dear ImGui shell, performance-critical paths |

Building Elisp ↔ C++ transpilation lets us **dogfood** Whetstone:
- Write editor logic in Elisp (comfortable, existing packages)
- Generate C++ for performance-critical rendering
- Same AST, different projections

### Language Mapping

| Elisp | Whetstone AST | C++ |
|-------|---------------|-----|
| `(defun name (args) body)` | Function node | `RetType name(Args) { body }` |
| `(let ((x val)) body)` | Variable + Block | `{ auto x = val; body }` |
| `(if cond then else)` | IfStatement | `if (cond) { then } else { else }` |
| `(lambda (x) body)` | Lambda | `[](auto x) { return body; }` |
| `(mapcar fn list)` | Call: map | `std::transform(...)` |
| `'(1 2 3)` | ListLiteral | `std::vector{1, 2, 3}` |

### Elisp-Specific Idioms

```
@lang_specific(elisp, "special_form", "(interactive \"sPrompt: \")", hint="user_input")
@lang_specific(elisp, "macro", "(with-temp-buffer ...)", hint="scoped_resource")
@lang_specific(elisp, "advice", "(advice-add 'fn :around ...)", hint="aspect_oriented")
```

### C++-Specific Idioms

```
@lang_specific(cpp, "template", "template<typename T>", hint="generic")
@lang_specific(cpp, "raii", "std::lock_guard<std::mutex>", hint="scoped_resource")
@lang_specific(cpp, "constexpr", "constexpr", hint="compile_time")
```

---

## Dear ImGui Integration

### What is Dear ImGui?

Immediate-mode GUI library. Every frame, you declare what to draw:

```cpp
// C++ Dear ImGui
void render() {
    if (ImGui::Button("Compile")) {
        runCompile();
    }
    ImGui::SliderInt("Opt Level", &optLevel, 0, 3);
    ImGui::Text("Status: %s", status.c_str());
}
```

### Elisp Wrapper (whetstone-imgui.el)

```elisp
;; Elisp Dear ImGui bindings
(defun render ()
  (imgui-button "Compile" #'run-compile)
  (imgui-slider-int "Opt Level" 'opt-level 0 3)
  (imgui-text "Status: %s" status))
```

### AST-Defined UI

The Whetstone AST can define UI elements:

```
UIPanel: "Build Settings"
├── @projection: imgui
├── children:
│   ├── UIButton
│   │   ├── label: "Compile"
│   │   ├── icon: "hammer"
│   │   └── action: → BuildSystem.compile()
│   ├── UISlider
│   │   ├── label: "Optimization Level"
│   │   ├── binding: → config.optLevel
│   │   ├── min: 0
│   │   └── max: 3
│   └── UIText
│       ├── template: "Status: {}"
│       └── binding: → BuildSystem.status
```

**Same AST, multiple UI projections:**

| Projection | Output |
|------------|--------|
| `@projection(imgui)` | Native Dear ImGui widgets |
| `@projection(emacs)` | Text-based `[Compile] Opt: [===----] 2` |
| `@projection(html)` | Web form with `<button>`, `<input type="range">` |
| `@projection(tui)` | ncurses terminal UI |

---

## Implementation Phases

### Phase 2a: Editor Shell (4 weeks)

| Week | Deliverable |
|------|-------------|
| 1 | Dear ImGui scaffold with file tree, editor pane, command palette |
| 2 | Management Node: spawn/manage headless Emacs servers |
| 3 | IPC: route commands ImGui ↔ Management ↔ Emacs |
| 4 | Integration: Helm/Vertico search, file operations via TRAMP |

**Exit Criteria:**
- [ ] Open files via Emacs file dialog
- [ ] Search with Consult/Helm results rendered in ImGui
- [ ] Basic text editing (Emacs buffers displayed in ImGui)

### Phase 2b: Structured Editing (4 weeks)

| Week | Deliverable |
|------|-------------|
| 5 | Structured editing panel: construct chooser UI |
| 6 | Field editors: constrained inputs for names, types, expressions |
| 7 | Scope awareness: dropdown shows only valid in-scope options |
| 8 | Agent API: JSON choice submission, validation responses |

**Exit Criteria:**
- [ ] Create function via UI without typing syntax
- [ ] Agent submits JSON → valid AST created
- [ ] Invalid choices rejected with helpful errors

### Phase 2c: Elisp ↔ C++ (4 weeks)

| Week | Deliverable |
|------|-------------|
| 9 | Elisp tree-sitter parser integration |
| 10 | Elisp projection (AST → Elisp text) |
| 11 | C++ projection improvements for Elisp idiom mapping |
| 12 | Round-trip testing: Elisp → AST → C++ → AST → Elisp |

**Exit Criteria:**
- [ ] Parse whetstone-bridge.el into AST
- [ ] Generate equivalent C++ for performance-critical functions
- [ ] Elisp `@lang_specific` annotations preserved in C++ as comments

---

## Technical Decisions

### Management Node: Why C++?

| Option | Pros | Cons |
|--------|------|------|
| **C++** | Same as ImGui, no FFI boundary, mature | Memory safety requires discipline |
| Rust | Memory safe, good IPC libs | Another language in stack |
| Elisp | Single ecosystem | Performance, threading limitations |

C++ provides:
- **Zero FFI overhead** with Dear ImGui (same language)
- Mature threading primitives for managing multiple Emacs servers
- Excellent JSON libraries (nlohmann/json, rapidjson)
- Direct integration with the rendering layer

**Dogfooding path:** Once Whetstone matures, the Management Node becomes our first major dogfooding target. We'll define its logic in Whetstone AST and generate C++ from it. This proves the system works for real software, not just toy examples.

### IPC: JSON-RPC over Unix Sockets

```
ImGui Shell ←──JSON-RPC──→ Management Node ←──emacsclient──→ Emacs Servers
```

**Why JSON-RPC:**
- Language agnostic
- Human readable (debuggable)
- Well-supported in C++ and Elisp
- LSP uses it (familiar pattern)

### Emacs Server Specialization

| Server | Packages Loaded | Purpose |
|--------|-----------------|---------|
| `file-server` | TRAMP, dired, recentf | File operations |
| `search-server` | Consult, ripgrep, fd | Fast search |
| `complete-server` | Company, Corfu, Cape | Completion |
| `git-server` | Magit, diff-hl | Version control |
| `org-server` | Org-mode, org-roam | Documentation |

**Benefit:** Each server loads only what it needs. Parallel operations don't block each other.

---

## Risks & Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Emacs IPC latency | Sluggish UI | Async commands, local caching |
| ImGui learning curve | Slow UI development | Start simple, iterate |
| Elisp ↔ C++ semantic gap | Lossy transpilation | `@lang_specific` annotations |
| Scope complexity | Wrong options offered | Conservative scoping, user override |
| Agent choice explosion | Too many options | Context-aware filtering, ML ranking |

---

## Success Metrics

| Metric | Target |
|--------|--------|
| File open latency | < 100ms (via Emacs server) |
| Search result latency | < 50ms first results |
| Structured edit: construct creation | < 10 clicks for simple function |
| Agent choice acceptance rate | > 80% first suggestion accepted |
| Syntax errors (structured mode) | 0 (by design) |
| Elisp → C++ round-trip fidelity | Semantically equivalent |

---

## Open Questions for Sprint 2

1. **Emacs package compatibility** - Which GUI-dependent packages (if any) are critical? Can they be adapted?

2. **Structured editing escape hatch** - Should users be able to drop into raw text mode? When?

3. **Agent trust levels** - Can agents auto-accept choices, or always require human confirmation?

4. **Incremental adoption** - Can structured editing work alongside traditional text editing in same file?

5. **Performance budget** - What's acceptable latency for Emacs server round-trips?

---

## Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| Dear ImGui | 1.90+ | GUI rendering |
| Emacs | 29+ | Headless servers |
| nlohmann/json | 3.11+ | JSON-RPC in C++ |
| tree-sitter | 0.22+ | Elisp parsing |
| tree-sitter-elisp | latest | Elisp grammar |
| C++ compiler | C++20 | Management node, ImGui shell |

---

## Relationship to Sprint 1

Sprint 1 (MPS) provides:
- Core AST node definitions
- SemAnno annotation schema
- Python ↔ C++ projection patterns
- Memory strategy annotation examples (canonical system: `@Deallocate`, `@Lifetime`, `@Reclaim`, `@Owner`, `@Allocate`)

Sprint 2 consumes Sprint 1 by:
- Porting AST definitions from MPS to C++ classes (initially)
- Reusing annotation schema for structured editing constraints
- Extending projections to include Elisp
- Building the editor that will eventually replace MPS for Whetstone development

**The Bootstrap Path:**
```
Sprint 1: MPS defines AST → generates Python/C++
Sprint 2: C++ editor (hand-written) uses AST definitions
Sprint 3: Editor logic defined IN Whetstone AST → generates C++
Sprint 4: Self-hosting complete - Whetstone builds Whetstone
```
