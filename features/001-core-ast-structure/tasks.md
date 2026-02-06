# Implementation Tasks: Core AST Structure (Feature 001)

**Feature**: Core AST Structure for SemAnno
**Phase**: Planning Complete → Ready for Phase 1 Execution
**Total Tasks**: 26
**Estimated Duration**: 42-53 hours

---

## Legend

- **[PHASE]**: 0 (Research), 1 (Structure), 2 (Editors), 3 (Testing)
- **[PRIORITY]**: P0 (blocker), P1 (must-have), P2 (should-have), P3 (nice-to-have)
- **[DURATION]**: Estimated hours
- **[DEPENDS]**: Task IDs that must complete first

---

## Phase 0: Research & Exploration

### RESEARCH-1: MPS Editor Design Patterns for Projections
**Priority**: P1 | **Duration**: 2h | **Depends**: None
**Status**: Pending

**Objective**: Research how MPS languages support multiple projections (e.g., Python and C++ views on same AST).

**Tasks**:
- Read MPS Language Design documentation on projection cells
- Find 2-3 examples in jetbrains.mps.lang.* languages (e.g., editor, structure)
- Document patterns: how are editors structured to allow runtime projection switching?
- Create summary: "Editor Design Patterns" with code examples

**Definition of Done**:
- Summary document created
- At least 2 patterns identified and documented
- Phase 1 editor design can proceed using these patterns

---

### RESEARCH-2: Nested Type Composition in MPS
**Priority**: P1 | **Duration**: 2h | **Depends**: None
**Status**: Pending

**Objective**: Verify that deeply nested type references don't cause circular dependencies or serialization issues.

**Tasks**:
- Research MPS reference composition patterns
- Test if Type → Type references can create lists[map[string, int]] without circular deps
- Document: How to prevent cycles, how to serialize nested structures
- Prepare example: 3-4 levels of nesting (list[map[string, optional[int]]])

**Definition of Done**:
- Patterns documented
- Circular reference prevention strategy clear
- Example type hierarchy designed

---

### RESEARCH-3: Annotation Attachment Patterns
**Priority**: P1 | **Duration**: 2h | **Depends**: None
**Status**: Pending

**Objective**: Understand best practices for attaching multiple independent annotations to a single node.

**Tasks**:
- Research multi-valued references in MPS
- Study how jetbrains.mps.lang.core.structure handles attributes/annotations
- Verify: Can a Function have [DerefStrategy, OptimizationLock, LangSpecific] simultaneously?
- Document ordering, optional/required logic, removal behavior

**Definition of Done**:
- Annotation design pattern documented
- Multi-valued reference behavior verified
- Phase 1 structure design can proceed

---

## Phase 1: Core Structure Definition

### TASK-1-1: Define Root & Container Concepts
**Priority**: P0 | **Duration**: 2h | **Depends**: None
**Phase**: 1
**Status**: Pending

**Objective**: Define Module concept and establish container pattern.

**Subtasks**:
1. Create Module concept in SemAnno.structure.mps
   - name: string property
   - annotations: Annotation* reference (multi-valued)
   - variables: Variable* reference (multi-valued)
   - functions: Function* reference (multi-valued)
2. Verify MPS recognizes all properties and references
3. Mark concept as "root" in MPS (if applicable)

**Definition of Done**:
- Module concept defined and saved
- MPS structure editor shows concept without errors
- All 4 features (name property, 3 references) visible in model tree

**Review Criteria**:
- [ ] Concept name, property names match spec
- [ ] References are multi-valued where appropriate
- [ ] No red errors in MPS

---

### TASK-1-2: Define Function & Parameter Concepts
**Priority**: P0 | **Duration**: 3h | **Depends**: TASK-1-1
**Phase**: 1
**Status**: Pending

**Objective**: Define Function and Parameter concepts with proper signature structure.

**Subtasks**:
1. Create Function concept
   - name: string property
   - parameters: Parameter* reference (multi-valued, ordered)
   - returnType: Type reference (single)
   - body: Statement* reference (multi-valued, ordered)
   - annotations: Annotation* reference (multi-valued)
2. Create Parameter concept
   - name: string property
   - type: Type reference (single)
   - defaultValue: Expression? reference (optional)
