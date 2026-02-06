---
feature_number: 001
feature_name: Core AST Structure
status: Planning
created_at: 2026-02-03T16:45:00Z
spec_version: 1.0
---

# Implementation Plan: Core AST Structure for SemAnno

## Technical Context

**Technology Stack**:
- **Workbench**: JetBrains MPS 2024.3+
- **Target Language**: SemAnno (language definition in MPS)
- **Model Format**: MPS XML (.mps files)
- **Development Method**: Incremental concept definition with immediate editor testing

**Key Decisions**:
- All concepts defined in single SemAnno language module (no separate WhetstoneCore module)
- Editors built in SemAnno.editor.mps with projection support for Python/C++ (added in future features)
- No code generation or external tools required for this feature
- Type system uses reference composition for nested type structures (ListType contains reference to inner Type)

**Dependencies**:
- MPS 2024.3 devkit for language design
- None on external libraries or frameworks
- Builds on existing SemAnno structure model (assumed to exist from previous session)

## Constitution Check

**Project Constitution Principles** (from `.specswarm/constitution.md` or assumed defaults):
- Clarity over cleverness: ✅ All concepts have clear, intuitive semantics
- Consistency: ✅ Naming conventions and patterns applied uniformly
- Type safety: ✅ Strong typing via MPS reference system
- Testability: ✅ Manual AST creation is the test mechanism
- Documentation: ✅ Specification documents design rationale

**No conflicting principles detected** - proceeding with implementation.

## Implementation Phases

### Phase 0: Research & Exploration

**Objectives**: Resolve any ambiguities, document assumptions, identify edge cases

**Tasks**:

1. **Research Task 1: MPS Editor Design Patterns**
   - Question: How are projection-ready editors structured in MPS for multi-projection languages?
   - Goal: Understand patterns used in multi-view languages (e.g., JetBrains MPS itself with structure/interface views)
   - Deliverable: Quick reference for editor cell patterns that support projection switching
   - Owner: Implementation team (research before Phase 1)

2. **Research Task 2: Nested Type Composition in MPS**
   - Question: Best practices for building deeply nested type structures (list[map[string, optional[int]]])
   - Goal: Ensure type references are properly composable without circular dependencies
   - Deliverable: Sample MPS structure showing 3-4 levels of type nesting
   - Owner: Implementation team

3. **Research Task 3: Annotation Attachment Patterns**
   - Question: How do MPS languages attach multiple annotations to a single concept?
   - Goal: Ensure annotations are optional, ordered, and independently removable
   - Deliverable: Example annotation structure from existing MPS language (e.g., jetbrains.mps.lang.core)
   - Owner: Implementation team

**Status**: Research items are light and can be completed during Phase 1 implementation (not blocking)

---

### Phase 1: Core Structure Definition

**Objectives**: Define all 30+ AST concepts in SemAnno.structure.mps

**Deliverables**:
- SemAnno.structure.mps with all concepts defined (Module, Function, Variable, all Statement types, all Expression types, all Type types, annotation concepts)
- Preliminary constraints model (SemAnno.constraints.mps) with basic validation rules
- Test model file (e.g., `languages/SemAnno/tests/SimpleExample.mps`) demonstrating a complete AST

**Key Subtasks**:

#### 1.1: Root & Container Concepts (1-2 hours)
- Module concept with properties (name: string) and references (annotations, variables, functions)
- Marker: "Container concepts complete"

#### 1.2: Function & Parameter Concepts (2-3 hours)
- Function concept: name, parameters[], returnType, body[], annotations
- Parameter concept: name, type, defaultValue?
- Establish parameter ordering and type reference patterns
- Marker: "Function signature complete"

#### 1.3: Variable Concept (1 hour)
- Variable concept: name, type, initializer?, annotations
- Ensure type and initializer reference logic
- Marker: "Variable concept complete"

#### 1.4: Statement Concepts (4-5 hours)
- Abstract Statement parent
- Concrete: Block, Assignment, IfStatement, WhileLoop, ForLoop, Return, ExpressionStatement
- Each with appropriate properties and references
- Marker: "All 7 statement types complete"

#### 1.5: Expression Concepts (4-5 hours)
- Abstract Expression parent
- Binary/Unary operations with operator properties
- Function call, variable reference, literal types (5 literal types)
- List literal, index access, member access
- Total: 13 expression concept types
- Marker: "All 13 expression types complete"

