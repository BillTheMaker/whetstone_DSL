#pragma once
#include "ASTNode.h"

class Expression : public ASTNode {
public:
    Expression() { conceptType = "Expression"; }
};

class BinaryOperation : public Expression {
public:
    std::string op;
    BinaryOperation() { conceptType = "BinaryOperation"; }
    BinaryOperation(const std::string& id, const std::string& op) : op(op) {
        this->id = id;
        this->conceptType = "BinaryOperation";
    }
    // children: left (1), right (1) via setChild
};

class UnaryOperation : public Expression {
public:
    std::string op;
    UnaryOperation() { conceptType = "UnaryOperation"; }
    // children: operand (1) via setChild("operand", ...)
};

class FunctionCall : public Expression {
public:
    std::string functionName;
    FunctionCall() { conceptType = "FunctionCall"; }
    // children: arguments (0..n) via addChild("arguments", ...)
};

class VariableReference : public Expression {
public:
    std::string variableName;
    VariableReference() { conceptType = "VariableReference"; }
    VariableReference(const std::string& id, const std::string& varName)
        : variableName(varName) {
        this->id = id;
        this->conceptType = "VariableReference";
    }
};

class IntegerLiteral : public Expression {
public:
    int value = 0;
    IntegerLiteral() { conceptType = "IntegerLiteral"; }
    IntegerLiteral(const std::string& id, int val) : value(val) {
        this->id = id;
        this->conceptType = "IntegerLiteral";
    }
};

class FloatLiteral : public Expression {
public:
    std::string value;
    FloatLiteral() { conceptType = "FloatLiteral"; }
    FloatLiteral(const std::string& id, const std::string& val) : value(val) {
        this->id = id;
        this->conceptType = "FloatLiteral";
    }
};

class StringLiteral : public Expression {
public:
    std::string value;
    StringLiteral() { conceptType = "StringLiteral"; }
    StringLiteral(const std::string& id, const std::string& val) : value(val) {
        this->id = id;
        this->conceptType = "StringLiteral";
    }
};

class BooleanLiteral : public Expression {
public:
    bool value = false;
    BooleanLiteral() { conceptType = "BooleanLiteral"; }
    BooleanLiteral(const std::string& id, bool val) : value(val) {
        this->id = id;
        this->conceptType = "BooleanLiteral";
    }
};

class NullLiteral : public Expression {
public:
    NullLiteral() { conceptType = "NullLiteral"; }
};

class ListLiteral : public Expression {
public:
    ListLiteral() { conceptType = "ListLiteral"; }
    // children: elements (0..n) via addChild("elements", ...)
};

class IndexAccess : public Expression {
public:
    IndexAccess() { conceptType = "IndexAccess"; }
    // children: target (1), index (1) via setChild
};

class MemberAccess : public Expression {
public:
    std::string memberName;
    MemberAccess() { conceptType = "MemberAccess"; }
    // children: target (1) via setChild("target", ...)
};