3. Verify Function.parameters and Parameter.type are composable
4. Test: Create Function, add 2 Parameters with types, set returnType

**Definition of Done**:
- Function and Parameter concepts defined
- Parameter ordering preserved
- Type references are composable (Type → Type reference works)
- Can create function with 2 parameters in test model without errors

---

### TASK-1-3: Define Variable Concept
**Priority**: P0 | **Duration**: 1h | **Depends**: TASK-1-1
**Phase**: 1
**Status**: Pending

**Objective**: Define Variable concept for module and local variables.

**Subtasks**:
1. Create Variable concept
   - name: string property
   - type: Type reference (single)
   - initializer: Expression? reference (optional)
   - annotations: Annotation* reference (multi-valued)
2. Ensure Variable can be added to Module.variables
3. Test: Create Module → add Variable with type and initializer

**Definition of Done**:
- Variable concept defined
- Can be added to Module and function scopes
- No errors in test model

---

### TASK-1-4: Define All Statement Concepts
**Priority**: P0 | **Duration**: 5h | **Depends**: TASK-1-2
**Phase**: 1
**Status**: Pending

**Objective**: Define abstract Statement parent and 7 concrete statement types.

**Subtasks**:
1. Create abstract Statement concept (parent)
2. Create concrete statement concepts:
   - **Block**: statements: Statement* (multi-valued)
   - **Assignment**: target: Expression, value: Expression
   - **IfStatement**: condition: Expression, thenBranch: Statement*, elseBranch: Statement? (optional)
   - **WhileLoop**: condition: Expression, body: Statement*
   - **ForLoop**: iteratorName: string property, iterable: Expression, body: Statement*
   - **Return**: value: Expression? (optional)
   - **ExpressionStatement**: expression: Expression
3. Verify inheritance hierarchy (all inherit from Statement)
4. Test: Create function → add each statement type to body
5. Test nesting: Block → Assignment → BinaryOperation expression
6. Verify statement ordering is preserved

**Definition of Done**:
- All 7 concrete statement types defined
- All inherit from Statement parent
- Can create function with mixed statement types
- Nesting works (statements inside blocks, expressions inside statements)
- No errors in test model

**Review Criteria**:
- [ ] All statement types from spec are defined
- [ ] Abstract Statement parent established
- [ ] Multi-valued statement references are ordered
- [ ] Optional references (elseBranch, return value) work correctly

---

### TASK-1-5: Define All Expression Concepts
**Priority**: P0 | **Duration**: 5h | **Depends**: TASK-1-4
**Phase**: 1
**Status**: Pending

**Objective**: Define abstract Expression parent and 13 concrete expression types.

**Subtasks**:
1. Create abstract Expression concept (parent)
2. Create concrete expression concepts:
   - **BinaryOperation**: operator: string, left: Expression, right: Expression
   - **UnaryOperation**: operator: string, operand: Expression
   - **FunctionCall**: functionName: string, arguments: Expression* (multi-valued)
   - **VariableReference**: variableName: string
   - **IntegerLiteral**: value: integer
   - **FloatLiteral**: value: float
   - **StringLiteral**: value: string
   - **BooleanLiteral**: value: boolean
   - **NullLiteral**: (no properties)
   - **ListLiteral**: elements: Expression* (multi-valued)
   - **IndexAccess**: target: Expression, index: Expression
   - **MemberAccess**: target: Expression, memberName: string
3. Verify inheritance hierarchy
4. Test: Create Assignment with BinaryOperation on right side
5. Test nesting: FunctionCall → BinaryOperation arguments
6. Test all literal types can be created

**Definition of Done**:
- All 13 concrete expression types defined
- All inherit from Expression parent
- Can create complex nested expressions
- Literals can be created and have correct value types
- No errors in test model

**Review Criteria**:
- [ ] All expression types from spec defined
- [ ] Abstract Expression parent established
- [ ] Expression nesting works (expressions contain expressions)
- [ ] Operator strings work as properties
- [ ] Argument lists for FunctionCall are ordered

---

### TASK-1-6: Define All Type Concepts
**Priority**: P0 | **Duration**: 4h | **Depends**: TASK-1-3
**Phase**: 1
**Status**: Pending

**Objective**: Define abstract Type parent and 8 concrete type types with composition support.

