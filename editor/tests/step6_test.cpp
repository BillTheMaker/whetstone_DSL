// Step 6: JSON round-trip. Save Calculator → load → save again → compare (byte-identical).

#include "../src/ast/Serialization.h"
#include <cassert>
#include <iostream>

int main() {
    // Build Calculator (same as step5)
    Module calc("Calc_M001", "Calculator", "cpp");

    Variable pi("Calc_V001", "PI");
    PrimitiveType piType("Calc_VT001", "float");
    pi.setChild("type", &piType);
    calc.addChild("variables", &pi);

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
    add.addChild("body", &assign);

    Return addReturn;
    addReturn.id = "Calc_R001";
    VariableReference vrRetResult("c_VR_return", "result");
    addReturn.setChild("value", &vrRetResult);
    add.addChild("body", &addReturn);
    calc.addChild("functions", &add);

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

    // --- Round-trip ---

    // Save 1
    json j1 = toJson(&calc);
    std::string s1 = j1.dump(2);

    // Load
    ASTNode* loaded = fromJson(j1);
    assert(loaded != nullptr);

    // Verify loaded tree structure
    assert(loaded->conceptType == "Module");
    auto* lm = static_cast<Module*>(loaded);
    assert(lm->name == "Calculator");
    assert(lm->targetLanguage == "cpp");
    assert(lm->getChildren("functions").size() == 2);
    assert(lm->getChildren("variables").size() == 1);

    // Verify deep node
    auto* lf = static_cast<Function*>(lm->getChildren("functions")[0]);
    assert(lf->name == "add");
    assert(lf->parent == loaded);
    assert(lf->getChildren("parameters").size() == 2);
    assert(lf->getChildren("body").size() == 2);

    auto* lp = static_cast<Parameter*>(lf->getChildren("parameters")[0]);
    assert(lp->name == "x");
    assert(lp->parent == lf);
    auto* lpt = static_cast<PrimitiveType*>(lp->getChild("type"));
    assert(lpt->kind == "int");
    assert(lpt->parent == lp);

    auto* la = static_cast<Assignment*>(lf->getChildren("body")[0]);
    auto* lbo = static_cast<BinaryOperation*>(la->getChild("value"));
    assert(lbo->op == "+");
    assert(static_cast<VariableReference*>(lbo->getChild("left"))->variableName == "x");

    // Save 2
    json j2 = toJson(loaded);
    std::string s2 = j2.dump(2);

    // Compare: must be byte-identical
    if (s1 != s2) {
        std::cerr << "MISMATCH!\n--- Save 1 ---\n" << s1
                  << "\n--- Save 2 ---\n" << s2 << std::endl;
        assert(false && "Round-trip JSON not identical");
    }

    // Cleanup heap-allocated tree
    deleteTree(loaded);

    std::cout << "Step 6: PASS — JSON round-trip (save/load/save identical)" << std::endl;
    return 0;
}
