#pragma once

#include "ASTNode.h"
#include <string>

// SQL-specific AST nodes (Step 410)

class TableDeclaration : public ASTNode {
public:
    std::string name;
    std::string schema;
    TableDeclaration() { conceptType = "TableDeclaration"; }
    TableDeclaration(const std::string& id_, const std::string& n, const std::string& s = "")
        : name(n), schema(s) {
        id = id_;
        conceptType = "TableDeclaration";
    }
    // children: "columns" (ColumnDefinition), "indexes" (IndexDefinition)
};

class ColumnDefinition : public ASTNode {
public:
    std::string name;
    std::string dataType;
    bool nullable = true;
    std::string defaultValue;
    ColumnDefinition() { conceptType = "ColumnDefinition"; }
    ColumnDefinition(const std::string& id_, const std::string& n,
                     const std::string& dt, bool isNullable = true)
        : name(n), dataType(dt), nullable(isNullable) {
        id = id_;
        conceptType = "ColumnDefinition";
    }
};

class SelectQuery : public ASTNode {
public:
    bool distinct = false;
    SelectQuery() { conceptType = "SelectQuery"; }
    // children: "columns", "from", "joins", "where"
};

class InsertStatement : public ASTNode {
public:
    std::string tableName;
    InsertStatement() { conceptType = "InsertStatement"; }
    InsertStatement(const std::string& id_, const std::string& table) : tableName(table) {
        id = id_;
        conceptType = "InsertStatement";
    }
    // children: "columns", "values"
};

class UpdateStatement : public ASTNode {
public:
    std::string tableName;
    UpdateStatement() { conceptType = "UpdateStatement"; }
    UpdateStatement(const std::string& id_, const std::string& table) : tableName(table) {
        id = id_;
        conceptType = "UpdateStatement";
    }
    // children: "set", "where"
};

class DeleteStatement : public ASTNode {
public:
    std::string tableName;
    DeleteStatement() { conceptType = "DeleteStatement"; }
    DeleteStatement(const std::string& id_, const std::string& table) : tableName(table) {
        id = id_;
        conceptType = "DeleteStatement";
    }
    // children: "where"
};

class JoinClause : public ASTNode {
public:
    std::string joinType;   // INNER/LEFT/RIGHT/FULL
    std::string tableName;
    JoinClause() { conceptType = "JoinClause"; }
    JoinClause(const std::string& id_, const std::string& jt, const std::string& tn)
        : joinType(jt), tableName(tn) {
        id = id_;
        conceptType = "JoinClause";
    }
    // children: "on"
};

class WhereClause : public ASTNode {
public:
    std::string expression;
    WhereClause() { conceptType = "WhereClause"; }
    WhereClause(const std::string& id_, const std::string& expr) : expression(expr) {
        id = id_;
        conceptType = "WhereClause";
    }
};

class IndexDefinition : public ASTNode {
public:
    std::string name;
    std::string tableName;
    bool unique = false;
    IndexDefinition() { conceptType = "IndexDefinition"; }
    IndexDefinition(const std::string& id_, const std::string& n,
                    const std::string& table, bool isUnique = false)
        : name(n), tableName(table), unique(isUnique) {
        id = id_;
        conceptType = "IndexDefinition";
    }
    // children: "columns"
};