**Subtasks**:
1. Create abstract Type concept (parent)
2. Create concrete type concepts:
   - **PrimitiveType**: kind: enum {int, float, string, bool}
   - **ListType**: elementType: Type (reference)
   - **SetType**: elementType: Type (reference)
   - **MapType**: keyType: Type, valueType: Type
   - **TupleType**: elementTypes: Type* (multi-valued, ordered)
   - **ArrayType**: elementType: Type, size: Expression
   - **OptionalType**: innerType: Type
   - **CustomType**: typeName: string
3. Verify Type → Type references are composable
4. Test: Create type "list[map[string, int]]" by composition
5. Test: Create type "optional[tuple[int, string, bool]]"
6. Verify no circular reference issues
7. Test: Create Variable with complex type, render in editor

**Definition of Done**:
- All 8 concrete type types defined
- Type references are fully composable (ListType.elementType → MapType.keyType → PrimitiveType)
- Can create complex nested types without errors
- PrimitiveType enum values work correctly
- No circular reference issues detected

**Review Criteria**:
- [ ] All type types from spec defined
- [ ] Type composition works (type → type → type nesting)
- [ ] Enum values for PrimitiveType are: int, float, string, bool
- [ ] ArrayType.size accepts Expression references
- [ ] TupleType elementTypes preserve order

---

### TASK-1-7: Define All Annotation Concepts
**Priority**: P1 | **Duration**: 3h | **Depends**: TASK-1-2, TASK-1-5
**Phase**: 1
**Status**: Pending

**Objective**: Define abstract Annotation parent and 3 concrete annotation types.

**Subtasks**:
1. Create abstract Annotation concept (parent, optional for extensibility)
2. Create concrete annotation concepts:
   - **DerefStrategy**:
     - strategy: enum {imperative, streamed, batched, content_addressed}
     - derefTime: Expression? (optional)
     - derefLocation: string? (optional)
     - owner: string? (optional)
   - **OptimizationLock**:
     - lockedBy: string
     - lockReason: string
     - lockLevel: enum {warning, soft, hard}
     - affectedStrategies: string* (multi-valued)
     - timestamp: string
   - **LangSpecific**:
     - language: enum {python, cpp, rust}
     - idiomType: string
     - rawSyntax: string
     - semanticHint: string? (optional)
     - position: enum {before, after, wrapping}
3. Verify annotations can be attached to Function.annotations
4. Test: Create Function with 1 DerefStrategy annotation
5. Test: Create Variable with 1 LangSpecific annotation
6. Test: Attach multiple annotations to same node

**Definition of Done**:
- All 3 concrete annotation types defined
- Optional properties work (derefTime, owner, semanticHint)
- Enum values are correct
- Can attach annotations to functions and variables
- Multi-valued affectedStrategies list works
- No errors in test model

---

### TASK-1-8: Define Constraints & Validation Rules
**Priority**: P1 | **Duration**: 2h | **Depends**: TASK-1-4, TASK-1-5, TASK-1-6
**Phase**: 1
**Status**: Pending

**Objective**: Create SemAnno.constraints.mps with basic validation to prevent invalid structures.

**Subtasks**:
1. Create SemAnno.constraints.mps model file
2. Define constraints:
   - Statements can only appear in Statement* contexts (Block.statements, Function.body, etc.)
   - Expressions can only appear in Expression contexts
   - Types can only appear in Type contexts
   - Statements cannot be direct children of Expression
   - Expressions cannot be direct children of Statement (only in ExpressionStatement)
3. Add can-be-parent/can-be-child rules to prevent invalid structures
4. Test: Attempt to place a Statement inside an Expression context → should be blocked by constraint

**Definition of Done**:
- SemAnno.constraints.mps created
- Basic structural constraints defined
- MPS enforces constraint violations
- Invalid structures are rejected during creation

---

### TASK-1-9: Create Test Model with Complex AST
**Priority**: P0 | **Duration**: 3h | **Depends**: TASK-1-7
**Phase**: 1
**Status**: Pending

**Objective**: Create SimpleExample.mps demonstrating all concept types.

