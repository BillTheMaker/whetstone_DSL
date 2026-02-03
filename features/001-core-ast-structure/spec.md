---
parent_branch: sprint-1-ast-redesign
feature_number: 001
status: In Progress
created_at: 2026-02-03T16:31:00Z
---

# Feature: Core AST Structure for SemAnno

## Overview

Enable developers to manually create and manipulate abstract syntax tree (AST) structures in MPS using the SemAnno language. This establishes the foundational data model for representing Python and C++ code as a unified, language-agnostic AST. The core AST captures the essential elements of both languages: functions, variables, statements, expressions, types, and annotations.

This feature is the prerequisite for all downstream features (Python projection, C++ projection, tree-sitter import, and warning system). Without a well-defined and properly-edited AST structure, the projections and generators cannot function.

## User Scenarios

### Scenario 1: Junior Developer Manually Creates a Function

**Actor**: Junior Python developer learning Whetstone DSL

**Flow**:
1. Developer opens MPS and creates a new SemAnno Module
2. Creates a Function concept with name "calculate_total"
3. Adds two Parameters: "items" (list of numbers) and "discount" (float)
4. Sets return type to "float"
5. Adds statements to the function body: a for loop iterating over items, an assignment to accumulate a total, an if statement to apply discount, and a return statement
6. Sees the function rendered in the MPS editor with proper syntax highlighting and indentation
7. Manually verifies the AST structure is correct by examining the model tree

**Outcome**: A complete, editable function AST exists in the model. The developer understands how to create and navigate AST nodes.

### Scenario 2: Developer Inspects Complex Type Structures

**Actor**: Developer working with nested data types

**Flow**:
1. Developer creates a Variable with type "list[map[string, widget]]"
2. Expands the type hierarchy and sees the nested structure: ListType → MapType → PrimitiveType
3. Adds another variable with type "optional[tuple[int, string, bool]]"
4. Verifies that tuple elements are correctly ordered and typed
5. Uses the editor's navigation to jump between type definitions and usages

**Outcome**: Complex nested types are correctly represented and navigable in the editor.

### Scenario 3: Developer Adds Annotations to AST Nodes

**Actor**: Senior developer preparing for optimization

**Flow**:
1. Developer creates a Function and manually attaches a DerefStrategy annotation
2. Sets the strategy to "batched"
3. Views the annotation in the editor as "@deref(batched)"
4. Later switches the strategy to "streamed" and sees the annotation update
5. Adds an OptimizationLock annotation with metadata: locked by "senior_dev_alice", reason "SIMD vectorization", warning level

**Outcome**: Annotations are properly attached to nodes, visible in the editor, and can be modified.

## Functional Requirements

### Core AST Nodes (Structure)

#### Requirement 1.1: Module Concept
A Module is the root container for all code. It must support:
- **name** property (string): identifier for the module
- **annotations** reference (multi-valued): collection of annotation nodes
- **variables** reference (multi-valued): module-level variables
- **functions** reference (multi-valued): top-level function definitions

#### Requirement 1.2: Function Concept
A Function represents a callable unit of code. It must support:
- **name** property (string): function identifier
- **parameters** reference (multi-valued): ordered list of Parameter nodes
- **returnType** reference (single): type reference for return value
- **body** reference (multi-valued): ordered list of Statement nodes
- **annotations** reference (multi-valued): annotation nodes attached to the function

#### Requirement 1.3: Parameter Concept
A Parameter represents a function input. It must support:
- **name** property (string): parameter identifier
- **type** reference (single): type reference for the parameter
- **defaultValue** reference (single, optional): optional default expression

#### Requirement 1.4: Variable Concept
A Variable represents a named data container. It must support:
- **name** property (string): variable identifier
- **type** reference (single): type reference for the variable
- **initializer** reference (single, optional): optional expression for initialization
- **annotations** reference (multi-valued): annotation nodes

#### Requirement 1.5: Abstract Statement Concept
An abstract Statement parent concept that all statement types inherit from. Concrete statement types:
- **Block**: container for multiple statements
- **Assignment**: assign an expression to a target (variable reference or member access)
- **IfStatement**: conditional execution with condition, thenBranch, and optional elseBranch
- **WhileLoop**: loop with condition and body statements
- **ForLoop**: iteration loop with iterator name, iterable expression, and body
- **Return**: return from function with optional expression
- **ExpressionStatement**: standalone expression (function call, etc.)

#### Requirement 1.6: Abstract Expression Concept
An abstract Expression parent concept. Concrete expression types:
- **BinaryOperation**: left operand, operator (string), right operand
- **UnaryOperation**: operator (string), operand
- **FunctionCall**: function name, ordered list of argument expressions
- **VariableReference**: variable name
- **Literal Types**: IntegerLiteral (value: integer), FloatLiteral (value: decimal), StringLiteral (value: string), BooleanLiteral (value: boolean), NullLiteral (no value)
- **ListLiteral**: ordered list of element expressions
- **IndexAccess**: target expression, index expression
- **MemberAccess**: target expression, member name (string)

### Type System (Structure)

#### Requirement 2.1: Abstract Type Concept
An abstract Type parent concept. Concrete type types:
- **PrimitiveType**: kind property (enum: int, float, string, bool)
- **ListType**: elementType reference (single)
- **SetType**: elementType reference (single)
- **MapType**: keyType reference (single), valueType reference (single)
- **TupleType**: elementTypes reference (multi-valued, ordered)
- **ArrayType**: elementType reference (single), size reference (single, expression for dimension)
- **OptionalType**: innerType reference (single)
- **CustomType**: typeName property (string, for user-defined types)

### Annotation Structures (Structure)

