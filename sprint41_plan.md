# Sprint 41 Plan: MCP Codegen Tools — Wire Missing Plumbing

## Context

Sprint 41 is a focused plumbing sprint. Two C++ classes have been implemented
(Steps 634, 642) but were never registered as MCP tools. Both exist in the
generator layer but are invisible to Claude Code.

This sprint makes them callable:
- `whetstone_schema_to_cpp` — JSON schema → typed C++ struct + nlohmann serializers
- `whetstone_generate_dispatch_table` — job entry specs → C++ dispatch table header

Use case for HiveMind: the drone binary (`hivemind/drone/src/`) currently has
hand-scaffolded structs and dispatch tables. These tools let an agent regenerate
them from the canonical schemas and capabilities.json automatically.

---

## Existing C++ Implementations (Do Not Rewrite)

| Class | File | Step | Interface |
|-------|------|------|-----------|
| `SchemaToCppGenerator` | `editor/src/SchemaToCppGenerator.h` | 634 | `::generate(json schema, string headerName, string targetName)` → `SchemaToCppOutput{success, headerCode, cmakeInterfaceTarget, errors[]}` |
| `JobDispatchTableGenerator` | `editor/src/JobDispatchTableGenerator.h` | 642 | `::generate(vector<JobDispatchEntrySpec>)` → `JobDispatchTableOutput{success, headerCode, errors[]}` |

`SchemaToCppGenerator::toolName()` already returns `"whetstone_schema_to_cpp"`.

---

## Wiring Pattern (copy from Sprint 36)

The pattern is identical to `RegisterArchitectIntakeTools.h`:

1. Create `editor/src/mcp/RegisterCodegenTools.h`
   - `#include` both generator headers
   - Define `registerCodegenTools()` method (same style as `registerArchitectIntakeTools()`)
   - Push tool definition to `tools_` vector
   - Assign lambda to `toolHandlers_[name]`

2. Add `#include "mcp/RegisterCodegenTools.h"` to `MCPServer.h`
   (after line 550, alongside other Register includes)

3. Add `registerCodegenTools();` call in `registerWhetstoneTools()` inside
   `RegisterOnboardingAndAllTools.h` (after `registerArchitectIntakeTools()`)

4. Add two entries to `tools/claude/tools.json`

5. Rebuild: `cmake --build editor/build-native --target whetstone_mcp`

---

## Steps

### Step 664: `RegisterCodegenTools.h` — whetstone_schema_to_cpp (12 tests)

Create `editor/src/mcp/RegisterCodegenTools.h`. Wire `SchemaToCppGenerator`.

Input schema for the MCP tool:
```json
{
  "schema":      { "type": "object",  "description": "JSON Schema object with 'title' and 'properties'." },
  "header_name": { "type": "string",  "description": "Output header filename, e.g. 'EnergyContext.h'." },
  "target_name": { "type": "string",  "description": "CMake INTERFACE target name, e.g. 'energy_context_types'." }
}
```

Output (returned as JSON):
```json
{
  "success": true,
  "header_code": "...",
  "cmake_interface_target": "...",
  "errors": []
}
```

Tests (12): valid schema produces struct + serializers, schema without title
errors cleanly, empty properties produces `raw` field fallback, integer/bool/number
types map correctly, sanitizeIdentifier strips special chars, cmakeInterfaceTarget
is non-empty on success, invalid (non-object) schema errors, multi-field struct
round-trips via from_json/to_json, MCP handler returns success=false on bad input,
handler returns success=true on minimal valid schema, headerCode contains
`#pragma once`, toolName() == "whetstone_schema_to_cpp".

### Step 665: `RegisterCodegenTools.h` — whetstone_generate_dispatch_table (12 tests)

Add `whetstone_generate_dispatch_table` to the same `RegisterCodegenTools.h`.

`JobDispatchEntrySpec` fields: `jobType` (string), `requiredCaps` (array of string),
`payloadType` (string), `executor` (string — C++ function name or lambda expression).

Input schema for the MCP tool:
```json
{
  "entries": {
    "type": "array",
    "description": "Array of {job_type, required_caps[], payload_type, executor} objects.",
    "items": {
      "type": "object",
      "properties": {
        "job_type":      { "type": "string" },
        "required_caps": { "type": "array", "items": { "type": "string" } },
        "payload_type":  { "type": "string" },
        "executor":      { "type": "string", "description": "C++ callable expression, e.g. 'handleCudaTask'" }
      },
      "required": ["job_type", "executor"]
    }
  }
}
```