**Subtasks**:
1. Create new SemAnno model file: `languages/SemAnno/tests/SimpleExample.mps`
2. Create Module with name "example"
3. Add 1 Function "sum" with:
   - Parameters: items (list[int]), factor (float)
   - returnType: float
   - Body with 4 statements:
     - ForLoop: iterate items, assign accumulator
     - Assignment: result = accumulator * factor
     - IfStatement: if factor > 0 then return result, else return 0
     - Return: return result
4. Add 1 Variable: cache (optional[map[string, list[int]]])
5. Add annotations:
   - DerefStrategy on function (strategy: batched)
   - OptimizationLock on function (locked by "alice", reason "optimization")
   - LangSpecific on variable (language: python, idiomType: "type_hint")
6. Save and reload model in MPS
7. Verify no errors, no warnings

**Definition of Done**:
- SimpleExample.mps created with all concept types demonstrated
- Model includes: Module, Function, Parameters, Variable, all Statement types, nested Expressions, nested Types, Annotations
- Model loads and renders without errors
- Model serializes and deserializes correctly
- No red errors in MPS error list

**Acceptance Criteria**:
- [ ] Module created with name
- [ ] Function with 2 parameters, return type, body
- [ ] Body contains ForLoop, Assignment, IfStatement, Return
- [ ] Variable with complex nested type
- [ ] All annotations attached and visible

---

## Phase 1 Completion Checkpoint

**Validation Checklist**:
- [ ] All 30+ concepts defined in SemAnno.structure.mps
- [ ] SemAnno.constraints.mps created with validation rules
- [ ] SimpleExample.mps created and loads without errors
- [ ] MPS language rebuild: zero errors, zero warnings
- [ ] All concept names match specification
- [ ] All properties and references match specification

**Approval**: Ready to proceed to Phase 2 (Editors) only if all checkpoints pass.

---

## Phase 2: Editor Definition

### TASK-2-1: Define Module & Function Editors
**Priority**: P0 | **Duration**: 3h | **Depends**: TASK-1-2
**Phase**: 2
**Status**: Pending

**Objective**: Create editor definitions for Module, Function, Parameter concepts.

**Subtasks**:
1. Open SemAnno.editor.mps (create if needed)
2. Define Module editor:
   - Render: "module" (blue keyword), space, name (editable property), newline
   - List functions and variables vertically, indented
3. Define Function editor:
   - Render: "def" (blue keyword), space, name, "(", parameters comma-separated, ")", space, "->", space, return type, ":", newline
   - Body statements indented vertically
4. Define Parameter editor:
   - Render: name, ":", space, type reference
5. Test in MPS:
   - Create Module, add Function, add Parameters
   - Verify rendering matches expected layout
   - Verify indentation works

**Definition of Done**:
- Module, Function, Parameter editors defined
- Editors render with proper syntax highlighting (blue keywords)
- Indentation works for nested structures
- Parameters display comma-separated
- Return type reference is selectable

---

### TASK-2-2: Define All Statement Editors
**Priority**: P0 | **Duration**: 5h | **Depends**: TASK-2-1
**Phase**: 2
**Status**: Pending

**Objective**: Create editor definitions for all 7 statement types.

**Subtasks**:
1. Define Block editor:
   - Render statements vertically, indented
2. Define Assignment editor:
   - Render: target, space, "=", space, value
3. Define IfStatement editor:
   - Render: "if" (blue), space, condition, ":", newline
   - Then-branch indented
   - Optional "else:" (blue), newline, else-branch indented
4. Define WhileLoop editor:
   - Render: "while" (blue), space, condition, ":", newline, body indented
5. Define ForLoop editor:
   - Render: "for" (blue), space, iteratorName, space, "in" (blue), space, iterable, ":", newline, body indented
6. Define Return editor:
   - Render: "return" (blue), space, optional value expression
7. Define ExpressionStatement editor:
   - Render: expression
8. Test: Create function with mixed statements, verify layout

**Definition of Done**:
- All 7 statement editors defined
- Syntax highlighting applied (keywords in blue)
- Indentation rendered correctly for nested statements
- Optional branches (else, return value) display when present, hidden when absent

---

### TASK-2-3: Define All Expression Editors
**Priority**: P0 | **Duration**: 4h | **Depends**: TASK-2-2
**Phase**: 2
**Status**: Pending