#### 1.6: Type Concepts (3-4 hours)
- Abstract Type parent
- Concrete: PrimitiveType (with enum), ListType, SetType, MapType, TupleType, ArrayType, OptionalType, CustomType
- 8 concrete type concepts
- Ensure ListType.elementType references Type (composable)
- MapType.keyType and valueType both reference Type
- TupleType.elementTypes are ordered, multi-valued Type references
- Marker: "All 8 type concepts complete, nesting tested"

#### 1.7: Annotation Concepts (2-3 hours)
- Abstract Annotation parent (optional, for future extensibility)
- DerefStrategy: strategy enum, derefTime?, derefLocation?, owner?
- OptimizationLock: lockedBy, lockReason, lockLevel enum, affectedStrategies[], timestamp
- LangSpecific: language enum, idiomType, rawSyntax, semanticHint?, position enum
- Marker: "All 3 annotation types complete"

#### 1.8: Constraints & Validation (2 hours)
- Create SemAnno.constraints.mps
- Basic constraints: statements only in statement lists, expressions only in expression contexts, types reference only Type concepts
- Can-be-parent/can-be-child rules to prevent invalid structures
- Marker: "Constraints model complete"

#### 1.9: Test Model Creation (2-3 hours)
- Create `languages/SemAnno/tests/SimpleExample.mps` demonstrating:
  - 1 Module with name "example"
  - 1 Function "sum" with 2 parameters (items: list[int], factor: float), return type: float
  - Function body with: ForLoop (iterate items), Assignment (accumulate), IfStatement (conditional factor), Return
  - 1 Variable with complex type: optional[map[string, list[int]]]
  - Annotations: DerefStrategy on function, OptimizationLock metadata, LangSpecific on variable
- Verify model loads without errors
- Marker: "Test model created and validated"

**Estimated Duration**: 20-25 hours of focused implementation

**Quality Gates**:
- SemAnno language compiles with zero errors
- Test model loads and renders without errors
- All concept definitions are complete (no stub concepts)

---

### Phase 2: Editor Definition

**Objectives**: Define editors for all concepts in SemAnno.editor.mps so AST is manually creatable and editable in MPS

**Deliverables**:
- SemAnno.editor.mps with editors for all 30+ concepts
- Editors use MPS cell models (constant, property, reference cells) with layout (horizontal, vertical, indent)
- Syntax highlighting: keywords in blue (def, return, if, for, while), strings in green, numbers in magenta, types in blue
- No projection switching yet (that comes in Phase 2 of Python/C++ features)

**Key Subtasks**:

#### 2.1: Container & Function Editors (2-3 hours)
- Module editor: "module" keyword, name property, vertical list of variables and functions
- Function editor: "def" keyword, name, parameters in parens separated by commas, "->", return type, ":", function body with indentation
- Parameter editor: name, ":", type reference
- Block editor: indented statements vertically

#### 2.2: Statement Editors (4-5 hours)
- Assignment: target expression, "=", value expression
- IfStatement: "if" keyword (blue), condition, ":", then-branch indented, optional "else:" keyword, else-branch indented
- WhileLoop: "while" keyword, condition, ":", body indented
- ForLoop: "for" keyword, iterator name, "in" keyword, iterable expression, ":", body indented
- Return: "return" keyword, optional return expression
- ExpressionStatement: expression only
- Indentation and keyword highlighting applied consistently

#### 2.3: Expression Editors (3-4 hours)
- BinaryOperation: left operand, operator property, right operand
- UnaryOperation: operator, operand
- FunctionCall: function name, "(", arguments comma-separated, ")"
- VariableReference: variable name property
- Literal editors: each shows its value with appropriate syntax highlighting (numbers, strings, keywords)
- ListLiteral: "[", elements comma-separated, "]"
- IndexAccess: target, "[", index, "]"
- MemberAccess: target, ".", member name

#### 2.4: Type Editors (2-3 hours)
- PrimitiveType: enum displayed as name (int, float, string, bool)
- ListType: "[", element type, "]"
- SetType: "set[", element type, "]"
- MapType: "map[", key type, ",", value type, "]"
- TupleType: "tuple[", element types comma-separated, "]"
- ArrayType: type, "[", size expression, "]"
- OptionalType: type, "?"
- CustomType: type name property
- Type nesting renders naturally: "list[map[string, int]]"

#### 2.5: Annotation Editors (1-2 hours)
- DerefStrategy: "@deref(", strategy enum, optional properties in comments/sub-lines, ")"
- OptimizationLock: "@lock(", lockedBy, ",", lockReason, optional properties, ")"
- LangSpecific: "@lang_specific(", language, ",", idiomType, ",", rawSyntax, optional properties, ")"
- Annotations render near the node they modify (before/after depending on position)

