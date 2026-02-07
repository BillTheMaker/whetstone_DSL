// Step 11: Python generator - Annotation output.
//
// DerefStrategy → `# @deref(strategy)` comment in Python
// OptimizationLock → `# @lock(...)` comment
// Test: SimpleFunctionExample with @deref(batched) → comment appears in output.

#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Parameter.h"
#include "../src/ast/Statement.h"
#include "../src/ast/Expression.h"
#include "../src/ast/Type.h"
#include "../src/ast/Annotation.h"
#include "../src/ast/Generator.h"
#include <cassert>
#include <iostream>
#include <sstream>

int main() {
    // Build a SimpleFunctionExample module with deref annotation
    Module simple("SFE_M001", "SimpleFunctionExample", "python");

    // Add a function: def calculate_sum(a, b) with deref annotation
    Function calcSum("SFE_F001", "calculate_sum");
    
    // Add parameters a and b
    Parameter pa("SFE_P001", "a");
    PrimitiveType paType("SFE_PT001", "int");
    pa.setChild("type", &paType);
    calcSum.addChild("parameters", &pa);

    Parameter pb("SFE_P002", "b");
    PrimitiveType pbType("SFE_PT002", "int");
    pb.setChild("type", &pbType);
    calcSum.addChild("parameters", &pb);

    // Add return type
    PrimitiveType retType("SFE_RT001", "int");
    calcSum.setChild("returnType", &retType);

    // Add deref strategy annotation: @deref(batched)
    DerefStrategy derefStrategy;
    derefStrategy.id = "SFE_DR001";
    derefStrategy.strategy = "batched";
    calcSum.addChild("annotations", &derefStrategy);

    // Add a simple assignment: result = a + b
    Assignment assign;
    assign.id = "SFE_A001";
    
    // Assignment target: result
    VariableReference assignTarget("SFE_VR001", "result");
    assign.setChild("target", &assignTarget);
    
    // Assignment value: a + b (BinaryOperation)
    BinaryOperation addOp("SFE_BO001", "+");
    
    // Left operand: a
    VariableReference varA("SFE_VR_a", "a");
    addOp.setChild("left", &varA);
    
    // Right operand: b
    VariableReference varB("SFE_VR_b", "b");
    addOp.setChild("right", &varB);
    
    assign.setChild("value", &addOp);
    calcSum.addChild("body", &assign);

    // Add return statement: return result
    Return retStmt;
    retStmt.id = "SFE_R001";
    VariableReference retVal("SFE_VR_return", "result");
    retStmt.setChild("value", &retVal);
    calcSum.addChild("body", &retStmt);

    // Add function to module
    simple.addChild("functions", &calcSum);

    // Create Python generator and generate code
    PythonGenerator gen;
    std::string output = gen.visitModule(&simple);

    std::cout << "Generated Python code:\n" << output << std::endl;

    // Verify the output contains the expected Python code with annotation comment
    assert(output.find("def calculate_sum(a: int, b: int):") != std::string::npos);
    assert(output.find("result = a + b") != std::string::npos);
    assert(output.find("return result") != std::string::npos);
    // Check that the deref annotation appears as a comment
    assert(output.find("# @deref(batched)") != std::string::npos);

    std::cout << "\nStep 11: PASS — Python generator handles annotation output" << std::endl;
    return 0;
}