**Objective**: Create editor definitions for all 13 expression types.

**Subtasks**:
1. Define BinaryOperation editor:
   - Render: left, space, operator, space, right
2. Define UnaryOperation editor:
   - Render: operator, operand
3. Define FunctionCall editor:
   - Render: functionName, "(", arguments comma-separated, ")"
4. Define VariableReference editor:
   - Render: variableName
5. Define Literal editors (5 types):
   - IntegerLiteral: value (number color - magenta)
   - FloatLiteral: value (number color - magenta)
   - StringLiteral: '"', value (string color - green), '"'
   - BooleanLiteral: value (keyword color - blue)
   - NullLiteral: "None" or "null" (keyword color - blue)
6. Define ListLiteral editor:
   - Render: "[", elements comma-separated, "]"
7. Define IndexAccess editor:
   - Render: target, "[", index, "]"
8. Define MemberAccess editor:
   - Render: target, ".", memberName
9. Test: Create complex nested expressions, verify rendering

**Definition of Done**:
- All 13 expression editors defined
- Syntax coloring applied (keywords blue, strings green, numbers magenta)
- Operators and operands display correctly
- Nested expressions render with proper precedence indication
- Literals render with appropriate colors

---

### TASK-2-4: Define All Type Editors
**Priority**: P0 | **Duration**: 3h | **Depends**: TASK-2-3
**Phase**: 2
**Status**: Pending

**Objective**: Create editor definitions for all 8 type types.

**Subtasks**:
1. Define PrimitiveType editor:
   - Render: kind enum value (int, float, string, bool)
2. Define ListType editor:
   - Render: "list" (blue), "[", elementType, "]"
3. Define SetType editor:
   - Render: "set" (blue), "[", elementType, "]"
4. Define MapType editor:
   - Render: "map" (blue), "[", keyType, ",", space, valueType, "]"
5. Define TupleType editor:
   - Render: "tuple" (blue), "[", elementTypes comma-separated, "]"
6. Define ArrayType editor:
   - Render: elementType, "[", size expression, "]"
7. Define OptionalType editor:
   - Render: innerType, "?"
8. Define CustomType editor:
   - Render: typeName
9. Test: Create complex nested types (list[map[string, optional[int]]]), verify rendering

**Definition of Done**:
- All 8 type editors defined
- Type keywords rendered in blue
- Nested types render naturally (list[map[...]])
- Type references are selectable
- Composition works (can select complex types)

---

### TASK-2-5: Define Annotation Editors
**Priority**: P1 | **Duration**: 2h | **Depends**: TASK-2-4
**Phase**: 2
**Status**: Pending

**Objective**: Create editor definitions for all 3 annotation types.

**Subtasks**:
1. Define DerefStrategy editor:
   - Render: "@deref" (gray), "(", strategy enum, optional properties as sub-lines, ")"
2. Define OptimizationLock editor:
   - Render: "@lock" (gray), "(", properties comma-separated, ")"
   - Include: lockedBy, lockReason, lockLevel, affectedStrategies, timestamp
3. Define LangSpecific editor:
   - Render: "@lang_specific" (gray), "(", language, ",", idiomType, ",", rawSyntax, optional properties, ")"
4. Test: Create function with each annotation type, verify rendering near function

**Definition of Done**:
- All 3 annotation editors defined
- Annotation keywords in gray/italic (distinguishes from code)
- Enum values display correctly
- Optional properties hidden when absent
- Annotations render attached to parent node

---

### TASK-2-6: Editor Testing & Refinement in MPS
**Priority**: P0 | **Duration**: 3h | **Depends**: TASK-2-5
**Phase**: 2
**Status**: Pending

**Objective**: Rebuild language in MPS, test editors interactively, fix issues.

**Subtasks**:
1. Rebuild SemAnno language in MPS
   - Check for cell model errors
   - Verify no "missing editor" errors
2. Open SimpleExample.mps and manually verify:
   - Module renders correctly (name, lists functions/variables)
   - Function renders with "def", name, parameters, return type, body
   - All statements render with proper indentation and syntax highlighting
   - All expressions render correctly (operators, operands, nesting)
   - All types render correctly (composition, nesting)
   - Annotations render near their nodes
