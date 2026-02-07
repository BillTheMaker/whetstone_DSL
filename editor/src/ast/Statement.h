#pragma once
#include "ASTNode.h"

class Statement : public ASTNode {
public:
    Statement() { conceptType = "Statement"; }
};

class Block : public Statement {
public:
    Block() { conceptType = "Block"; }
    // children: statements (0..n) via addChild("statements", ...)
};

class Assignment : public Statement {
public:
    Assignment() { conceptType = "Assignment"; }
    // children: target (1) via setChild("target", ...)
    // children: value  (1) via setChild("value", ...)
};

class IfStatement : public Statement {
public:
    IfStatement() { conceptType = "IfStatement"; }
    // children: condition  (1)   via setChild("condition", ...)
    // children: thenBranch (0..n) via addChild("thenBranch", ...)
    // children: elseBranch (0..n) via addChild("elseBranch", ...)
};

class WhileLoop : public Statement {
public:
    WhileLoop() { conceptType = "WhileLoop"; }
    // children: condition (1)   via setChild("condition", ...)
    // children: body      (0..n) via addChild("body", ...)
};

class ForLoop : public Statement {
public:
    std::string iteratorName;
    ForLoop() { conceptType = "ForLoop"; }
    // children: iterable (1)   via setChild("iterable", ...)
    // children: body     (0..n) via addChild("body", ...)
};

class Return : public Statement {
public:
    Return() { conceptType = "Return"; }
    // children: value (0..1) via setChild("value", ...)
};

class ExpressionStatement : public Statement {
public:
    ExpressionStatement() { conceptType = "ExpressionStatement"; }
    // children: expression (1) via setChild("expression", ...)
};