Output:
```json
{
  "success": true,
  "header_code": "...",
  "errors": []
}
```

Tests (12): single entry produces valid C++ header, multiple entries all appear
in output, empty entries[] errors with entries_required, missing executor errors,
headerCode contains `#pragma once` and `makeDispatchTable`, requiredCaps appear
as comments in output, MCP handler parses json array of objects correctly,
handler returns success=false on bad input, output is parseable as C++ header
(contains `{` and `}`), job_type appears as string key in table, executor appears
as value in table, roundtrip: parse output → verify jobType string present.

### Step 666: Wire into MCPServer.h + RegisterOnboardingAndAllTools.h + tools.json (8 tests)

**Files modified:**
- `editor/src/MCPServer.h` — add `#include "mcp/RegisterCodegenTools.h"` after line 550
- `editor/src/mcp/RegisterOnboardingAndAllTools.h` — add `registerCodegenTools();` in `registerWhetstoneTools()`
- `tools/claude/tools.json` — add two tool entries (see format below)

**tools.json entries to add** (match existing entry format exactly):
```json
{
  "name": "whetstone_schema_to_cpp",
  "description": "Generate a typed C++ header struct with nlohmann JSON serializers from a JSON Schema object. Returns header_code (ready to write to a .h file) and a cmake_interface_target snippet.",
  "input_schema": {
    "type": "object",
    "properties": {
      "schema":      { "type": "object",  "description": "JSON Schema with 'title' and 'properties'." },
      "header_name": { "type": "string",  "description": "Output header filename, e.g. 'EnergyContext.h'." },
      "target_name": { "type": "string",  "description": "CMake INTERFACE target name." }
    },
    "required": ["schema", "header_name", "target_name"]
  }
},
{
  "name": "whetstone_generate_dispatch_table",
  "description": "Generate a C++ dispatch table header (makeDispatchTable()) from a list of job type specs. Each entry maps a job_type string to a C++ executor callable.",
  "input_schema": {
    "type": "object",
    "properties": {
      "entries": {
        "type": "array",
        "description": "Array of job dispatch entries.",
        "items": {
          "type": "object",
          "properties": {
            "job_type":      { "type": "string" },
            "required_caps": { "type": "array", "items": { "type": "string" } },
            "payload_type":  { "type": "string" },
            "executor":      { "type": "string" }
          },
          "required": ["job_type", "executor"]
        }
      }
    },
    "required": ["entries"]
  }
}
```

Tests (8): MCPServer initializes without error with new includes, `registerWhetstoneTools()`
calls `registerCodegenTools()`, tools.json entries are valid JSON, `whetstone_schema_to_cpp`
present in tools.json, `whetstone_generate_dispatch_table` present in tools.json,
tool count increases by 2 from previous sprint, MCPServer tools_ vector contains both
new tool names, handlers map contains both new tool names.

### Step 667: Rebuild whetstone_mcp + smoke test (8 tests)

Build the MCP binary and verify both new tools respond correctly over stdio.

```bash
cmake -S editor -B editor/build-native
cmake --build editor/build-native --target whetstone_mcp
```

Smoke test via stdin/stdout (MCP protocol):
1. Send `tools/list` — verify both new tool names appear
2. Send `tools/call` for `whetstone_schema_to_cpp` with the `energy_context.schema.json`
   from `hivemind/schemas/` — verify success=true, header_code non-empty
3. Send `tools/call` for `whetstone_generate_dispatch_table` with the 4 HiveMind job
   types (agent_swarm, cuda_task, compile_job, shell_script) — verify success=true

Tests (8): binary builds without error, binary runs without crash on `--help`,
tools/list response is valid JSON, tools/list contains whetstone_schema_to_cpp,
tools/list contains whetstone_generate_dispatch_table, schema_to_cpp smoke call
returns success=true, dispatch_table smoke call returns success=true, binary
exits cleanly on SIGTERM.

### Step 668: Sprint 41 Integration Summary (8 tests)

Create `editor/src/Sprint41IntegrationSummary.h`.
Record: steps completed, tools added (count before/after), files modified,
regression check result.

Tests (8): summary struct is constructable, tool_count_before == 82,
tool_count_after == 84, steps_completed == 4, files_modified contains
RegisterCodegenTools.h, files_modified contains MCPServer.h,
files_modified contains tools.json, success == true.

---

## Architecture Gate (applies to all steps)

- All new code in `editor/src/mcp/RegisterCodegenTools.h` — header-only
- File must stay under 600 lines
- No new external dependencies
- Binary rebuild must pass without warnings
- Test count per step: as specified above (not a fixed template)