3. Test interactive editing:
   - Create new Module, add Function by adding statements
   - Edit statement properties (if condition, loop iterator, etc.)
   - Edit expression operators and operands
   - Change types by selecting from dropdown
   - Navigate between references (e.g., click parameter type → jump to Variable definition)
4. Fix any rendering issues:
   - Indentation not working → adjust cell layout
   - Colors not applying → verify color codes
   - References not selectable → verify reference cell type
5. Document any workarounds or editor-specific behavior

**Definition of Done**:
- Language rebuilds with zero errors, zero warnings
- All editors render without errors
- Interactive editing works (properties editable, references selectable)
- All syntax highlighting and indentation work correctly
- SimpleExample.mps renders as designed

**Acceptance Criteria**:
- [ ] Language rebuild: zero errors
- [ ] All concepts have working editors
- [ ] Indentation works for nested structures
- [ ] Syntax highlighting works (blue keywords, green strings, magenta numbers)
- [ ] Properties can be edited inline
- [ ] References are selectable

---

## Phase 2 Completion Checkpoint

**Validation Checklist**:
- [ ] All concept editors defined in SemAnno.editor.mps
- [ ] MPS language rebuild: zero errors, zero warnings
- [ ] SimpleExample.mps renders without editor errors
- [ ] Manual AST creation works (can add statements, expressions, types)
- [ ] Syntax highlighting and indentation correct
- [ ] All references are selectable and navigable

**Approval**: Ready to proceed to Phase 3 (Testing) only if all checkpoints pass.

---

## Phase 3: Integration & Testing

### TASK-3-1: Create Extended Test Models
**Priority**: P1 | **Duration**: 3h | **Depends**: TASK-2-6
**Phase**: 3
**Status**: Pending

**Objective**: Create 3-5 test models demonstrating various AST structures.

**Subtasks**:
1. Create `SimpleFunction.mps`: Basic function (no loops, minimal types)
   - Function: greet(name: string) -> string
   - Body: Return StringLiteral
2. Create `ComplexFunction.mps`: Nested control flow
   - Function with ForLoop → IfStatement → BinaryOperation
   - Demonstrates statement nesting
3. Create `NestedTypes.mps`: Complex type structures
   - Variables with types: list[map[string, optional[int]]], tuple[int, string, bool]
   - Demonstrates type composition
4. Create `WithAnnotations.mps`: Full example with annotations
   - Function with DerefStrategy, OptimizationLock, LangSpecific
   - Variables with annotations
   - Demonstrates annotation attachment
5. Save all models and verify load without errors

**Definition of Done**:
- 5 test models created
- Each demonstrates different AST features
- All load and render without errors
- Each model documents its purpose in comments (if MPS supports)

---

### TASK-3-2: Test Model Serialization Round-Trips
**Priority**: P0 | **Duration**: 2h | **Depends**: TASK-3-1
**Phase**: 3
**Status**: Pending

**Objective**: Verify models persist correctly and don't corrupt on save/reload.

**Subtasks**:
1. For each test model:
   - Save model to disk (automatic via MPS)
   - Close MPS completely
   - Reopen MPS and reload project
   - Reload each test model
   - Verify no errors, warnings, or corrupted nodes
   - Verify content matches what was saved
2. Test model properties:
   - All string properties preserved correctly
   - All enum values preserved
   - All references intact (no broken links)
   - Statement and type ordering preserved
3. Document any serialization quirks or issues found

**Definition of Done**:
- All test models round-trip successfully (save → close → reopen → load)
- No corruption or data loss detected
- Content and structure verified after reload

---

### TASK-3-3: Quality & Correctness Verification
**Priority**: P0 | **Duration**: 2h | **Depends**: TASK-3-2
**Phase**: 3
**Status**: Pending

**Objective**: Run comprehensive checks to ensure language quality.

**Subtasks**:
1. Rebuild SemAnno language in MPS:
   - Check for zero errors, zero warnings
   - Verify all concept definitions are complete
   - Verify all editor definitions are complete
2. Run MPS consistency checks on all test models:
   - Check for dangling references
   - Check for orphaned nodes
   - Verify hierarchy is valid
3. Test constraint enforcement:
   - Attempt to create invalid structure (e.g., Statement as Expression)
   - Verify constraint blocks it