#### Requirement 3.1: DerefStrategy Annotation Concept
Represents a memory dereferencing strategy. Must support:
- **strategy** property (enum: imperative, streamed, batched, content_addressed)
- **derefTime** reference (single, optional): expression specifying when deref occurs (only for imperative)
- **derefLocation** property (string, optional): where deref happens (only for imperative)
- **owner** property (string, optional): agent reference

#### Requirement 3.2: OptimizationLock Annotation Concept
Represents a lock on optimization changes. Must support:
- **lockedBy** property (string): identifier of the agent who locked it
- **lockReason** property (string): explanation of why it was locked
- **lockLevel** property (enum: warning, soft, hard) – Sprint 1 uses "warning" only
- **affectedStrategies** property (string, multi-valued): names of affected deref strategies
- **timestamp** property (string): ISO datetime when lock was applied

#### Requirement 3.3: LangSpecific Annotation Concept
Preserves language-specific idioms during import. Must support:
- **language** property (enum: python, cpp, rust)
- **idiomType** property (string): "decorator", "template", "attribute", "pragma", etc.
- **rawSyntax** property (string): the original syntax as written
- **semanticHint** property (string, optional): "memoization", "generic", "parallelization", etc.
- **position** property (enum: before, after, wrapping): where it attaches to the node

### Editor Integration (Editor Model)

#### Requirement 4.1: Editor Rendering for AST Nodes
Each concept must have an editor definition that:
- Renders the node's structure readably
- Uses syntax highlighting for keywords (if, else, while, for, def, return)
- Uses color coding for different element types (types in blue, keywords in blue, strings in green, numbers in magenta)
- Supports inline property editing (names, operators, keywords)
- Supports reference selection (picking target concepts)
- Indents nested structures (blocks, function bodies)

#### Requirement 4.2: Projection-Ready Editors
Editors must be built to support dual projection (Python and C++ renderings will be added in later features):
- Module editor shows module name and lists functions/variables
- Function editor shows signature and body with clear statement structure
- Statement editors show statement type-specific syntax (if/else indentation, loop structure, etc.)
- Expression editors render operators and operands clearly
- Type editors show type hierarchy (list[map[...]] nesting)

### Testability

#### Requirement 5.1: Manual AST Creation Tests
The language must allow users to:
- Create a Module concept manually in MPS
- Add Functions with proper parameters and return types
- Add Statements to function bodies and have them parse correctly
- Add Expressions with operators and operands
- Add Type references and verify nesting
- All created structures must be serializable to the .mps model file and loadable without errors

#### Requirement 5.2: Property and Reference Integrity
- All properties must accept and save their defined types (strings, enums, etc.)
- All references must accept only the correct concept types
- Deletion of a referenced node must trigger appropriate error or cleanup
- The editor must prevent creating invalid structures (e.g., statements outside of statement lists, expressions in the wrong context)

## Success Criteria

1. **Manual Creation**: A developer can manually create a Module with at least one Function containing statements and expressions in MPS, with no red error marks in the error list.

2. **AST Completeness**: The created AST models all required concepts from Req 1.1-1.6 and 2.1 with proper hierarchy and reference structure.

3. **Editor Usability**: The editor rendering supports selecting, modifying, and deleting AST nodes without requiring manual XML editing.

4. **Type System**: Complex nested types (e.g., list[map[string, optional[int]]]) can be created and rendered correctly in the editor.

5. **Annotations**: DerefStrategy, OptimizationLock, and LangSpecific annotations can be created and attached to appropriate nodes (functions, variables, statements).

6. **Model Persistence**: Created AST models save to .mps files without corruption and reload without errors.

7. **Zero Errors**: The SemAnno language compiles with zero errors and zero warnings when building and rebuilding.

8. **Test Model**: A test model exists in the project demonstrating a non-trivial AST (function with multiple statements, nested types, annotations) that serves as a reference for downstream features.

## Key Entities

- **Module**: Root AST container
- **Function**: Callable unit with parameters, return type, and body
- **Parameter**: Named input to a function with a type
- **Variable**: Named data container with type and optional initializer
- **Statement**: Abstract parent for Block, Assignment, IfStatement, WhileLoop, ForLoop, Return, ExpressionStatement
- **Expression**: Abstract parent for operations, calls, literals, references, and access patterns
- **Type**: Abstract parent for primitive types, collections (list, set, map, tuple, array), optional types, and custom types
- **Annotation**: Metadata attached to AST nodes (DerefStrategy, OptimizationLock, LangSpecific)

## Assumptions

1. **MPS Version**: JetBrains MPS 2024.3 or later is available and properly configured.

2. **Single Language Module**: All AST concepts, editors, and supporting models are defined in the SemAnno language module. No separate modules (e.g., "WhetstoneCore") are created during this phase.

3. **Manual Creation**: No automatic code generation or tree-sitter import is performed in this feature. AST creation is entirely manual via the MPS editor.

4. **Stateless Annotations**: Annotations are passive metadata. No annotation-specific behavior (e.g., warning logic) is implemented until the Warning System feature (Week 9-10).

5. **Default Type Bindings**: When a function or variable is created without an explicit type, the editor provides a default (e.g., OptionalType or a reasonable primitive).

6. **English-Based Names**: All concept names, properties, and references use English identifiers (no localization).

7. **Linear Statement Ordering**: Statements are always ordered (Block contains an ordered list), preserving the logical flow of code.

8. **Reference Resolution**: MPS's built-in reference resolution is used for all concept-to-concept references. No custom resolution logic is needed.

9. **No Cross-Module References**: AST nodes do not reference concepts outside the SemAnno language during this feature.

10. **Single Root Module**: Each .mps model file contains exactly one Module instance (the root of the AST).
