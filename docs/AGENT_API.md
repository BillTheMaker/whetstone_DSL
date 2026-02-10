# Whetstone Agent API Reference

## Overview

The Whetstone Agent API provides programmatic access to the Whetstone DSL compiler and analysis tools via a WebSocket-based JSON-RPC 2.0 interface. Agents can query AST structures, apply mutations, generate code, and leverage semantic analysis features.

**Protocol**: WebSocket with JSON-RPC 2.0
**Agent Roles**:
- **Linter**: Read-only access (queries, diagnostics)
- **Refactor**: Read + mutation operations
- **Generator**: Read + mutation + code generation

**Library Context**: Automatically sent on connection if a library provider is configured.

---

## Method Reference Table

| Method | Category | Required Role | Description |
|--------|----------|---------------|-------------|
| `ping` | Session | Any | Health check |
| `getSessionInfo` | Session | Any | Get session details |
| `setAgentName` | Session | Any | Set agent display name |
| `listSessions` | Session | Any | List all active sessions |
| `setAgentRole` | Role | Any | Set agent role (linter/refactor/generator) |
| `getAST` | AST | Any | Get full AST with annotations |
| `getInScopeSymbols` | Context | Any | Get symbols visible at node |
| `getCallHierarchy` | Context | Any | Get function call relationships |
| `getDependencyGraph` | Context | Any | Get node dependencies |
| `applyMutation` | Mutation | Refactor/Generator | Apply single AST mutation |
| `applyBatch` | Mutation | Refactor/Generator | Apply atomic batch mutations |
| `getAnnotationSuggestions` | Annotation | Any | Get ML annotation suggestions |
| `applyAnnotationSuggestion` | Annotation | Refactor/Generator | Apply suggested annotation |
| `recordAnnotationFeedback` | Annotation | Any | Record user feedback |
| `generateCode` | Generation | Refactor/Generator | Generate code from spec |
| `runPipeline` | Pipeline | Any | Full parse→validate→optimize pipeline |
| `parseSource` | Pipeline | Any | Parse source to AST |
| `generateFromAST` | Pipeline | Any | Generate code from AST |
| `projectLanguage` | Pipeline | Any | Project AST to different language |
| `startWorkflowRecording` | Workflow | Any | Start recording workflow |
| `stopWorkflowRecording` | Workflow | Any | Stop and get workflow |
| `getWorkflowRecording` | Workflow | Any | Get current workflow state |
| `replayWorkflow` | Workflow | Any | Replay recorded workflow |

---

## Quick Start

Here's a typical 5-step agent workflow:

```javascript
// 1. Connect to WebSocket
const ws = new WebSocket('ws://localhost:8080');

// 2. Set agent role
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 1,
  method: "setAgentRole",
  params: { role: "refactor" }
}));

// 3. Get AST
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 2,
  method: "getAST"
}));

// 4. Get annotation suggestions for a specific node
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 3,
  method: "getAnnotationSuggestions",
  params: { nodeId: "node_42" }
}));

// 5. Apply a suggestion
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 4,
  method: "applyAnnotationSuggestion",
  params: {
    nodeId: "node_42",
    annotationType: "performance",
    strategy: "cache",
    reason: "Repeated computation detected",
    confidence: 0.85,
    accepted: true
  }
}));
```

---

## Session Management

### ping

Health check to verify connection.

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "message": "pong",
    "sessionId": "sess_abc123"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "ping"
}
```

---

### getSessionInfo

Retrieve current session information.

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "sessionId": "sess_abc123",
    "agentName": "MyAgent",
    "connected": true,
    "connectedAtMs": 1707580800000,
    "lastMessageAtMs": 1707580900000,
    "messageCount": 42
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "getSessionInfo"
}
```

---

### setAgentName

Set a display name for the agent session.

**Params**:
- `name` (string, required): Agent display name

**Response**: Session info object

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "setAgentName",
  "params": {
    "name": "RefactorBot"
  }
}
```

**Example Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "sessionId": "sess_abc123",
    "agentName": "RefactorBot",
    "connected": true,
    "connectedAtMs": 1707580800000,
    "lastMessageAtMs": 1707580900000,
    "messageCount": 43
  }
}
```