4. Document findings: Are there any edge cases or limitations?

**Definition of Done**:
- Language rebuild: zero errors, zero warnings
- All test models pass consistency checks
- Constraints work as designed
- No unresolved issues

---

### TASK-3-4: Write Quickstart Guide
**Priority**: P1 | **Duration**: 2h | **Depends**: TASK-3-3
**Phase**: 3
**Status**: Pending

**Objective**: Document how to create AST in SemAnno for future users.

**Deliverable**: `languages/SemAnno/QUICKSTART.md`

**Contents**:
1. Overview: What is SemAnno, what can you do with it
2. Step-by-step guide: Create first Module
   - Create new model file
   - Add Module concept
   - Set module name
3. Create first Function
   - Add Function to Module.functions
   - Set function name, return type
   - Add parameters
4. Create statements and expressions
   - Add statements to function body
   - Show examples: ForLoop, Assignment, Return
   - Show expression examples: BinaryOperation, FunctionCall
5. Work with types
   - Create primitive type
   - Create nested type (list[map[...]])
   - Show how types compose
6. Add annotations
   - Attach DerefStrategy to function
   - Attach OptimizationLock to variable
7. Tips & tricks
   - How to navigate references
   - How to edit properties
   - Keyboard shortcuts (if any)
8. Examples section
   - Link to test models (SimpleFunction.mps, ComplexFunction.mps, etc.)
   - Show rendered output for each

**Definition of Done**:
- QUICKSTART.md created
- All major workflows documented
- Step-by-step instructions clear
- Examples provided with screenshots or ASCII renderings
- No unexplained concepts

---

### TASK-3-5: Prepare for Phase 2 Features (Python/C++ Projections)
**Priority**: P2 | **Duration**: 1h | **Depends**: TASK-3-4
**Phase**: 3
**Status**: Pending

**Objective**: Document how Phase 2 features will extend core AST.

**Deliverable**: `languages/SemAnno/NEXT_PHASES.md`

**Contents**:
1. Overview: What comes next (Python Projection, C++ Projection, Tree-sitter Import)
2. Python Projection (Week 3-4):
   - Will extend editors to show Python-syntax rendering
   - Will add Python text generator
   - Will not change core AST structure
   - Dependencies: This feature (core AST structure)
3. C++ Projection (Week 5-6):
   - Will extend editors to show C++-syntax rendering
   - Will add C++ text generator
   - Will add deref strategy translation
   - Dependencies: This feature + Python Projection
4. Tree-sitter Import (Week 7-8):
   - Will add behavior model (SemAnno.behavior.mps) with import logic
   - Will integrate tree-sitter-python and tree-sitter-cpp
   - Will parse source files into AST
   - Dependencies: This feature + both projections
5. Warning System (Week 9-10):
   - Will add OptimizationLock enforcement logic
   - Will add warning UI to editors
   - Will add provenance tracking
   - Dependencies: All previous features
6. Blockers or constraints for next phases (if any)

**Definition of Done**:
- NEXT_PHASES.md created
- Clear roadmap for future features
- Dependencies documented
- No surprises in Phase 2+ implementation

---

## Phase 3 Completion Checkpoint

**Validation Checklist**:
- [ ] 5 test models created (SimpleFunction, ComplexFunction, NestedTypes, WithAnnotations, etc.)
- [ ] All models load and render without errors
- [ ] Serialization round-trips work (save → close → reopen)
- [ ] Language rebuild: zero errors, zero warnings
- [ ] Consistency checks pass
- [ ] Constraints work as designed
- [ ] Quickstart guide complete and clear
- [ ] NEXT_PHASES.md prepared

**Approval**: Feature 001 (Core AST Structure) is COMPLETE and ready for merge to parent branch.

---

## Summary

**Total Tasks**: 26
**Phases**: 3 (Research, Implementation, Testing)
**Estimated Duration**: 42-53 hours
**Status**: Ready for execution

**Next Step**: Begin Phase 1 (TASK-1-1) with core structure definition.

---

## Approval Signature

Once all tasks are complete and checkpoints pass:

```
Feature: 001 - Core AST Structure
Status: ✅ COMPLETE
Merged to: sprint-1-ast-redesign (parent branch)
Timestamp: [Completion date]
```
