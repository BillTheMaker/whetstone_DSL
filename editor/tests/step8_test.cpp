// Step 8: Python generator - Module header and Function signatures only.
//
// PythonGenerator generates Module header and Function signatures only.
// Test verifies Calculator AST → Python output has correct `def add(x, y):` signature.

#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Parameter.h"
#include "../src/ast/Type.h"
#include "../src/ast/Generator.h"
#include <cassert>
#include <iostream>
#include <sstream>

int main() {
    // Build a simple Calculator module with one function
    Module calc("Calc_M001", "Calculator", "python");

    // Add a function: def add(x, y):
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

    // Add function to module
    calc.addChild("functions", &add);

    // Create Python generator and generate code
    PythonGenerator gen;
    std::string output = gen.visitModule(&calc);

    // Verify the output contains the correct function signature
    // The function should be present with parameters
    assert(output.find("def add") != std::string::npos);  // Function exists
    assert(output.find("Calculator") != std::string::npos);  // Module header

    // Verify parameter types are included (the function should have typed parameters)
    assert(output.find("x: int") != std::string::npos);
    assert(output.find("y: int") != std::string::npos);
    
    // The function should have the basic signature structure
    assert(output.find("def add(") != std::string::npos);

    std::cout << "Generated Python code:\n" << output << std::endl;
    std::cout << "\nStep 8: PASS — Python generator creates correct function signatures" << std::endl;
    return 0;
}