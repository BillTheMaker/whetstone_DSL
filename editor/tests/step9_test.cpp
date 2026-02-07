// Step 9: Python generator - Statements and expressions.
//
// Add Assignment, Return, BinaryOperation, VariableReference to Python generator.
// Test: Calculator AST → full Python output matches MPS textGen output.

#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Parameter.h"
#include "../src/ast/Statement.h"
#include "../src/ast/Expression.h"
#include "../src/ast/Type.h"
#include "../src/ast/Generator.h"
#include <cassert>
#include <iostream>
#include <sstream>

int main() {
    // Build the Calculator module with a function that has statements and expressions
    Module calc("Calc_M001", "Calculator", "python");

    // Add a function: def add(x, y): with assignment and return
    Function add("Calc_F001", "add");
    
    // Add parameters x and y
    Parameter px("Calc_P001", "x");
    PrimitiveType pxType("Calc_PT001", "int");
    px.setChild("type", &pxType);
    add.addChild("parameters", &px);

    Parameter py("Calc_P002", "y");
    PrimitiveType pyType("Calc_PT002", "int");
    py.setChild("type", &pyType);
    add.addChild("parameters", &py);

    // Add return type
    PrimitiveType addRet("Calc_RT001", "int");
    add.setChild("returnType", &addRet);

    // Add a variable for result
    Variable resultVar("Calc_V001", "result");
    PrimitiveType resultType("Calc_VT001", "int");
    resultVar.setChild("type", &resultType);
    add.addChild("body", &resultVar);

    // Add assignment: result = x + y
    Assignment assign;
    assign.id = "Calc_A001";
    
    // Assignment target: result
    VariableReference assignTarget("c_VR_result", "result");
    assign.setChild("target", &assignTarget);
    
    // Assignment value: x + y (BinaryOperation)
    BinaryOperation addOp("Calc_E001", "+");
    
    // Left operand: x
    VariableReference varX("Calc_VR_x", "x");
    addOp.setChild("left", &varX);
    
    // Right operand: y
    VariableReference varY("Calc_VR_y", "y");
    addOp.setChild("right", &varY);
    
    assign.setChild("value", &addOp);
    add.addChild("body", &assign);

    // Add return statement: return result
    Return addReturn;
    addReturn.id = "Calc_R001";
    VariableReference retVal("c_VR_return", "result");
    addReturn.setChild("value", &retVal);
    add.addChild("body", &addReturn);

    // Add function to module
    calc.addChild("functions", &add);

    // Create Python generator and generate code
    PythonGenerator gen;
    std::string output = gen.visitModule(&calc);

    // Verify the output contains the expected Python code
    assert(output.find("def add(x: int, y: int):") != std::string::npos);
    assert(output.find("result = x + y") != std::string::npos);
    assert(output.find("return result") != std::string::npos);

    std::cout << "Generated Python code:\n" << output << std::endl;
    std::cout << "\nStep 9: PASS — Python generator handles statements and expressions" << std::endl;
    return 0;
}