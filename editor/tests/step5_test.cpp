// Step 5: Serialize Calculator AST to JSON, verify structure.

#include "../src/ast/Serialization.h"
#include <cassert>
#include <iostream>

int main() {
    // Build the Calculator model (same as step3)
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

    // --- Serialize ---
    json j = toJson(&calc);

    // Top-level structure
    assert(j["id"] == "Calc_M001");
    assert(j["concept"] == "Module");
    assert(j["properties"]["name"] == "Calculator");
    assert(j["properties"]["targetLanguage"] == "cpp");

    // Functions array
    assert(j["children"]["functions"].size() == 2);

    auto& jAdd = j["children"]["functions"][0];
    assert(jAdd["id"] == "Calc_F001");
    assert(jAdd["concept"] == "Function");
    assert(jAdd["properties"]["name"] == "add");

    // add's returnType
    assert(jAdd["children"]["returnType"].size() == 1);
    assert(jAdd["children"]["returnType"][0]["concept"] == "PrimitiveType");
    assert(jAdd["children"]["returnType"][0]["properties"]["kind"] == "int");

    // add's parameters
    assert(jAdd["children"]["parameters"].size() == 2);
    assert(jAdd["children"]["parameters"][0]["properties"]["name"] == "x");
    assert(jAdd["children"]["parameters"][1]["properties"]["name"] == "y");

    // Parameter type nested
    auto& jPx = jAdd["children"]["parameters"][0];
    assert(jPx["children"]["type"].size() == 1);
    assert(jPx["children"]["type"][0]["properties"]["kind"] == "int");

    // add's body
    assert(jAdd["children"]["body"].size() == 2);
    auto& jAssign = jAdd["children"]["body"][0];
    assert(jAssign["concept"] == "Assignment");
    assert(jAssign["children"]["target"].size() == 1);
    assert(jAssign["children"]["target"][0]["concept"] == "VariableReference");
    assert(jAssign["children"]["target"][0]["properties"]["variableName"] == "result");

    auto& jBinOp = jAssign["children"]["value"][0];
    assert(jBinOp["concept"] == "BinaryOperation");
    assert(jBinOp["properties"]["op"] == "+");
    assert(jBinOp["children"]["left"][0]["properties"]["variableName"] == "x");
    assert(jBinOp["children"]["right"][0]["properties"]["variableName"] == "y");

    // multiply function
    auto& jMul = j["children"]["functions"][1];
    assert(jMul["properties"]["name"] == "multiply");
    auto& jMulBody = jMul["children"]["body"][0];
    assert(jMulBody["concept"] == "Return");
    auto& jMulOp = jMulBody["children"]["value"][0];
    assert(jMulOp["properties"]["op"] == "*");

    // Variables
    assert(j["children"]["variables"].size() == 1);
    assert(j["children"]["variables"][0]["properties"]["name"] == "PI");
    assert(j["children"]["variables"][0]["children"]["type"][0]["properties"]["kind"] == "float");

    // Print for manual inspection
    std::cout << j.dump(2) << std::endl;
    std::cout << "\nStep 5: PASS — Calculator serialized to JSON" << std::endl;
    return 0;
}
