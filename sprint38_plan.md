# Sprint 38 Plan: HiveMind Build Support — Drone Generators + Shared Types

## Context

Sprint 38 makes Whetstone the tool that builds HiveMind. Everything is C++20
with CMake + vcpkg — the same toolchain as the editor itself. The drone binary
shares job schema types with Whetstone, and the generators produce the MQTT
client loops, SQLite schema layers, and dispatch tables that the drone needs.

Primary goals:
1. Shared C++ type library for job schemas (editor + drone use the same headers)
2. MQTT and SQLite boilerplate generators targeting C++ / paho / sqlite3
3. Full project skeleton generation from a description string

---

## Phase 38a: Shared Type Library + Drone Skeleton (Steps 634–638)

### Step 634: Job schema → typed C++ structs generator (12 tests)
New MCP tool: `whetstone_schema_to_cpp`. Input: JSON schema (the nexus job
payload schema). Output: C++ structs with nlohmann-json `from_json`/`to_json`
overloads, field validation, and a CMakeLists.txt INTERFACE target so both
the drone and Whetstone editor can include the same headers.

### Step 635: Capability declaration struct generator (12 tests)
Generate the C++ types for capability graphs: NodeCapability, CapabilitySet,
EnergyContext, JobRequirements. These are shared between drone (declares
capabilities) and the orchestrator (matches jobs). Output as a header-only
library following Whetstone's existing header-only pattern.

### Step 636: Error type synthesis for drone (12 tests)
Generate C++ error hierarchies for drone operations: MqttError, NexusError,
ExecutionError, CapabilityMismatch. Uses std::error_code / std::expected
patterns (C++23-compatible). Needed for drone's error handling across the
MQTT, SQLite, and executor layers.

### Step 637: Cross-platform CMake target generator (12 tests)
New annotation: @Platform(target = "aarch64-apple-darwin") on platform-specific
code blocks. Generator emits CMake `if(CMAKE_SYSTEM_PROCESSOR MATCHES ...)` guards
and compile-time `#ifdef` blocks. Lets one drone CMakeLists.txt build correctly
on Mac M2, x86_64 Linux, and N100 without per-platform forks.

### Step 638: Phase 38a Integration (8 tests)
Generate a skeleton drone binary: CMakeLists.txt, main.cpp entry, capability
declaration, shared type headers included, all targets building cleanly on the
host platform via vcpkg.

---

## Phase 38b: Infrastructure Generators (Steps 639–643)

### Step 639: `whetstone_generate_project` MCP tool (12 tests)
Generate an entire C++ project skeleton from a description string. Input: {name,
description, dependencies (vcpkg names)}. Output: CMakeLists.txt, main.cpp,
module stub headers, test scaffold following Whetstone's own architecture
conventions. The HiveMind drone bootstrap in one call.

### Step 640: MQTT pub/sub boilerplate generator (12 tests)
Given topic names + payload schemas, generate C++ MQTT client code using paho-mqtt
(already in vcpkg). Output: typed topic handler classes with nlohmann-json
deserialization, connection/reconnect logic, QoS config. Drone subscribes to
borg/energy/context, borg/jobs/pending, borg/registry/commit — one generator call.

### Step 641: SQLite schema → C++ data layer generator (12 tests)
Input: SQL CREATE TABLE statements (from hivemind/ARCHITECTURE.md nexus schema).
Output: C++ structs + sqlite3 helper classes with prepared statement wrappers,
CRUD methods, and transaction helpers. Generates the entire nexus DB layer
without hand-writing a single SQL string.

### Step 642: Job dispatch table generator (12 tests)
Input: job type registry (job_type → {required_caps, payload_schema, executor}).
Output: C++ dispatch table — std::unordered_map<std::string, ExecutorFn> with
type-safe payload parsing and executor function stubs. The drone's dispatch
table generated from one schema file.

### Step 643: Sprint 38 Integration + Summary (8 tests)
End-to-end: `whetstone_generate_project` for drone → MQTT generator → SQLite
generator → dispatch table → drone binary compiling cleanly via CMake + vcpkg
on all target platforms.

---

## Step & Test Summary

| Phase | Steps | Tests | Theme |
|-------|-------|-------|-------|
| 38a | 634–638 | 56 | Shared C++ types + capability structs + platform CMake |
| 38b | 639–643 | 56 | Project skeleton + MQTT + SQLite + dispatch generators |
| **Total** | **634–643** | **~112** | 10 steps |