#### 2.6: Editor Testing in MPS (2-3 hours)
- Rebuild language in MPS
- Manually create Module in test model
- Test each concept's editor: create, edit properties, add references
- Verify indentation, syntax highlighting, navigation work correctly
- Test nested structures (function → body → statements → expressions → nested types)

**Estimated Duration**: 15-18 hours

**Quality Gates**:
- All editors render without cell errors
- All properties are editable inline
- All references are selectable from dropdown/search
- Indentation and syntax highlighting work as specified
- Nested structures (functions with statements, statements with expressions, types with types) display correctly

---

### Phase 3: Integration & Testing

**Objectives**: Verify AST structure works end-to-end, document for next phase, prepare for Python/C++ projections

**Deliverables**:
- Comprehensive test model (SemAnno.tests.mps) with multiple examples
- Behavior model stubs (SemAnno.behavior.mps) - empty for now, but structure in place
- Typesystem model stub (SemAnno.typesystem.mps) - empty for now
- Quick-start guide for creating AST in MPS

**Key Subtasks**:

#### 3.1: Extended Test Model (2-3 hours)
- Add 3-5 example functions covering:
  - Simple function (no loops or conditionals)
  - Function with nested loops and conditionals
  - Function with complex types (nested collections, optional types)
  - Function with annotations (DerefStrategy, OptimizationLock)
  - Function with language-specific idiom annotations
- Verify all examples load and render without errors

#### 3.2: Model Serialization Tests (1-2 hours)
- Save test models to disk (automatic via MPS)
- Close and reopen MPS
- Reload test models and verify no corruption
- Verify all editor state preserved

#### 3.3: Quality & Correctness Checks (1-2 hours)
- Run SemAnno language rebuild: zero errors, zero warnings
- Run MPS consistency checks on test models
- Verify no dangling references or broken hierarchy
- Document any edge cases found (e.g., circular type references if possible)

#### 3.4: Documentation & Quickstart (1-2 hours)
- Create `languages/SemAnno/QUICKSTART.md` with steps to:
  - Create a new SemAnno model
  - Add a Module and Function
  - Add parameters and statements
  - Create type references
  - Add annotations
- Include screenshots or ASCII examples of expected editor rendering

#### 3.5: Prepare for Phase 2 (Python Projection) (1 hour)
- Note any editor patterns that will need enhancement for projections
- Document how Python/C++ projection editors will overlay the core AST editors
- Ensure no blocking issues for projection implementation

**Estimated Duration**: 7-10 hours

**Quality Gates**:
- All test models load, render, and save without errors
- Serialization round-trips (save→close→reopen) work correctly
- Zero errors from MPS language rebuild and consistency checks
- Quickstart documentation is clear and complete

---

## Phase Completion Criteria

### Phase 1 Completion
- [ ] All 30+ concept definitions in SemAnno.structure.mps complete
- [ ] SemAnno.constraints.mps defines validation rules
- [ ] SimpleExample.mps test model created with multi-statement, multi-type function
- [ ] MPS rebuild produces zero errors, zero warnings

### Phase 2 Completion
- [ ] All concept editors defined in SemAnno.editor.mps
- [ ] Manual AST creation works in MPS (can create Module → Function → Statements → Expressions)
- [ ] Syntax highlighting, indentation, and reference selection work correctly
- [ ] Complex nested types display correctly (e.g., "list[map[string, optional[int]]]")

### Phase 3 Completion
- [ ] Extended test models (3-5 examples) all load without errors
- [ ] Save/close/reopen cycle works without corruption
- [ ] Zero errors from language rebuild and consistency checks
- [ ] Quickstart guide complete and documented

---

## Data Model

### Entities

**Module** (root AST)
- Properties: name (string)
- References: annotations (Annotation*), variables (Variable*), functions (Function*)

**Function**
- Properties: name (string)
- References: parameters (Parameter*), returnType (Type), body (Statement*), annotations (Annotation*)

**Parameter**
- Properties: name (string)
- References: type (Type), defaultValue (Expression?)

**Variable**
- Properties: name (string)
- References: type (Type), initializer (Expression?), annotations (Annotation*)

**Statement** (abstract)
- Subtypes: Block, Assignment, IfStatement, WhileLoop, ForLoop, Return, ExpressionStatement

**Block**
- References: statements (Statement*)

**Assignment**
- References: target (Expression), value (Expression)

