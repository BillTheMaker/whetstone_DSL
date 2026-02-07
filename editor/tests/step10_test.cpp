// Step 10: Python generator - Remaining concepts.
//
// Add Literals, control flow (If/While/For), types, FunctionCall, IndexAccess, MemberAccess.
// Test: ConditionalExample AST → correct Python with if/else.

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
    // Build a ConditionalExample module with if/else logic
    Module cond("CE_M001", "ConditionalExample", "python");

    // Add a function: def check_status(status_code):
    Function checkStatus("CE_F001", "check_status");
    
    // Add parameter status_code
    Parameter param("CE_P001", "status_code");
    PrimitiveType paramType("CE_PT001", "int");
    param.setChild("type", &paramType);
    checkStatus.addChild("parameters", &param);

    // Add return type
    PrimitiveType retType("CE_RT001", "string");
    checkStatus.setChild("returnType", &retType);

    // Add if statement: if status_code == 200: return "OK", else: return "Error"
    IfStatement ifStmt;
    ifStmt.id = "CE_IF001";
    
    // Condition: status_code == 200 (BinaryOperation)
    BinaryOperation eqOp("CE_BO001", "==");
    
    // Left: status_code (VariableReference)
    VariableReference statusVar("CE_VR001", "status_code");
    eqOp.setChild("left", &statusVar);
    
    // Right: 200 (IntegerLiteral)
    IntegerLiteral intLit("CE_IL001", 200);
    eqOp.setChild("right", &intLit);
    
    ifStmt.setChild("condition", &eqOp);
    
    // Then branch: return "OK"
    Return okReturn;
    okReturn.id = "CE_R001";
    StringLiteral okLit("CE_SL001", "OK");
    okReturn.setChild("value", &okLit);
    ifStmt.addChild("thenBranch", &okReturn);
    
    // Else branch: return "Error"
    Return errorReturn;
    errorReturn.id = "CE_R002";
    StringLiteral errorLit("CE_SL002", "Error");
    errorReturn.setChild("value", &errorLit);
    ifStmt.addChild("elseBranch", &errorReturn);
    
    checkStatus.addChild("body", &ifStmt);

    // Add function to module
    cond.addChild("functions", &checkStatus);

    // Create Python generator and generate code
    PythonGenerator gen;
    std::string output = gen.visitModule(&cond);

    // Verify the output contains the expected Python code
    assert(output.find("def check_status(status_code: int):") != std::string::npos);
    assert(output.find("if status_code == 200:") != std::string::npos);
    assert(output.find("return \"OK\"") != std::string::npos);
    assert(output.find("else:") != std::string::npos);
    assert(output.find("return \"Error\"") != std::string::npos);

    std::cout << "Generated Python code:\n" << output << std::endl;
    std::cout << "\nStep 10: PASS — Python generator handles remaining concepts" << std::endl;
    return 0;
}