---

### listSessions

List all active agent sessions.

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 4,
  "result": [
    {
      "sessionId": "sess_abc123",
      "agentName": "RefactorBot",
      "connected": true,
      "connectedAtMs": 1707580800000,
      "lastMessageAtMs": 1707580900000,
      "messageCount": 43
    },
    {
      "sessionId": "sess_def456",
      "agentName": "LinterAgent",
      "connected": true,
      "connectedAtMs": 1707581000000,
      "lastMessageAtMs": 1707581100000,
      "messageCount": 12
    }
  ]
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 4,
  "method": "listSessions"
}
```

---

## Role Management

### setAgentRole

Set the agent's role, which determines permissions.

**Params**:
- `role` (string, required): One of `"linter"`, `"refactor"`, or `"generator"`

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 5,
  "result": {
    "role": "refactor"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 5,
  "method": "setAgentRole",
  "params": {
    "role": "refactor"
  }
}
```

**Role Capabilities**:
- **linter**: Read-only (getAST, queries, diagnostics)
- **refactor**: Read + mutations (applyMutation, applyBatch, applyAnnotationSuggestion)
- **generator**: Read + mutations + code generation (generateCode, projectLanguage)

---

## AST Access

### getAST

Retrieve the full Abstract Syntax Tree of the active structured buffer.

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 6,
  "result": {
    "ast": {
      "type": "Program",
      "id": "node_1",
      "children": [
        {
          "type": "FunctionDeclaration",
          "id": "node_2",
          "name": "calculateTotal",
          "parameters": [],
          "body": {
            "type": "Block",
            "id": "node_3",
            "statements": []
          }
        }
      ]
    },
    "annotationCount": 5,
    "diagnostics": [
      {
        "severity": "warning",
        "message": "Unused variable 'x'",
        "nodeId": "node_7",
        "line": 10,
        "column": 5
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 6,
  "method": "getAST"
}
```

**Errors**:
- `-32000`: No structured buffer active
- `-32001`: AST unavailable

---

## Context Queries

### getInScopeSymbols

Get all symbols visible at a specific AST node (variables, functions, types).

**Params**:
- `nodeId` (string, required): Target node ID

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 7,
  "result": {
    "symbols": [
      {
        "name": "calculateTotal",
        "kind": "function",
        "nodeId": "node_2"
      },
      {
        "name": "price",
        "kind": "variable",
        "nodeId": "node_15"
      },
      {
        "name": "Order",
        "kind": "type",
        "nodeId": "node_8"
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 7,
  "method": "getInScopeSymbols",
  "params": {
    "nodeId": "node_42"
  }
}
```

**Available to**: All roles

---

### getCallHierarchy

Get the call hierarchy for a function (callers and callees).

**Params**:
- `functionId` (string, required): Function node ID

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 8,
  "result": {
    "functionId": "node_2",
    "functionName": "calculateTotal",
    "callerIds": ["node_50", "node_61"],
    "calleeIds": ["node_15", "node_23"]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 8,
  "method": "getCallHierarchy",
  "params": {
    "functionId": "node_2"
  }
}
```

**Available to**: All roles

---

### getDependencyGraph

Get the dependency graph for a node (declarations it depends on).

**Params**:
- `nodeId` (string, required): Target node ID

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 9,
  "result": {
    "dependencies": ["node_8", "node_15", "node_23"]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 9,
  "method": "getDependencyGraph",
  "params": {
    "nodeId": "node_42"
  }
}
```

**Available to**: All roles

---

## Mutations

### applyMutation

Apply a single mutation to the AST.

**Params**:
- `type` (string, required): One of `"setProperty"`, `"updateNode"`, `"deleteNode"`, `"insertNode"`
- `nodeId` (string, required for setProperty/updateNode/deleteNode): Target node ID
- `property` (string, required for setProperty): Property name to set
- `value` (any, required for setProperty): Property value
- `parentId` (string, required for insertNode): Parent node ID
- `role` (string, required for insertNode): Role in parent (e.g., "children", "parameters")
- `node` (object, required for insertNode/updateNode): New AST node
- `properties` (object, optional for updateNode): Properties to update
- `preferImports` (boolean, optional): Prefer library imports over inline code
- `strictMode` (boolean, optional): Enable strict validation

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 10,
  "result": {
    "success": true,
    "warning": null,
    "libraryWarning": null,
    "unknownFunctions": []
  }
}
```

**Example Request (setProperty)**:
```json
{
  "jsonrpc": "2.0",
  "id": 10,
  "method": "applyMutation",
  "params": {
    "type": "setProperty",
    "nodeId": "node_42",
    "property": "name",
    "value": "newFunctionName"
  }
}
```

**Example Request (insertNode)**:
```json
{
  "jsonrpc": "2.0",
  "id": 11,
  "method": "applyMutation",
  "params": {
    "type": "insertNode",
    "parentId": "node_3",
    "role": "statements",
    "node": {
      "type": "ReturnStatement",
      "value": {
        "type": "Literal",
        "value": 42
      }
    }
  }
}
```

**Example Request (updateNode)**:
```json
{
  "jsonrpc": "2.0",
  "id": 12,
  "method": "applyMutation",
  "params": {
    "type": "updateNode",
    "nodeId": "node_42",
    "properties": {
      "async": true,
      "returnType": "Promise"
    }
  }
}
```

**Example Request (deleteNode)**:
```json
{
  "jsonrpc": "2.0",
  "id": 13,
  "method": "applyMutation",
  "params": {
    "type": "deleteNode",
    "nodeId": "node_42"
  }
}
```

**Available to**: Refactor, Generator
**Errors**:
- `-32031`: Role not permitted
- `-32010`: Mutation failed
- `-32011`: Library policy violation

---

### applyBatch

Apply multiple mutations atomically. All mutations succeed or all are rolled back.

**Params**:
- `mutations` (array, required): Array of mutation objects (same format as applyMutation params)

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 14,
  "result": {
    "success": true,
    "appliedCount": 3
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 14,
  "method": "applyBatch",
  "params": {
    "mutations": [
      {
        "type": "setProperty",
        "nodeId": "node_42",
        "property": "name",
        "value": "refactoredFunction"
      },
      {
        "type": "insertNode",
        "parentId": "node_3",
        "role": "statements",
        "node": {
          "type": "VariableDeclaration",
          "name": "result",
          "initializer": {
            "type": "Literal",
            "value": 0
          }
        }
      },
      {
        "type": "deleteNode",
        "nodeId": "node_99"
      }
    ]
  }
}
```

**Available to**: Refactor, Generator
**Errors**:
- `-32031`: Role not permitted
- `-32010`: Batch mutation failed (no changes applied)

---

## Annotations

### getAnnotationSuggestions

Get ML-powered annotation suggestions for a node or position.

**Params**:
- `nodeId` (string, optional): Target node ID
- `line` (integer, optional): Line number
- `col` (integer, optional): Column number

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 15,
  "result": {
    "scopeId": "node_42",
    "suggestions": [
      {
        "nodeId": "node_42",
        "annotationType": "performance",
        "strategy": "cache",
        "reason": "Repeated computation detected in loop",
        "confidence": 0.85
      },
      {
        "nodeId": "node_42",
        "annotationType": "security",
        "strategy": "sanitize",
        "reason": "User input not validated",
        "confidence": 0.72
      }
    ],
    "diagnostics": [
      {
        "severity": "warning",
        "message": "Potential performance issue",
        "nodeId": "node_42"
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 15,
  "method": "getAnnotationSuggestions",
  "params": {
    "nodeId": "node_42"
  }
}
```

**Available to**: All roles

---

### applyAnnotationSuggestion

Apply an annotation suggestion to the AST.

**Params**:
- `nodeId` (string, required): Target node ID
- `annotationType` (string, required): Annotation type (e.g., "performance", "security")
- `strategy` (string, required): Strategy name (e.g., "cache", "sanitize")
- `reason` (string, required): Human-readable reason
- `confidence` (number, required): Confidence score (0.0-1.0)
- `accepted` (boolean, optional): Whether user accepted suggestion (default: true)

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 16,
  "result": {
    "success": true,
    "warning": null
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 16,
  "method": "applyAnnotationSuggestion",
  "params": {
    "nodeId": "node_42",
    "annotationType": "performance",
    "strategy": "cache",
    "reason": "Repeated computation detected in loop",
    "confidence": 0.85,
    "accepted": true
  }
}
```

**Available to**: Refactor, Generator
**Errors**:
- `-32031`: Role not permitted
- `-32010`: Application failed

---

### recordAnnotationFeedback

Record user feedback on an annotation suggestion (for ML training).

**Params**:
- `nodeId` (string, required): Target node ID
- `annotationType` (string, required): Annotation type
- `strategy` (string, required): Strategy name
- `reason` (string, required): Reason
- `confidence` (number, required): Confidence score
- `accepted` (boolean, required): Whether user accepted or rejected

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 17,
  "result": true
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 17,
  "method": "recordAnnotationFeedback",
  "params": {
    "nodeId": "node_42",
    "annotationType": "performance",
    "strategy": "cache",
    "reason": "Repeated computation detected",
    "confidence": 0.85,
    "accepted": false
  }
}
```

**Available to**: All roles

---

## Code Generation

### generateCode

Generate code from a natural language specification.

**Params**:
- `spec` (string, required): Natural language code specification
- `preferImports` (boolean, optional): Prefer library imports over inline implementations

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 18,
  "result": {
    "node": {
      "type": "FunctionDeclaration",
      "id": "generated_1",
      "name": "calculateDiscount",
      "parameters": [
        {
          "type": "Parameter",
          "name": "price",
          "paramType": "number"
        }
      ],
      "body": {
        "type": "Block",
        "statements": []
      }
    },
    "note": "Generated function with single parameter",
    "usedSymbols": ["price", "discount"],
    "language": "whetstone"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 18,
  "method": "generateCode",
  "params": {
    "spec": "Create a function that calculates a 10% discount on a given price",
    "preferImports": true
  }
}
```

**Available to**: Refactor, Generator
**Errors**:
- `-32031`: Role not permitted
- `-32020`: Code generation failed

---

## Pipeline

### runPipeline

Execute the full compilation pipeline: parse → validate → optimize → generate.

**Params**:
- `source` (string, required): Source code to process
- `sourceLanguage` (string, required): Source language (e.g., "whetstone", "javascript")
- `targetLanguage` (string, required): Target language for code generation

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 19,
  "result": {
    "success": true,
    "generatedCode": "function calculateTotal() {\n  return 42;\n}",
    "ast": {
      "type": "Program",
      "children": []
    },
    "parseDiagnostics": [],
    "validationDiagnostics": [
      {
        "severity": "warning",
        "message": "Unused import",
        "line": 1,
        "column": 0
      }
    ],
    "violations": [],
    "suggestions": [
      {
        "nodeId": "node_5",
        "annotationType": "style",
        "strategy": "simplify",
        "reason": "Expression can be simplified",
        "confidence": 0.90
      }
    ],
    "foldCount": 3,
    "dceCount": 2
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 19,
  "method": "runPipeline",
  "params": {
    "source": "function calculateTotal() { return 42; }",
    "sourceLanguage": "javascript",
    "targetLanguage": "whetstone"
  }
}
```

**Available to**: All roles
**Errors**:
- `-32020`: Pipeline execution failed

---

### parseSource

Parse source code into an AST.

**Params**:
- `source` (string, required): Source code to parse
- `language` (string, required): Source language

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 20,
  "result": {
    "ast": {
      "type": "Program",
      "id": "parsed_1",
      "children": [
        {
          "type": "FunctionDeclaration",
          "id": "parsed_2",
          "name": "calculateTotal",
          "parameters": [],
          "body": {
            "type": "Block",
            "id": "parsed_3",
            "statements": []
          }
        }
      ]
    },
    "diagnostics": [
      {
        "line": 5,
        "column": 12,
        "message": "Unexpected token",
        "severity": "error"
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 20,
  "method": "parseSource",
  "params": {
    "source": "function calculateTotal() { return 42; }",
    "language": "javascript"
  }
}
```

**Available to**: All roles

---

### generateFromAST

Generate code from the current AST in the active structured buffer.

**Params**:
- `language` (string, optional): Target language (defaults to buffer's language)

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 21,
  "result": {
    "code": "function calculateTotal() {\n  return 42;\n}",
    "language": "javascript"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 21,
  "method": "generateFromAST",
  "params": {
    "language": "javascript"
  }
}
```

**Available to**: All roles
**Errors**:
- `-32000`: No structured buffer active
- `-32020`: Code generation failed

---

### projectLanguage

Project the current AST to a different target language.

**Params**:
- `targetLanguage` (string, required): Target language for projection

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 22,
  "result": {
    "ast": {
      "type": "Program",
      "children": []
    },
    "generatedCode": "def calculate_total():\n    return 42",
    "sourceLanguage": "javascript",
    "targetLanguage": "python"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 22,
  "method": "projectLanguage",
  "params": {
    "targetLanguage": "python"
  }
}
```

**Available to**: All roles
**Errors**:
- `-32000`: No structured buffer active
- `-32020`: Language projection failed

---

## Workflow Recording

### startWorkflowRecording

Start recording all agent operations into a replayable workflow.

**Params**:
- `name` (string, optional): Workflow name/description

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 23,
  "result": {
    "recording": true,
    "name": "Refactoring workflow"
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 23,
  "method": "startWorkflowRecording",
  "params": {
    "name": "Refactoring workflow"
  }
}
```

---

### stopWorkflowRecording

Stop recording and return the complete workflow JSON.

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 24,
  "result": {
    "name": "Refactoring workflow",
    "steps": [
      {
        "method": "applyMutation",
        "params": {
          "type": "setProperty",
          "nodeId": "node_42",
          "property": "name",
          "value": "refactoredName"
        },
        "timestamp": 1707580800000
      },
      {
        "method": "applyAnnotationSuggestion",
        "params": {
          "nodeId": "node_42",
          "annotationType": "performance",
          "strategy": "cache",
          "reason": "Optimization applied",
          "confidence": 0.85
        },
        "timestamp": 1707580801000
      }
    ],
    "recording": false
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 24,
  "method": "stopWorkflowRecording"
}
```

---

### getWorkflowRecording

Get the current state of the workflow recording (without stopping).

**Params**: None

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 25,
  "result": {
    "recording": true,
    "name": "Refactoring workflow",
    "steps": [
      {
        "method": "applyMutation",
        "params": {},
        "timestamp": 1707580800000
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 25,
  "method": "getWorkflowRecording"
}
```

---

### replayWorkflow

Replay a previously recorded workflow.

**Params**:
- `workflow` (object, required): Workflow object (from stopWorkflowRecording)

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 26,
  "result": {
    "count": 2,
    "responses": [
      {
        "success": true,
        "warning": null
      },
      {
        "success": true,
        "warning": null
      }
    ]
  }
}
```

**Example Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 26,
  "method": "replayWorkflow",
  "params": {
    "workflow": {
      "name": "Refactoring workflow",
      "steps": [
        {
          "method": "applyMutation",
          "params": {
            "type": "setProperty",
            "nodeId": "node_42",
            "property": "name",
            "value": "refactoredName"
          }
        }
      ]
    }
  }
}
```

---

## Error Codes

The API uses standard JSON-RPC 2.0 error codes plus custom application codes:

| Code | Name | Description |
|------|------|-------------|
| `-32700` | Parse error | Invalid JSON |
| `-32600` | Invalid Request | Invalid JSON-RPC request |
| `-32601` | Method not found | Method does not exist |
| `-32602` | Invalid params | Invalid method parameters |
| `-32603` | Internal error | Internal JSON-RPC error |
| `-32000` | No structured buffer | No active structured buffer |
| `-32001` | AST unavailable | AST not available for current buffer |
| `-32002` | Node not found | Specified AST node not found |
| `-32010` | Mutation failed | AST mutation operation failed |
| `-32011` | Library policy violation | Operation violates library policy |
| `-32020` | Code generation failed | Code generation or projection failed |
| `-32031` | Role not permitted | Current agent role lacks permission |

**Example Error Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 99,
  "error": {
    "code": -32031,
    "message": "Role not permitted: mutation operations require 'refactor' or 'generator' role"
  }
}
```

---

## Connection Details

### WebSocket Endpoint

Default: `ws://localhost:8080` (configurable in Whetstone settings)

### Authentication

Currently none. Future versions may support token-based authentication.

### Library Context

When an agent connects and a library provider is configured, Whetstone automatically sends library context information including:
- Available library functions and types
- Security policies (allowed/blocked operations)
- Semantic tags for library search

### Message Format

All messages must conform to JSON-RPC 2.0:

**Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "methodName",
  "params": {}
}
```

**Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {}
}
```

**Error**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32000,
    "message": "Error description"
  }
}
```

---

## Best Practices

1. **Set Role Early**: Call `setAgentRole` immediately after connection to establish permissions.

2. **Handle Errors Gracefully**: Always check for error responses and handle appropriately.

3. **Use Batch Mutations**: For multiple related changes, use `applyBatch` to ensure atomicity.

4. **Cache AST**: Call `getAST` once and cache the result rather than repeatedly fetching.

5. **Validate Nodes**: Use `getInScopeSymbols` and `getDependencyGraph` to understand context before mutations.

6. **Record Workflows**: Use workflow recording for complex multi-step operations that may need to be replayed.

7. **Provide Feedback**: Call `recordAnnotationFeedback` to help improve ML suggestions.

8. **Check Library Policies**: Be aware that some operations may be blocked by library security policies.

---

## Example: Complete Refactoring Session

```javascript
// Connect and set up
const ws = new WebSocket('ws://localhost:8080');

// 1. Set role
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 1,
  method: "setAgentRole",
  params: { role: "refactor" }
}));