**IfStatement**
- References: condition (Expression), thenBranch (Statement*), elseBranch (Statement*)

**WhileLoop**
- References: condition (Expression), body (Statement*)

**ForLoop**
- Properties: iteratorName (string)
- References: iterable (Expression), body (Statement*)

**Return**
- References: value (Expression?)

**ExpressionStatement**
- References: expression (Expression)

**Expression** (abstract)
- Subtypes: BinaryOperation, UnaryOperation, FunctionCall, VariableReference, Literals, ListLiteral, IndexAccess, MemberAccess

**BinaryOperation**
- Properties: operator (string)
- References: left (Expression), right (Expression)

**UnaryOperation**
- Properties: operator (string)
- References: operand (Expression)

**FunctionCall**
- Properties: functionName (string)
- References: arguments (Expression*)

**VariableReference**
- Properties: variableName (string)

**Literals**
- IntegerLiteral: value (int)
- FloatLiteral: value (float)
- StringLiteral: value (string)
- BooleanLiteral: value (bool)
- NullLiteral: (no properties)

**ListLiteral**
- References: elements (Expression*)

**IndexAccess**
- References: target (Expression), index (Expression)

**MemberAccess**
- Properties: memberName (string)
- References: target (Expression)

**Type** (abstract)
- Subtypes: PrimitiveType, ListType, SetType, MapType, TupleType, ArrayType, OptionalType, CustomType

**PrimitiveType**
- Properties: kind (enum: int, float, string, bool)

**ListType**
- References: elementType (Type)

**SetType**
- References: elementType (Type)

**MapType**
- References: keyType (Type), valueType (Type)

**TupleType**
- References: elementTypes (Type*)

**ArrayType**
- References: elementType (Type), size (Expression)

**OptionalType**
- References: innerType (Type)

**CustomType**
- Properties: typeName (string)

**Annotation** (abstract)
- Subtypes: DerefStrategy, OptimizationLock, LangSpecific

**DerefStrategy**
- Properties: strategy (enum: imperative, streamed, batched, content_addressed), derefLocation (string?), owner (string?)
- References: derefTime (Expression?)

**OptimizationLock**
- Properties: lockedBy (string), lockReason (string), lockLevel (enum: warning, soft, hard), affectedStrategies (string*), timestamp (string)

**LangSpecific**
- Properties: language (enum: python, cpp, rust), idiomType (string), rawSyntax (string), semanticHint (string?), position (enum: before, after, wrapping)

---

## Risk Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|-----------|
| Circular type references (e.g., Type → Type) | Medium | High | Research MPS patterns during Phase 1; use composition, not inheritance |
| Editor cell model complexity | Medium | Medium | Reference MPS documentation and examples; iterate incrementally |
| Missing statement/expression types | Low | Medium | Verify against spec requirements before starting; use checklist |
| Annotation attachment logic issues | Low | Medium | Test annotation creation manually; verify multi-valued references work |
| Serialization/persistence issues | Low | High | Test save/close/reopen cycle extensively in Phase 3 |

---

## Timeline Estimate

**Total Duration**: 42-53 hours (5-7 working days at 8 hours/day, or 2-3 weeks part-time)

- Phase 0 (Research): Concurrent with Phase 1, not blocking
- Phase 1 (Structure): 20-25 hours
- Phase 2 (Editors): 15-18 hours
- Phase 3 (Testing & Docs): 7-10 hours

---

## Success Criteria (from Specification)

1. ✅ **Manual Creation**: Developer can create Module with Function, statements, expressions in MPS with no red errors
2. ✅ **AST Completeness**: All concepts from Req 1.1-1.6 and 2.1 defined and working
3. ✅ **Editor Usability**: Selecting, modifying, deleting nodes works in editor (no manual XML editing)
4. ✅ **Type System**: Complex nested types render correctly
5. ✅ **Annotations**: DerefStrategy, OptimizationLock, LangSpecific can be created and attached
6. ✅ **Model Persistence**: Models save and reload without corruption
7. ✅ **Zero Errors**: Language compiles with zero errors, zero warnings
8. ✅ **Test Model**: Reference test model with multi-statement functions and annotations exists

---

## Next Feature Prerequisites

Once Phase 3 is complete:
- Python Projection feature can begin (uses core AST + Python-specific editors)
- C++ Projection feature can begin (uses core AST + C++-specific editors)
- Behavior model (SemAnno.behavior.mps) can be populated with import logic
- Typesystem model (SemAnno.typesystem.mps) can define type checking rules

No blockers expected. Core AST is standalone and self-contained.
