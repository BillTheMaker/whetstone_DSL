#pragma once

#include "PostgreSQLGenerator.h"
#include "SqlNodes.h"

#include <sstream>
#include <string>

class MySQLGenerator : public PostgreSQLGenerator {
public:
    std::string commentPrefix() const { return "-- "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "-- Unknown concept: ");
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        if (type->kind == "int") return "INT";
        if (type->kind == "float" || type->kind == "double") return "DOUBLE";
        if (type->kind == "string") return "VARCHAR(255)";
        if (type->kind == "bool") return "TINYINT(1)";
        if (type->kind == "void") return "VOID";
        return type->kind;
    }

    std::string visitTableDeclaration(const ASTNode* node) override {
        auto* table = static_cast<const TableDeclaration*>(node);
        std::ostringstream oss;
        oss << "CREATE TABLE " << quoteIdentifier(table->name) << " (";

        auto cols = table->getChildren("columns");
        if (cols.empty()) {
            oss << "\n    `id` INT AUTO_INCREMENT PRIMARY KEY";
        } else {
            for (size_t i = 0; i < cols.size(); ++i) {
                oss << "\n    " << generate(cols[i]);
                if (i + 1 < cols.size()) oss << ",";
            }
        }
        oss << "\n)";

        auto* engine = table->getChild("engine");
        if (engine && engine->conceptType == "CustomType") {
            oss << " ENGINE=" << static_cast<const CustomType*>(engine)->typeName;
        }
        oss << ";";
        return oss.str();
    }

    std::string visitColumnDefinition(const ASTNode* node) override {
        auto* col = static_cast<const ColumnDefinition*>(node);
        std::ostringstream oss;
        oss << quoteIdentifier(col->name)
            << " " << (col->dataType.empty() ? "VARCHAR(255)" : col->dataType);
        if (!col->nullable) oss << " NOT NULL";
        if (!col->defaultValue.empty()) oss << " DEFAULT " << col->defaultValue;
        return oss.str();
    }

    std::string visitSelectQuery(const ASTNode* node) override {
        auto* q = static_cast<const SelectQuery*>(node);
        std::ostringstream oss;
        oss << "SELECT ";
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
            auto* tbl = static_cast<const TableDeclaration*>(from);
            oss << " FROM " << quoteIdentifier(tbl->name);
        } else if (from) {
            oss << " FROM " << generate(from);
        }
        for (const auto* join : q->getChildren("joins")) {
            oss << " " << generate(join);
        }
        auto* where = q->getChild("where");
        if (where) oss << " " << generate(where);

        auto* limit = q->getChild("limit");
        if (limit && limit->conceptType == "IntegerLiteral") {
            oss << " LIMIT " << static_cast<const IntegerLiteral*>(limit)->value;
        }
        oss << ";";
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        if (call->functionName == "GROUP_CONCAT") {
            return "GROUP_CONCAT(*)";
        }
        return PostgreSQLGenerator::visitFunctionCall(call);
    }

    std::string visitInsertStatement(const ASTNode* node) override {
        auto* ins = static_cast<const InsertStatement*>(node);
        std::ostringstream oss;
        oss << "INSERT INTO " << quoteIdentifier(ins->tableName);

        auto cols = ins->getChildren("columns");
        if (!cols.empty()) {
            oss << " (";
            for (size_t i = 0; i < cols.size(); ++i) {
                if (i > 0) oss << ", ";
                auto* col = static_cast<const ColumnDefinition*>(cols[i]);
                oss << quoteIdentifier(col->name);
            }
            oss << ")";
        }

        auto values = ins->getChildren("values");
        if (!values.empty()) {
            oss << " VALUES (";
            for (size_t i = 0; i < values.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(values[i]);
            }
            oss << ")";
        }
        oss << ";";
        return oss.str();
    }

    std::string visitUpdateStatement(const ASTNode* node) override {
        auto* up = static_cast<const UpdateStatement*>(node);
        std::ostringstream oss;
        oss << "UPDATE " << quoteIdentifier(up->tableName) << " SET ";
        auto sets = up->getChildren("set");
        if (sets.empty()) {
            oss << "/* STUB: set clause missing */";
        } else {
            for (size_t i = 0; i < sets.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(sets[i]);
            }
        }
        auto* where = up->getChild("where");
        if (where) oss << " " << generate(where);
        oss << ";";
        return oss.str();
    }

    std::string visitDeleteStatement(const ASTNode* node) override {
        auto* del = static_cast<const DeleteStatement*>(node);
        std::ostringstream oss;
        oss << "DELETE FROM " << quoteIdentifier(del->tableName);
        auto* where = del->getChild("where");
        if (where) oss << " " << generate(where);
        oss << ";";
        return oss.str();
    }

private:
    static std::string quoteIdentifier(const std::string& ident) {
        if (ident.empty()) return "``";
        if (ident.front() == '`' && ident.back() == '`') return ident;

        std::ostringstream oss;
        std::stringstream ss(ident);
        std::string part;
        bool first = true;
        while (std::getline(ss, part, '.')) {
            if (!first) oss << ".";
            oss << "`" << part << "`";
            first = false;
        }
        return oss.str();
    }
};
