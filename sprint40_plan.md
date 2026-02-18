# Sprint 40 Plan: HiveMind Integration — Whetstone + Nexus as One System

## Context

Sprint 40 is the convergence sprint. Whetstone and HiveMind become one integrated
system: the editor dispatches jobs to the swarm, monitors drone status, browses
the apiary, sees the energy state, and acts as the Pilot seat. Active Inference
closes the loop — the editor detects entropy and feeds it back to the swarm.

This is the "system is complete" milestone.

Primary goals:
1. Editor dispatches jobs directly to the HiveMind nexus
2. Live swarm status, apiary browser, and energy context panels
3. Active Inference entropy scanner in the editor
4. Pilot queue panel for human-in-the-loop swarm steering
5. Cross-session context bridge: editor ↔ HiveMind ↔ drone ↔ editor

---

## Phase 40a: Nexus Integration (Steps 654–658)

### Step 654: HiveMind job publisher from editor (12 tests)
New MCP tool: `whetstone_dispatch_to_hivemind`. Input: taskitem JSON (AnnotatedTaskitem
format). Writes job to nexus SQLite + publishes to borg/jobs/pending. Editor can
now feed the swarm directly — architect → Whetstone → HiveMind → drone.

### Step 655: Job status subscriber panel (12 tests)
New editor panel: "Swarm Status". Subscribes to MQTT topics for job updates.
Shows: pending jobs, claimed by which node, running duration, completed/failed.
Live refresh. Editor becomes the Pilot seat for the HiveMind.

### Step 656: Apiary browser panel (12 tests)
Browse /mnt/nas/swarm_tools/ from within the editor. Shows: tool name, version,
language, capabilities, doc summary. Click to load tool source into editor buffer.
Search by capability. Lets architect see what the swarm has already built.

### Step 657: Energy context status bar (12 tests)
Subscribe to borg/energy/context. Show current energy state in editor status bar:
`HighSolar 450W` or `Conservation 28%`. Flashes orange in conservation mode.
Agent context injection: include energy state in system prompt so agent knows
whether to suggest GPU-heavy or CPU-light implementations.

### Step 658: Phase 40a Integration (8 tests)
Full loop: architect opens editor → sees energy state → writes spec → dispatches
job to HiveMind → watches job status panel → drone completes → result appears in
apiary browser → architect loads result into buffer.

---

## Phase 40b: Active Inference Bridge (Steps 659–663)

### Step 659: Entropy scanner — editor-side (12 tests)
Editor scans open workspace: detect duplicate function signatures, similar module
names, unused exports. Score: entropy count. If above threshold, suggest:
"Consider extracting common logic — generate refactor job?" User can dispatch
the suggestion directly to HiveMind.

### Step 660: `whetstone_generate_inference_job` MCP tool (12 tests)
Given an entropy observation (duplication, gap, staleness), generate a properly
formatted HiveMind Active Inference job: {type: "refactor", goal: "...", context:
{files, entropy_score}, bounty: "normal"}. Write to nexus.

### Step 661: Pilot queue panel (12 tests)
A dedicated panel showing jobs flagged for human review (escalate=true, or
ambiguity > threshold). Jobs arrive via MQTT subscription to a pilot-specific
topic. Human can: approve (dispatch to swarm), reject (discard), modify (edit
task in editor), inject strategy (add a free-text guidance note).

### Step 662: Cross-session context bridge (12 tests)
When the HiveMind dispatches an agent_task job with context_files, the drone
can launch whetstone_mcp --workspace and call whetstone_architect_intake on
those files. Output feeds back to the editor session that dispatched the job.
Closes the loop: editor ↔ HiveMind ↔ drone ↔ editor.

### Step 663: Sprint 40 Integration + Final Summary (8 tests)
The complete system: Whetstone editor + HiveMind nexus + drone workers + apiary
+ active inference + pilot seat. One integrated system. Sprint 40 is the
"system is complete" milestone.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 40a | 654–658 | 56 | Nexus dispatch + swarm status + apiary + energy |
| 40b | 659–663 | 56 | Active inference + pilot queue + context bridge |
| **Total** | **654–663** | **~112** | 10 steps |

---

## Program Summary: Sprints 36–40

| Sprint | Steps | Theme |
|--------|-------|-------|
| 36 | 614–623 | MCP expansion + architect intake tools |
| 37 | 624–633 | In-editor agent chat panel |
| 38 | 634–643 | HiveMind build support (Rust, MQTT, SQLite generators) |
| 39 | 644–653 | Self-hosting + distribution pipeline |
| 40 | 654–663 | HiveMind full integration + Pilot seat |

**Total new steps:** 50
**Total tests (est.):** ~550
**System state after Sprint 40:** Whetstone + HiveMind = one integrated agentic system
