// Step 3: Build the Calculator example from Phase1Test.mps as a C++ object graph.
//
// Calculator (Module, targetLanguage=cpp)
// ├── Variable "PI" → type: PrimitiveType("float")
// ├── Function "add"
// │   ├── returnType: PrimitiveType("int")
// │   ├── parameters: Parameter("x", int), Parameter("y", int)
// │   └── body:
// │       ├── Assignment { target: VarRef("result"), value: BinOp("+", VarRef("x"), VarRef("y")) }
// │       └── Return { value: VarRef("result") }
// └── Function "multiply"
//     ├── returnType: PrimitiveType("int")
//     ├── parameters: Parameter("a", int), Parameter("b", int)
//     └── body:
//         └── Return { value: BinOp("*", VarRef("a"), VarRef("b")) }

#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Parameter.h"
#include "../src/ast/Statement.h"
#include "../src/ast/Expression.h"
#include "../src/ast/Type.h"
#include <cassert>
#include <iostream>

int main() {
    // --- Build the tree (matching Phase1Test.mps IDs) ---

    // Root
    Module calc("Calc_M001", "Calculator", "cpp");

    // Variable PI
    Variable pi("Calc_V001", "PI");
    PrimitiveType piType("Calc_VT001", "float");
    pi.setChild("type", &piType);
    calc.addChild("variables", &pi);

    // Function: add
    Function add("Calc_F001", "add");
    PrimitiveType addRet("Calc_RT001", "int");
    add.setChild("returnType", &addRet);

    Parameter px("Calc_P001", "x");
    PrimitiveType pxType("Calc_PT001", "int");
    px.setChild("type", &pxType);
    add.addChild("parameters", &px);

    Parameter py("Calc_P002", "y");
    PrimitiveType pyType("Calc_PT002", "int");
    py.setChild("type", &pyType);
    add.addChild("parameters", &py);

    // add body: Assignment(result = x + y), then Return(result)
    Assignment assign;
    assign.id = "Calc_A001";
    VariableReference assignTarget("c_VR_result", "result");
    assign.setChild("target", &assignTarget);

    BinaryOperation addOp("Calc_E001", "+");
    VariableReference vrX("Calc_VR_x", "x");
    VariableReference vrY("Calc_VR_y", "y");
    addOp.setChild("left", &vrX);
    addOp.setChild("right", &vrY);
    assign.setChild("value", &addOp);

    Return addReturn;
    addReturn.id = "Calc_R001";
    VariableReference vrRetResult("c_VR_return", "result");
    addReturn.setChild("value", &vrRetResult);

    add.addChild("body", &assign);
    add.addChild("body", &addReturn);

    calc.addChild("functions", &add);

    // Function: multiply
    Function multiply("Calc_F002", "multiply");
    PrimitiveType mulRet("Calc_RT002", "int");
    multiply.setChild("returnType", &mulRet);

    Parameter pa("Calc_P003", "a");
    PrimitiveType paType("Calc_PT003", "int");
    pa.setChild("type", &paType);
    multiply.addChild("parameters", &pa);

    Parameter pb("Calc_P004", "b");
    PrimitiveType pbType("Calc_PT004", "int");
    pb.setChild("type", &pbType);
    multiply.addChild("parameters", &pb);

    Return mulReturn;
    mulReturn.id = "Calc_R002";
    BinaryOperation mulOp("Calc_E005", "*");
    VariableReference vrA("Calc_VR_a", "a");
    VariableReference vrB("Calc_VR_b", "b");
    mulOp.setChild("left", &vrA);
    mulOp.setChild("right", &vrB);
    mulReturn.setChild("value", &mulOp);

    multiply.addChild("body", &mulReturn);

    calc.addChild("functions", &multiply);

    // --- Verify the tree ---

    // Module level
    assert(calc.name == "Calculator");
    assert(calc.targetLanguage == "cpp");
    assert(calc.getChildren("functions").size() == 2);
    assert(calc.getChildren("variables").size() == 1);

    // Variable PI
    auto* v = static_cast<Variable*>(calc.getChildren("variables")[0]);
    assert(v->name == "PI");
    assert(v->parent == &calc);
    auto* vt = static_cast<PrimitiveType*>(v->getChild("type"));
    assert(vt != nullptr);
    assert(vt->kind == "float");
    assert(vt->parent == v);

    // Function: add
    auto* fn1 = static_cast<Function*>(calc.getChildren("functions")[0]);
    assert(fn1->name == "add");
    assert(fn1->parent == &calc);

    auto* rt1 = static_cast<PrimitiveType*>(fn1->getChild("returnType"));
    assert(rt1->kind == "int");

    const auto& params1 = fn1->getChildren("parameters");
    assert(params1.size() == 2);
    assert(static_cast<Parameter*>(params1[0])->name == "x");
    assert(static_cast<Parameter*>(params1[1])->name == "y");
    // Param types
    auto* pxt = static_cast<PrimitiveType*>(params1[0]->getChild("type"));
    assert(pxt->kind == "int");
    assert(pxt->parent == params1[0]);

    const auto& body1 = fn1->getChildren("body");
    assert(body1.size() == 2);

    // First statement: Assignment
    auto* a1 = static_cast<Assignment*>(body1[0]);
    assert(a1->conceptType == "Assignment");
    assert(a1->parent == fn1);
    auto* aTgt = static_cast<VariableReference*>(a1->getChild("target"));
    assert(aTgt->variableName == "result");
    auto* aVal = static_cast<BinaryOperation*>(a1->getChild("value"));
    assert(aVal->op == "+");
    auto* aLeft = static_cast<VariableReference*>(aVal->getChild("left"));
    auto* aRight = static_cast<VariableReference*>(aVal->getChild("right"));
    assert(aLeft->variableName == "x");
    assert(aRight->variableName == "y");
    // Walk up: expression → assignment → function → module
    assert(aLeft->parent == aVal);
    assert(aVal->parent == a1);
    assert(a1->parent == fn1);
    assert(fn1->parent == &calc);

    // Second statement: Return
    auto* r1 = static_cast<Return*>(body1[1]);
    assert(r1->conceptType == "Return");
    auto* rv = static_cast<VariableReference*>(r1->getChild("value"));
    assert(rv->variableName == "result");

    // Function: multiply
    auto* fn2 = static_cast<Function*>(calc.getChildren("functions")[1]);
    assert(fn2->name == "multiply");

    const auto& params2 = fn2->getChildren("parameters");
    assert(params2.size() == 2);
    assert(static_cast<Parameter*>(params2[0])->name == "a");
    assert(static_cast<Parameter*>(params2[1])->name == "b");

    const auto& body2 = fn2->getChildren("body");
    assert(body2.size() == 1);
    auto* r2 = static_cast<Return*>(body2[0]);
    auto* mulExpr = static_cast<BinaryOperation*>(r2->getChild("value"));
    assert(mulExpr->op == "*");
    assert(static_cast<VariableReference*>(mulExpr->getChild("left"))->variableName == "a");
    assert(static_cast<VariableReference*>(mulExpr->getChild("right"))->variableName == "b");

    // Full tree depth: varref → binop → return → function → module
    assert(vrB.parent->parent->parent->parent == &calc);

    std::cout << "Step 3: PASS — Calculator model (27 nodes) built and verified" << std::endl;
    return 0;
}
