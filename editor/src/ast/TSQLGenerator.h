#pragma once

#include "PostgreSQLGenerator.h"
#include "SqlNodes.h"

#include <sstream>
#include <string>

class TSQLGenerator : public PostgreSQLGenerator {
public:
    std::string commentPrefix() const { return "-- "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "-- Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "-- Module: " << module->name << "\n\n";
        for (const auto* v : module->getChildren("variables")) {
            oss << generate(v) << "\n";
        }
        for (const auto* stmt : module->getChildren("statements")) {
            oss << generate(stmt) << "\n";
        }
        for (const auto* fn : module->getChildren("functions")) {
            oss << generate(fn) << "\n";
        }
        std::string out = oss.str();
        while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) out.pop_back();
        return out;
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        for (const auto* anno : function->getChildren("annotations")) {
            oss << generate(anno) << "\n";
        }
        oss << "CREATE PROCEDURE " << function->name << "\nAS\nBEGIN\n";
        for (const auto* stmt : function->getChildren("body")) {
            oss << "    " << generate(stmt) << "\n";
        }
        oss << "END;";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "DECLARE " << variable->name;
        auto* type = variable->getChild("type");
        if (type) oss << " " << generate(type);
        auto* init = variable->getChild("initializer");
        if (init) oss << " = " << generate(init);
        oss << ";";
        return oss.str();
    }

    std::string visitAssignment(const Assignment* assignment) override {
        auto* t = assignment->getChild("target");
        auto* v = assignment->getChild("value");
        return "SET " + std::string(t ? generate(t) : "@value") +
               " = " + std::string(v ? generate(v) : "NULL") + ";";
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        auto* expr = stmt->getChild("expression");
        return expr ? generate(expr) : "";
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        if (call->functionName == "PRINT") {
            oss << "PRINT ";
            auto args = call->getChildren("arguments");
            if (!args.empty()) oss << generate(args.front());
            oss << ";";
            return oss.str();
        }

        oss << call->functionName << "(";
        auto args = call->getChildren("arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(args[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        if (type->kind == "int" || type->kind == "INTEGER") return "INT";
        if (type->kind == "float" || type->kind == "double") return "FLOAT";
        if (type->kind == "string" || type->kind == "TEXT") return "NVARCHAR(MAX)";
        if (type->kind == "bool") return "BIT";
        if (type->kind == "void") return "VOID";
        return type->kind;
    }

    std::string visitTableDeclaration(const ASTNode* node) override {
        auto* table = static_cast<const TableDeclaration*>(node);
        std::ostringstream oss;
        oss << "CREATE TABLE " << table->name << " (";

        auto cols = table->getChildren("columns");
        if (cols.empty()) {
            oss << "\n    id INT IDENTITY(1,1) PRIMARY KEY";
        } else {
            for (size_t i = 0; i < cols.size(); ++i) {
                oss << "\n    " << generate(cols[i]);
                if (i + 1 < cols.size()) oss << ",";
            }
        }
        oss << "\n);";
        return oss.str();
    }

    std::string visitSelectQuery(const ASTNode* node) override {
        auto* q = static_cast<const SelectQuery*>(node);
        std::ostringstream oss;
        oss << "SELECT ";
        auto* topNode = q->getChild("top");
        if (topNode) {
            auto* top = static_cast<const IntegerLiteral*>(topNode);
            oss << "TOP " << top->value << " ";
        }
        if (q->distinct) oss << "DISTINCT ";

        auto cols = q->getChildren("columns");
        if (cols.empty()) {
            oss << "*";
        } else {
            for (size_t i = 0; i < cols.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(cols[i]);
            }
        }

        auto* from = q->getChild("from");
        if (from && from->conceptType == "TableDeclaration") {
            oss << " FROM " << static_cast<const TableDeclaration*>(from)->name;
        } else if (from) {
            oss << " FROM " << generate(from);
        }
        for (const auto* join : q->getChildren("joins")) {
            oss << " " << generate(join);
        }
        auto* where = q->getChild("where");
        if (where) oss << " " << generate(where);
        oss << ";";
        return oss.str();
    }

    std::string visitColumnDefinition(const ASTNode* node) override {
        auto* col = static_cast<const ColumnDefinition*>(node);
        std::ostringstream oss;
        oss << col->name << " " << (col->dataType.empty() ? "NVARCHAR(MAX)" : col->dataType);
        if (!col->nullable) oss << " NOT NULL";
        if (!col->defaultValue.empty()) oss << " DEFAULT " << col->defaultValue;
        return oss.str();
    }
};
