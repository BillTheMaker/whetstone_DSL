#pragma once
#include "ASTNode.h"
#include "Function.h"
#include "Expression.h"
#include <string>
#include <vector>

// AsyncFunction — an async function (extends Function)
class AsyncFunction : public Function {
public:
    bool isAsync = true;

    AsyncFunction() { conceptType = "AsyncFunction"; isAsync = true; }
    AsyncFunction(const std::string& id_, const std::string& name_)
        : Function(id_, name_) {
        this->conceptType = "AsyncFunction";
        isAsync = true;
    }
};

// AwaitExpression — await <expression>
class AwaitExpression : public Expression {
public:
    AwaitExpression() { conceptType = "AwaitExpression"; }
    AwaitExpression(const std::string& id_) {
        this->id = id_;
        this->conceptType = "AwaitExpression";
    }
    // Children: "expression" (the awaited expression)
};

// LambdaExpression — anonymous function / closure
class LambdaExpression : public Expression {
public:
    std::vector<std::string> captureList;  // captured variables

    LambdaExpression() { conceptType = "LambdaExpression"; }
    LambdaExpression(const std::string& id_) {
        this->id = id_;
        this->conceptType = "LambdaExpression";
    }
    // Children: "parameters" (list of Parameter), "body" (list of Statement)
};

// DecoratorAnnotation — @decorator(args) on a function/class
class DecoratorAnnotation : public ASTNode {
public:
    std::string name;

    DecoratorAnnotation() { conceptType = "DecoratorAnnotation"; }
    DecoratorAnnotation(const std::string& id_, const std::string& name_)
        : name(name_) {
        this->id = id_;
        this->conceptType = "DecoratorAnnotation";
    }
    // Children: "arguments" (list of Expression)
};
