# Core AST Structure - Testing & Validation Guide

**Status**: Phase 1 (Structures) & Phase 2 (Editors) Complete
**Created**: 2026-02-03
**Feature**: [spec.md](spec.md)

## Overview

This guide provides step-by-step instructions for validating that the Core AST Structure implementation is working correctly in MPS. All structure concepts and editor definitions have been added to the SemAnno language files.

## Files Modified

### Phase 1: Structure Definitions
- **File**: `languages/SemAnno/models/SemAnno.structure.mps`
- **Changes**: Added ~30+ concept definitions including:
  - Core AST Nodes: Module, Function, Parameter, Variable
  - Statements: Block, Assignment, IfStatement, WhileLoop, ForLoop, Return, ExpressionStatement
  - Expressions: BinaryOperation, UnaryOperation, FunctionCall, Literals, Collections, Access
  - Types: PrimitiveType, ListType, SetType, MapType, TupleType, ArrayType, OptionalType, CustomType
  - Annotations: DerefStrategy, OptimizationLock, LangSpecific

### Phase 2: Editor Definitions
- **File**: `languages/SemAnno/models/SemAnno.editor.mps`
- **Changes**: Added editor declarations for all Phase 1 concepts with:
  - Proper syntax highlighting (keywords in blue)
  - Nested structure indentation
  - Inline property editing
  - Reference cell handling

## Validation Checklist

### Step 1: Rebuild the SemAnno Language

1. Open MPS 2024.3 (or compatible version)
2. Open the Whetstone_DSL project
3. Right-click on the `SemAnno` language module
4. Select **Rebuild Language**
5. **Expected Result**: Build completes with **zero errors and zero warnings**

**If you see errors**:
- Check the MPS Error pane for red highlights
- Common issues:
  - Missing concept IDs (should not happen if structure file is valid)
  - Invalid parent concept references (check structure.mps file)
  - Editor referencing non-existent concepts (check editor.mps file)

### Step 2: Create a Test Model

1. In MPS, create a new Model: Right-click project > New > Model
2. Name it: `SemAnno.tests.SimpleExample`
3. Choose **Language**: `SemAnno`
4. Create the model file

### Step 3: Create a Module Root

1. In the new model, click "Add Root" or right-click the model node
2. Select **Module** from the concept list
3. **Expected Result**:
   - Editor shows: `module [editable field]`
   - Type a name like "example"

### Step 4: Add a Function

1. Within the Module, add a Function element
2. Set name: `sum`
3. Add parameters by clicking the "+" next to parameters:
   - Parameter 1: name="items", type="list[int]"
   - Parameter 2: name="factor", type="float"
4. Set return type to "float"
5. **Expected Result**:
   - Editor shows: `def sum(items: list[int], factor: float) -> float:`
   - Function body is ready for statements

### Step 5: Add Statements to Function Body

1. Click in the function body area
2. Add a ForLoop:
   - Iterator name: "item"
   - Iterable: reference to "items"
   - Body: add an assignment or expression
3. Add an Assignment statement:
   - Target: "result"
   - Value: `item + accumulator`
4. Add a Return statement:
   - Value: multiply result by factor
5. **Expected Result**:
   - Statements display with proper indentation
   - Keywords (for, in, return) are highlighted in blue
   - Nested structures are readable

### Step 6: Add a Variable at Module Level

1. In the Module, add a Variable:
   - Name: "cache"
   - Type: "optional[map[string, list[int]]]"
2. **Expected Result**:
   - Complex nested type renders as: `optional[map[string, list[int]]]`
   - Type hierarchy is readable and selectable

### Step 7: Add Expressions

1. Within an assignment or expression context, add:
   - BinaryOperation: `items + factor` (or similar)
   - FunctionCall: `print("message")`
   - VariableReference to existing variables
   - Literals: integers, floats, strings, booleans
2. **Expected Result**:
   - Expressions render with proper operator spacing
   - Function calls show: `functionName(arg1, arg2)`
   - Literals display with syntax highlighting (strings in green, numbers in magenta)

### Step 8: Test Annotations (Future Expansion)

1. Try to add a DerefStrategy annotation to the function:
   - Note: The structure supports this, but UI for adding annotations is part of future features
2. **Expected Result**:
   - Concept exists and can be added
   - Property and reference fields work

## Success Criteria

The implementation is complete and working when:

1. ✅ **SemAnno language rebuilds** with zero errors, zero warnings
2. ✅ **All concepts are available** when creating model roots and adding children
3. ✅ **Editors render correctly**:
   - Keywords appear in blue
   - Indentation works for nested structures
   - Properties are editable inline
   - References are selectable
4. ✅ **Type system works**: Nested types (list, optional, map, etc.) render correctly
5. ✅ **Model persistence**: Models save and reload without errors
6. ✅ **No red error marks** appear in the model tree for correctly created structures

## Troubleshooting

### Problem: "Concept not found" error

**Cause**: Structure definitions may not have reloaded after rebuild.

**Solution**:
1. In MPS, go to File > Invalidate Caches and Restart
2. Reopen the project
3. Rebuild the language again

### Problem: Editor shows placeholder text instead of structured content

**Cause**: Editor definitions may have incorrect role references.

**Solution**:
1. Check SemAnno.editor.mps file for correct role names
2. Role names must match LinkDeclaration "role" properties in structure.mps
3. Rebuild language after fixing

### Problem: Model has red error squiggles

**Cause**: Invalid structure creation (wrong concept in wrong context).

**Solution**:
1. Delete the invalid node
2. Recreate it, ensuring you select the correct concept
3. Verify type compatibility

### Problem: Reference fields won't accept concepts

**Cause**: Concept may not be the target of that reference.

**Solution**:
1. Check structure.mps to verify reference type compatibility
2. Only accept valid target concept types
3. If needed, create intermediate wrapper concepts

## Next Steps

After validation is complete:

1. **Phase 3**: Create constraint rules (SemAnno.constraints.mps)
2. **Phase 4**: Add type system and typesystem rules
3. **Downstream Features**: Python/C++ projections will build on this AST

## Reference: Concept Hierarchy

```
Module (root)
├── functions: Function*
├── variables: Variable*
└── annotations: Annotation*

Function
├── name: string
├── parameters: Parameter*
├── returnType: Type
├── body: Statement*
└── annotations: Annotation*

Statement (abstract)
├── Block
├── Assignment
├── IfStatement
├── WhileLoop
├── ForLoop
├── Return
└── ExpressionStatement

Expression (abstract)
├── BinaryOperation
├── UnaryOperation
├── FunctionCall
├── VariableReference
├── Literals: Integer, Float, String, Boolean, Null
├── ListLiteral
├── IndexAccess
└── MemberAccess

Type (abstract)
├── PrimitiveType (int, float, string, bool)
├── ListType
├── SetType
├── MapType
├── TupleType
├── ArrayType
├── OptionalType
└── CustomType

Annotation (abstract)
├── DerefStrategy
├── OptimizationLock
└── LangSpecific
```

## Documentation References

- Feature Specification: [spec.md](spec.md)
- Implementation Plan: [plan.md](plan.md)
- Task Definitions: [tasks.md](tasks.md)
- MPS Language Documentation: https://www.jetbrains.com/help/mps/

---

**Last Updated**: 2026-02-03
**Author**: Claude Haiku 4.5
