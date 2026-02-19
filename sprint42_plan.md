# Sprint 42 Plan: MCP Modeling Tools — Wire Unregistered Generators

## Context

Sprint 41 wired `whetstone_schema_to_cpp` and `whetstone_generate_dispatch_table`
(Steps 664-668, tool count 82→84). During that audit, two more existing classes
were found unregistered:

- `whetstone_generate_project` (`ProjectSkeletonGenerator`, Step 639)
- `whetstone_generate_inference_job` (`InferenceJobGenerator`, Step 660)

Both C++ classes are complete and tested. Both have `toolName()` methods. This
sprint is pure plumbing — wire them into the live MCP server following the Sprint 41
pattern. No new logic is written.

---

## Existing C++ Implementations (Do Not Rewrite)

| Class | File | Step | Interface |
|-------|------|------|-----------|
| `ProjectSkeletonGenerator` | `editor/src/ProjectSkeletonGenerator.h` | 639 | `::generate(name, description, dependencies)` → `GeneratedProjectSkeleton{success, cmakeLists, mainCpp, moduleHeaders, dependencies, errors}`. Use `::asToolResponse(out)` to convert to JSON for MCP return. |
| `InferenceJobGenerator` | `editor/src/InferenceJobGenerator.h` | 660 | `::generate(goal, entropyScore, files, bounty)` → `json` directly (no `asToolResponse` needed — already returns JSON). |

Both `toolName()` methods already return the correct MCP tool name strings.

---

## Review Notes (from Sprint 41 agent review)

- Original plan did not specify the `step672_test.cpp` file for the smoke test step.
  This sprint follows the pattern established in Sprint 41 Step 667: a test file is
  written for the rebuild+smoke step using the MCP Content-Length framing protocol.
- `InferenceJobGenerator::generate()` returns `json` directly; no `asToolResponse`
  wrapper is needed (unlike `ProjectSkeletonGenerator`).
- Call order in `registerWhetstoneTools()`: add `registerModelingTools()` after
  `registerCodegenTools()` — do not rearrange existing calls.
- Architecture invariant: header-only, 600-line limit, no new vcpkg dependencies.

---

## Wiring Pattern

Identical to Sprint 41 (`RegisterCodegenTools.h`):

1. Create `editor/src/mcp/RegisterModelingTools.h`
2. Push tool definitions to `tools_`, assign lambdas to `toolHandlers_`
3. `#include "mcp/RegisterModelingTools.h"` in `MCPServer.h` (after RegisterCodegenTools.h)
4. `registerModelingTools();` in `registerWhetstoneTools()` in `RegisterOnboardingAndAllTools.h`
5. Add two entries to `tools/claude/tools.json` (tool count 84→86)
6. Rebuild `whetstone_mcp` and smoke test both tools

---

## Steps

### Step 669: `RegisterModelingTools.h` — wire `whetstone_generate_project` (12 tests)

Create `editor/src/mcp/RegisterModelingTools.h`. Register `whetstone_generate_project`.

Input schema (all args → `ProjectSkeletonGenerator::generate(name, description, dependencies)`):
```json
{
  "name":         { "type": "string",  "description": "Project name (required)." },
  "description":  { "type": "string",  "description": "Short project description." },
  "dependencies": { "type": "array",   "description": "CMake find_package dependencies.",
                    "items": { "type": "string" } }
}
```
Required: `["name", "description"]`

Handler: call `ProjectSkeletonGenerator::generate(...)`, return
`ProjectSkeletonGenerator::asToolResponse(out)` as the MCP response JSON.

Tests (12): name required (empty name → error), description required, valid inputs
succeed, cmakeLists non-empty on success, mainCpp non-empty on success, moduleHeaders
non-empty on success, single dependency appears in cmakeLists, multiple dependencies
all appear, toolName() == "whetstone_generate_project", success==false on empty name,
errors non-empty on failure, success==true on valid minimal input.

### Step 670: `RegisterModelingTools.h` — wire `whetstone_generate_inference_job` (12 tests)

Add `whetstone_generate_inference_job` to the same `RegisterModelingTools.h`.

Input schema:
```json
{
  "goal":          { "type": "string",  "description": "Refactor goal description (required)." },
  "entropy_score": { "type": "integer", "description": "Entropy score from entropy scanner (required, ≥0)." },
  "files":         { "type": "array",   "description": "Source files relevant to this job.",
                     "items": { "type": "string" } },
  "bounty":        { "type": "string",  "description": "Bounty level: low|normal|high (default: normal)." }
}
```
Required: `["goal", "entropy_score", "files"]`

