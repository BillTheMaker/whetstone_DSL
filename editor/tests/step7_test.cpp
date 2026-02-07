// Step 7: Schema validation. Verify legal placements accepted, illegal rejected.

#include "../src/ast/ASTNode.h"
#include "../src/ast/Module.h"
#include "../src/ast/Function.h"
#include "../src/ast/Variable.h"
#include "../src/ast/Statement.h"
#include "../src/ast/Expression.h"
#include "../src/ast/Type.h"
#include "../src/ast/Annotation.h"
#include "../src/ast/Schema.h"
#include <cassert>
#include <iostream>

int main() {
    ASTSchema schema;

    // Test 1: Legal placements should be accepted
    assert(schema.isLegalChild("Module", "functions", "Function"));
    assert(schema.isLegalChild("Module", "variables", "Variable"));
    assert(schema.isLegalChild("Function", "parameters", "Parameter"));
    assert(schema.isLegalChild("Function", "returnType", "PrimitiveType"));
    assert(schema.isLegalChild("Function", "body", "Return"));
    assert(schema.isLegalChild("Function", "body", "Assignment"));
    assert(schema.isLegalChild("Assignment", "target", "VariableReference"));
    assert(schema.isLegalChild("Assignment", "value", "BinaryOperation"));
    assert(schema.isLegalChild("BinaryOperation", "left", "VariableReference"));
    assert(schema.isLegalChild("BinaryOperation", "right", "IntegerLiteral"));

    // Test 2: Illegal placements should be rejected
    assert(!schema.isLegalChild("Module", "functions", "Variable"));  // Variable not allowed in functions role
    assert(!schema.isLegalChild("Module", "variables", "Function"));  // Function not allowed in variables role
    assert(!schema.isLegalChild("Function", "parameters", "Variable"));  // Variable not allowed in parameters role
    assert(!schema.isLegalChild("Function", "returnType", "Function"));  // Function not allowed as return type
    assert(!schema.isLegalChild("Assignment", "target", "IntegerLiteral"));  // Literal not allowed as target
    assert(!schema.isLegalChild("Assignment", "value", "Function"));  // Function not allowed as value (directly)
    assert(!schema.isLegalChild("IntegerLiteral", "value", "Variable"));  // IntegerLiteral has no "value" role
    assert(!schema.isLegalChild("NonExistentConcept", "someRole", "SomeChild"));  // Non-existent parent

    // Test 3: Verify cardinality (single vs multi-valued roles)
    assert(schema.isSingleValued("Function", "returnType"));  // Single-valued
    assert(!schema.isSingleValued("Function", "parameters"));  // Multi-valued
    assert(schema.isSingleValued("Assignment", "target"));  // Single-valued
    assert(schema.isSingleValued("Assignment", "value"));  // Single-valued
    assert(!schema.isSingleValued("Function", "body"));  // Multi-valued

    // Test 4: Get allowed concepts
    auto funcParamsAllowed = schema.getAllowedConcepts("Function", "parameters");
    assert(funcParamsAllowed.size() == 1);
    assert(funcParamsAllowed[0] == "Parameter");

    auto funcBodyAllowed = schema.getAllowedConcepts("Function", "body");
    assert(funcBodyAllowed.size() >= 3);  // Should have multiple allowed concepts
    bool hasAssignment = false, hasReturn = false, hasIf = false;
    for (const auto& allowedConcept : funcBodyAllowed) {
        if (allowedConcept == "Assignment") hasAssignment = true;
        if (allowedConcept == "Return") hasReturn = true;
        if (allowedConcept == "IfStatement") hasIf = true;
    }
    assert(hasAssignment && hasReturn && hasIf);

    // Test 5: Edge cases
    assert(schema.getAllowedConcepts("NonExistentConcept", "someRole").empty());
    assert(schema.getAllowedConcepts("Function", "nonExistentRole").empty());

    std::cout << "Step 7: PASS — Schema validation rejects illegal placements, accepts legal ones" << std::endl;
    return 0;
}