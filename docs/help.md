# Documentation

Welcome to Whetstone. This panel provides a compact reference to the editor,
annotations, agent APIs, and language support.

## Getting Started
- Open a folder from File > Open or Project > Open.
- Use the left File Tree to browse files.
- Toggle Text/Structured mode with the Mode switch in the toolbar.
- Run or build from the Build tab (or toolbar buttons).

## Annotation Reference
Whetstone uses canonical memory annotations:
- `@Reclaim(Tracing|Escape|Cycle)`
- `@Owner(Single|Shared_ARC)`
- `@Lifetime(RAII)`
- `@Deallocate(Explicit)`
- `@Allocate(Static|Register|Allocator)`

Annotations can be added from the gutter or context menu. Conflicts are
highlighted in the Problems panel with quick fixes.

## Keyboard Shortcuts
Shortcuts are configurable. See Settings > Keybindings. Default profile:
- `Ctrl+P` Command palette
- `Ctrl+F` Find/Replace
- `Ctrl+S` Save
- `Ctrl+Z` Undo, `Ctrl+Y` Redo

## Language Support
Supported languages (parsing + generation):
- Python, C++, Elisp, JavaScript, TypeScript, Java, Rust, Go

## Agent API (JSON-RPC)
Core RPC methods:
- `getAST`
- `applyMutation`
- `generateCode`
- `getAnnotationSuggestions`
- `applyAnnotationSuggestion`
- `startWorkflowRecording`, `stopWorkflowRecording`, `replayWorkflow`

## Troubleshooting
- If an AST is unavailable, switch to Structured mode.
- If LSP is offline, completions still work from indexed stubs.
- If Emacs integration fails, check the Output panel for Emacs logs.
