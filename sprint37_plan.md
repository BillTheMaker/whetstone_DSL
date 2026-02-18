# Sprint 37 Plan: In-Editor Agent Chat Panel

## Context

Sprint 37 builds the "full window in the editor" agent UX — the plug-and-play
experience for non-technical users. Agent chat embedded directly in the ImGui
editor with live tool call visualization, mutation preview, and accept/reject
controls. No terminal required.

Primary goals:
1. Embedded chat panel with message history and streaming responses
2. Live tool call visualization and AST mutation preview
3. Human-in-the-loop accept/reject for every agent mutation
4. Context auto-injection and session persistence

---

## Phase 37a: Chat Window Core (Steps 624–628)

### Step 624: Agent Chat Panel model (12 tests)
New panel: message history (scrollable), text input (multi-line), send button.
Messages: role (user/assistant/tool), content, timestamp. State lives in
AgentChatState sub-struct. Renders via ImGui but shares no state with EditorState
globals.

### Step 625: Tool call visualization (12 tests)
When Claude calls a Whetstone tool, render it inline in the chat as a collapsible
block: `[TOOL: whetstone_mutate] ▶ setProperty fn1.name → "newName"`. Show
input/output JSON. Lets user see exactly what the agent is doing.

### Step 626: AST mutation preview (12 tests)
Before applying any agent-requested mutation: show a side-by-side diff of the
generated code before vs after. User sees the change before it's committed.
Requires generating code from both old and new AST.

### Step 627: Inline accept/reject controls (12 tests)
Under each mutation preview: [Accept] [Reject] [Modify]. Accept applies the
mutation. Reject discards it and posts "rejected: <reason>" back to the agent.
Modify opens the mutation in an editable JSON view. Human-in-the-loop at every
agent action.

### Step 628: Phase 37a Integration (8 tests)
Full chat flow: user types spec → agent calls whetstone_generate_code → mutation
preview shown → user accepts → AST updated → code view refreshes.

---

## Phase 37b: Context + Persistence (Steps 629–633)

### Step 629: Project context auto-injection (12 tests)
On chat start, auto-inject a context block into the system prompt: active file
name, language, AST node count, open annotations count, workspace path. The agent
starts every conversation knowing where it is.

### Step 630: Chat session persistence (12 tests)
Save/restore chat sessions per project file. Format: JSON alongside .whetstone.json.
Sessions indexed by timestamp. Reload on next open. User can continue a previous
conversation without losing context.

### Step 631: Background task slots (parallel agent work) (12 tests)
Run up to N agent tasks in parallel (N from project config). Each slot has its
own chat thread. Slots shown as tabs in the chat panel. Lets user kick off
"generate the test suite" while also asking "explain this function."

### Step 632: Agent task status overlay (12 tests)
Status bar item: `[Agent: 2 running, 1 pending]`. Click to open task panel.
Each task shows: description, elapsed time, tool calls made, current step.
Cancel button per task.

### Step 633: Sprint 37 Integration + Summary (8 tests)
Demo scenario: open hivemind project → chat panel opens → paste BS-S1-01 task
description → agent generates drone/nexus code → user accepts mutations →
code panel shows generated Rust/C++ → chat session saved.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 37a | 624–628 | 56 | Chat window core + mutation preview |
| 37b | 629–633 | 56 | Context injection + session persistence |
| **Total** | **624–633** | **~112** | 10 steps |
