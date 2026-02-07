#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

struct ChildRule {
    std::vector<std::string> allowedConcepts;
    bool isSingleValued = false;  // false = multi-valued (list), true = single-valued (pointer)
};

class ASTSchema {
public:
    ASTSchema() {
        initializeSchema();
    }

    // Check if a child concept is allowed under a parent concept in a specific role
    bool isLegalChild(const std::string& parentConcept, const std::string& role, const std::string& childConcept) const {
        auto parentIt = schema_.find(parentConcept);
        if (parentIt == schema_.end()) {
            return false;  // Parent concept not in schema
        }

        auto roleIt = parentIt->second.find(role);
        if (roleIt == parentIt->second.end()) {
            return false;  // Role not defined for this parent
        }

        const ChildRule& rule = roleIt->second;
        
        // Check if the child concept is in the allowed list
        for (const auto& allowedConcept : rule.allowedConcepts) {
            if (allowedConcept == childConcept) {
                return true;
            }
        }
        
        return false;
    }

    // Get the cardinality of a role (single vs multi-valued)
    bool isSingleValued(const std::string& parentConcept, const std::string& role) const {
        auto parentIt = schema_.find(parentConcept);
        if (parentIt == schema_.end()) {
            return false;  // Parent concept not in schema
        }

        auto roleIt = parentIt->second.find(role);
        if (roleIt == parentIt->second.end()) {
            return false;  // Role not defined for this parent
        }

        return roleIt->second.isSingleValued;
    }

    // Get allowed child concepts for a role
    std::vector<std::string> getAllowedConcepts(const std::string& parentConcept, const std::string& role) const {
        std::vector<std::string> result;
        auto parentIt = schema_.find(parentConcept);
        if (parentIt == schema_.end()) {
            return result;  // Empty if parent not found
        }

        auto roleIt = parentIt->second.find(role);
        if (roleIt == parentIt->second.end()) {
            return result;  // Empty if role not found
        }

        return roleIt->second.allowedConcepts;
    }

private:
    std::map<std::string, std::map<std::string, ChildRule>> schema_;

    void initializeSchema() {
        // Module concept rules
        addRule("Module", "functions", {"Function"}, false);  // multi-valued
        addRule("Module", "variables", {"Variable"}, false);  // multi-valued
        addRule("Module", "targetLanguage", {}, true);  // property, not used for AST nodes
        addRule("Module", "annotations", {"DerefStrategy", "OptimizationLock", "LangSpecific"}, false);  // multi-valued

        // Function concept rules
        addRule("Function", "parameters", {"Parameter"}, false);  // multi-valued
        addRule("Function", "returnType", {"PrimitiveType", "ListType", "SetType", "MapType", "TupleType", "ArrayType", "OptionalType", "CustomType"}, true);  // single-valued
        addRule("Function", "body", {"Block", "Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued
        addRule("Function", "annotations", {"DerefStrategy", "OptimizationLock", "LangSpecific"}, false);  // multi-valued

        // Parameter concept rules
        addRule("Parameter", "type", {"PrimitiveType", "ListType", "SetType", "MapType", "TupleType", "ArrayType", "OptionalType", "CustomType"}, true);  // single-valued
        addRule("Parameter", "defaultValue", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference"}, true);  // single-valued

        // Variable concept rules
        addRule("Variable", "type", {"PrimitiveType", "ListType", "SetType", "MapType", "TupleType", "ArrayType", "OptionalType", "CustomType"}, true);  // single-valued
        addRule("Variable", "initializer", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference"}, true);  // single-valued
        addRule("Variable", "annotations", {"DerefStrategy", "OptimizationLock", "LangSpecific"}, false);  // multi-valued

        // Block concept rules
        addRule("Block", "statements", {"Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued

        // Assignment concept rules
        addRule("Assignment", "target", {"VariableReference"}, true);  // single-valued
        addRule("Assignment", "value", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, true);  // single-valued

        // IfStatement concept rules
        addRule("IfStatement", "condition", {"BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral"}, true);  // single-valued
        addRule("IfStatement", "thenBranch", {"Block", "Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued
        addRule("IfStatement", "elseBranch", {"Block", "Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued

        // WhileLoop concept rules
        addRule("WhileLoop", "condition", {"BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral"}, true);  // single-valued
        addRule("WhileLoop", "body", {"Block", "Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued

        // ForLoop concept rules
        addRule("ForLoop", "iteratorName", {}, true);  // property, not used for AST nodes
        addRule("ForLoop", "iterable", {"VariableReference", "FunctionCall", "BinaryOperation", "UnaryOperation", "ListLiteral"}, true);  // single-valued
        addRule("ForLoop", "body", {"Block", "Assignment", "IfStatement", "WhileLoop", "ForLoop", "Return", "ExpressionStatement"}, false);  // multi-valued

        // Return concept rules
        addRule("Return", "value", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, true);  // single-valued

        // ExpressionStatement concept rules
        addRule("ExpressionStatement", "expression", {"BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "IndexAccess", "MemberAccess"}, true);  // single-valued

        // BinaryOperation concept rules
        addRule("BinaryOperation", "left", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, true);  // single-valued
        addRule("BinaryOperation", "right", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, true);  // single-valued

        // UnaryOperation concept rules
        addRule("UnaryOperation", "operand", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, true);  // single-valued

        // FunctionCall concept rules
        addRule("FunctionCall", "functionName", {}, true);  // property, not used for AST nodes
        addRule("FunctionCall", "arguments", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, false);  // multi-valued

        // VariableReference concept rules
        addRule("VariableReference", "variableName", {}, true);  // property, not used for AST nodes

        // ListLiteral concept rules
        addRule("ListLiteral", "elements", {"IntegerLiteral", "FloatLiteral", "StringLiteral", "BooleanLiteral", "NullLiteral", "ListLiteral", "BinaryOperation", "UnaryOperation", "FunctionCall", "VariableReference", "IndexAccess", "MemberAccess"}, false);  // multi-valued

        // IndexAccess concept rules
        addRule("IndexAccess", "target", {"VariableReference", "FunctionCall", "IndexAccess", "MemberAccess"}, true);  // single-valued
        addRule("IndexAccess", "index", {"IntegerLiteral", "VariableReference", "BinaryOperation", "FunctionCall"}, true);  // single-valued

        // MemberAccess concept rules
        addRule("MemberAccess", "target", {"VariableReference", "FunctionCall", "IndexAccess", "MemberAccess"}, true);  // single-valued
        addRule("MemberAccess", "memberName", {}, true);  // property, not used for AST nodes

        // Type concepts - no child rules needed as they're leaf nodes for types

        // Annotation concepts - no child rules needed as they're leaf nodes for annotations
    }

    void addRule(const std::string& parentConcept, const std::string& role, const std::vector<std::string>& allowedConcepts, bool isSingleValued) {
        ChildRule rule;
        rule.allowedConcepts = allowedConcepts;
        rule.isSingleValued = isSingleValued;
        schema_[parentConcept][role] = rule;
    }
};