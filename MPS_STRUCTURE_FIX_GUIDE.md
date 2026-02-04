# MPS Structure ID Fix Guide

## Problem
The SemAnno.structure.mps file contains ~30 Phase 1 concepts with duplicate conceptId (EcuMT) values, causing "Duplicate concept id" errors when rebuilding the language.

## Solution: Use MPS "Correct ID" Intention

### Step 1: Open the Project in MPS
1. Launch JetBrains MPS 2024.3 (or compatible)
2. Open the Whetstone_DSL project
3. Navigate to **Languages > SemAnno > models > SemAnno.structure**

### Step 2: Rebuild to See Errors
1. Right-click the **SemAnno** language module
2. Select **Rebuild Language**
3. Wait for the build to complete
4. You'll see red error marks on concepts with duplicate IDs in the structure tree

### Step 3: Fix Each Concept Using "Correct ID"

For each concept with a red error mark:

1. **Click on the concept node** (e.g., "Module", "Function", "Block", etc.)
   - Look for the red highlighted node in the tree

2. **Open the Intentions menu**:
   - **macOS**: Cmd + Alt + Return
   - **Windows/Linux**: Ctrl + Alt + Return
   - Or: Right-click > Intentions

3. **Select "Correct ID"** from the menu
   - MPS will auto-generate a unique ID
   - The red error should disappear

4. **Repeat for all Phase 1 concepts**:

   These are the ~30 concepts you need to fix (in this order for efficiency):

   **Core AST Nodes:**
   - [ ] Module
   - [ ] Function
   - [ ] Parameter
   - [ ] Variable

   **Statements:**
   - [ ] Statement (abstract)
   - [ ] Block
   - [ ] Assignment
   - [ ] IfStatement
   - [ ] WhileLoop
   - [ ] ForLoop
   - [ ] Return
   - [ ] ExpressionStatement

   **Expressions:**
   - [ ] Expression (abstract)
   - [ ] BinaryOperation
   - [ ] UnaryOperation
   - [ ] FunctionCall
   - [ ] VariableReference
   - [ ] IntegerLiteral
   - [ ] FloatLiteral
   - [ ] StringLiteral
   - [ ] BooleanLiteral
   - [ ] NullLiteral
   - [ ] ListLiteral
   - [ ] IndexAccess
   - [ ] MemberAccess

   **Types:**
   - [ ] Type (abstract)
   - [ ] PrimitiveType
   - [ ] ListType
   - [ ] SetType
   - [ ] MapType
   - [ ] TupleType
   - [ ] ArrayType
   - [ ] OptionalType
   - [ ] CustomType

   **Annotations:**
   - [ ] Annotation (abstract)
   - [ ] DerefStrategy
   - [ ] OptimizationLock
   - [ ] LangSpecific

### Step 4: Rebuild and Verify

1. After fixing all concepts, right-click SemAnno again
2. Select **Rebuild Language**
3. **Expected Result**: Build completes with **zero errors, zero warnings**

### Step 5: Commit Changes

Once the build is clean:

```bash
cd /e/Whetstone_DSL
git add languages/SemAnno/models/SemAnno.structure.mps
git commit -m "Fix duplicate concept IDs using MPS Correct ID intention

All Phase 1 concepts now have unique IDs:
- Module, Function, Parameter, Variable
- 7 Statement types
- 13 Expression types
- 8 Type types
- 3 Annotation types

Language builds with zero errors after ID correction."
```

## Troubleshooting

### Problem: "Correct ID" intention not appearing
**Solution**:
- Make sure you clicked on the concept node (ConceptDeclaration), not just a child element
- The node should be the one with the `<node concept="1TIwiD" ...>` tag
- Try pressing Ctrl+Alt+Return again

### Problem: Still seeing duplicate ID error after "Correct ID"
**Solution**:
- The concept might have child elements (properties/links) that also need ID fixing
- Run "Correct ID" on any child elements with red highlights
- Rebuild language again

### Problem: Build still shows errors after all concepts fixed
**Solution**:
- Close the project and reopen it
- File > Invalidate Caches and Restart
- Try rebuilding again

## What "Correct ID" Does

The "Correct ID" intention:
1. Generates a new unique conceptId value for the selected concept
2. Updates all internal references to use the new ID
3. Updates property IDs (IQ2nx) and link IDs (IQ2ns) if needed
4. Maintains all the concept's structure and properties

This is the standard MPS way to fix ID conflicts.

## After IDs Are Fixed

Once the structure file is clean:

1. I will recreate the editor definitions for all concepts
2. Provide comprehensive test instructions
3. You'll be able to create test AST models in MPS

## Questions?

If you encounter issues during this process, let me know the specific error message and which concept was being fixed, and I can provide additional guidance.

---

**Expected Duration**: 10-15 minutes to fix all 30+ concepts
**Difficulty**: Low - just click and use the intention menu repeatedly