Handler: parse args, call `InferenceJobGenerator::generate(goal, entropy_score, files, bounty)`.
Return result directly (it is already a json object with `success`, `job`, etc.).

Tests (12): goal required, entropy_score required, files required, valid inputs succeed,
negative entropy_score → success==false, empty goal → success==false, result.job.type=="refactor",
result.job.goal matches input, result.job.context.files matches input, bounty defaults to
"normal", explicit bounty "high" reflected in output, toolName()=="whetstone_generate_inference_job".

### Step 671: Wire into `MCPServer.h` + `RegisterOnboardingAndAllTools.h` + `tools.json` (8 tests)

**MCPServer.h:** add `#include "mcp/RegisterModelingTools.h"` after `RegisterCodegenTools.h`.

**RegisterOnboardingAndAllTools.h:** add `registerModelingTools();` after `registerCodegenTools();`.

**tools/claude/tools.json:** add two entries (tool count 84→86):
```json
{
  "name": "whetstone_generate_project",
  "description": "Generate a C++ project skeleton from a name, description, and dependencies. Returns CMakeLists.txt content, main.cpp stub, and module header paths.",
  "input_schema": {
    "type": "object",
    "properties": {
      "name":         { "type": "string",  "description": "Project name." },
      "description":  { "type": "string",  "description": "Short project description." },
      "dependencies": { "type": "array",   "items": { "type": "string" } }
    },
    "required": ["name", "description"]
  }
},
{
  "name": "whetstone_generate_inference_job",
  "description": "Generate a HiveMind inference job payload from an entropy observation. Returns a structured job suitable for dispatch to the nexus.",
  "input_schema": {
    "type": "object",
    "properties": {
      "goal":          { "type": "string"  },
      "entropy_score": { "type": "integer" },
      "files":         { "type": "array",  "items": { "type": "string" } },
      "bounty":        { "type": "string"  }
    },
    "required": ["goal", "entropy_score", "files"]
  }
}
```

Tests (8): tools.json valid JSON, whetstone_generate_project in tools.json,
whetstone_generate_inference_job in tools.json, tool count == 86, no duplicate names,
both tools have input_schema.required, MCPServer includes RegisterModelingTools.h,
registerWhetstoneTools() calls registerModelingTools().

### Step 672: Rebuild `whetstone_mcp` + smoke test (8 tests)

Build:
```bash
cmake -S editor -B editor/build-native
cmake --build editor/build-native --target whetstone_mcp
```

Write `editor/tests/step672_test.cpp`. Test via MCP Content-Length framing protocol
(same pattern as step667_test.cpp from Sprint 41):

1. `tools/list` → verify both new tool names present, total count == 86
2. `whetstone_generate_project` call with `{name:"MyApp", description:"Example app", dependencies:["nlohmann_json"]}` → verify success=true, cmakeLists non-empty
3. `whetstone_generate_inference_job` call with `{goal:"extract duplicate handlers", entropy_score:3, files:["main.cpp"]}` → verify success=true, job.type=="refactor"

Tests (8): binary builds, binary runs on --help, tools/list is valid JSON, generate_project
in tools/list, generate_inference_job in tools/list, generate_project call success=true,
generate_inference_job call success=true, binary exits cleanly.

### Step 673: Sprint 42 Integration Summary (8 tests)

Create `editor/src/Sprint42IntegrationSummary.h` (model on Sprint41IntegrationSummary.h).
Record: tool_count_before=84, tool_count_after=86, steps_completed=5 (669-673),
files_added=[RegisterModelingTools.h, Sprint42IntegrationSummary.h],
files_modified=[MCPServer.h, RegisterOnboardingAndAllTools.h, tools/claude/tools.json].

Tests (8): struct constructable, generate_project_wired, generate_inference_job_wired,
tool_count_before==84, tool_count_after==86, steps_completed==5,
files_added contains RegisterModelingTools.h, success==true.

---

## Architecture Gate (All Steps)

- Header-only for all new registration and summary files
- 600-line max per file — `RegisterModelingTools.h` will be well under
- No new vcpkg dependencies
- Build target: `whetstone_mcp` must rebuild without new warnings
- Call order in `registerWhetstoneTools()` must not rearrange existing calls
- Full sprint regression at step 673: all steps 669-673 pass