// 2. Start recording
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 2,
  method: "startWorkflowRecording",
  params: { name: "Optimize performance" }
}));

// 3. Get AST
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 3,
  method: "getAST"
}));

// 4. Get suggestions for specific function
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 4,
  method: "getAnnotationSuggestions",
  params: { nodeId: "node_calculateTotal" }
}));

// 5. Apply batch mutations
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 5,
  method: "applyBatch",
  params: {
    mutations: [
      {
        type: "setProperty",
        nodeId: "node_calculateTotal",
        property: "name",
        value: "calculateTotalOptimized"
      },
      {
        type: "insertNode",
        parentId: "node_functionBody",
        role: "statements",
        node: {
          type: "VariableDeclaration",
          name: "cache",
          initializer: { type: "ObjectLiteral", properties: [] }
        }
      }
    ]
  }
}));

// 6. Stop recording
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 6,
  method: "stopWorkflowRecording"
}));

// 7. Generate code
ws.send(JSON.stringify({
  jsonrpc: "2.0",
  id: 7,
  method: "generateFromAST",
  params: { language: "javascript" }
}));
```

---

## Sprint 7 Features

Sprint 7 introduced significant enhancements to the Agent API:

### New Context Queries
- `getInScopeSymbols`: Semantic scope analysis
- `getCallHierarchy`: Function call graph analysis
- `getDependencyGraph`: Declaration dependency tracking

### New Pipeline Methods
- `runPipeline`: Full compilation pipeline execution
- `parseSource`: Standalone parsing
- `generateFromAST`: Code generation from buffer
- `projectLanguage`: Cross-language projection

### Batch Mutations
- `applyBatch`: Atomic multi-mutation operations with automatic rollback

### Enhanced Diagnostics
All query methods now return richer diagnostic information including severity levels, line/column information, and suggested fixes.

---

## Version History

- **Sprint 7**: Context queries, pipeline methods, batch mutations
- **Sprint 6**: Security diagnostics, library integration, semantic search
- **Sprint 5**: Annotation suggestions, ML feedback loop
- **Sprint 4**: Workflow recording and replay
- **Sprint 3**: Code generation, AST mutations
- **Sprint 2**: Role management, session tracking
- **Sprint 1**: Initial API with basic AST access

---

For implementation details and source code, see:
- Agent server: `editor/src/core/AgentServer.cpp`
- Protocol handlers: `editor/src/core/AgentProtocol.h`
- AST operations: `compiler/src/ast/`

For usage examples and integration guides, see the `/docs` directory in the Whetstone repository.
