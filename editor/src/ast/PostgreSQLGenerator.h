#pragma once

#include "ProjectionGenerator.h"
#include "SqlNodes.h"
#include "../SemannoAnnotationImpl.h"

#include <sstream>
#include <string>

class PostgreSQLGenerator : public ProjectionGenerator,
                            public SemannoAnnotationImpl<PostgreSQLGenerator> {
public:
    std::string commentPrefix() const { return "-- "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "-- Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "-- Module: " << module->name << "\n\n";
        for (const auto* stmt : module->getChildren("statements")) {
            oss << generate(stmt) << "\n";
        }
        for (const auto* fn : module->getChildren("functions")) {
            oss << generate(fn) << "\n";
        }
        return trimTrailingNewlines(oss.str());
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        for (const auto* anno : function->getChildren("annotations")) {
            oss << generate(anno) << "\n";
        }

        oss << "CREATE FUNCTION " << function->name << "(";
        auto params = function->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")\nRETURNS VOID\nLANGUAGE plpgsql\nAS $$\nBEGIN\n";
        for (const auto* stmt : function->getChildren("body")) {
            oss << "    " << generate(stmt) << "\n";
        }
        oss << "END;\n$$;";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << variable->name;
        auto* init = variable->getChild("initializer");
        if (init) oss << " = " << generate(init);
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        auto* type = parameter->getChild("type");
        return parameter->name + " " + (type ? generate(type) : "TEXT");
    }

    std::string visitAssignment(const Assignment* assignment) override {
        auto* t = assignment->getChild("target");
        auto* v = assignment->getChild("value");
        return (t ? generate(t) : "column") + " = " + (v ? generate(v) : "NULL");
    }

    std::string visitReturn(const Return* ret) override {
        auto* value = ret->getChild("value");
        return value ? "RETURN " + generate(value) + ";" : "RETURN;";
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        auto* l = binOp->getChild("left");
        auto* r = binOp->getChild("right");
        return (l ? generate(l) : "NULL") + " " + binOp->op + " " + (r ? generate(r) : "NULL");
    }

    std::string visitVariableReference(const VariableReference* varRef) override {
        return varRef->variableName;
    }

    std::string visitIntegerLiteral(const IntegerLiteral* lit) override {
        return std::to_string(lit->value);
    }

    std::string visitFloatLiteral(const FloatLiteral* lit) override {
        return lit->value;
    }

    std::string visitStringLiteral(const StringLiteral* lit) override {
        return "'" + lit->value + "'";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "TRUE" : "FALSE";
    }

    std::string visitNullLiteral(const NullLiteral*) override {
        return "NULL";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        auto* cond = stmt->getChild("condition");
        oss << "IF " << (cond ? generate(cond) : "TRUE") << " THEN";
        for (const auto* s : stmt->getChildren("thenBranch")) {
            oss << "\n    " << generate(s);
        }
        auto elseBranch = stmt->getChildren("elseBranch");
        if (!elseBranch.empty()) {
            oss << "\nELSE";
            for (const auto* s : elseBranch) {
                oss << "\n    " << generate(s);
            }
        }
        oss << "\nEND IF;";
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* loop) override {
        std::ostringstream oss;
        auto* cond = loop->getChild("condition");
        oss << "WHILE " << (cond ? generate(cond) : "TRUE") << " LOOP";
        for (const auto* s : loop->getChildren("body")) {
            oss << "\n    " << generate(s);
        }
        oss << "\nEND LOOP;";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;
        auto* iter = loop->getChild("iterable");
        oss << "FOR " << (loop->iteratorName.empty() ? "item" : loop->iteratorName)
            << " IN " << (iter ? generate(iter) : "SELECT 1") << " LOOP";
        for (const auto* s : loop->getChildren("body")) {
            oss << "\n    " << generate(s);
        }
        oss << "\nEND LOOP;";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        auto* expr = stmt->getChild("expression");
        return expr ? generate(expr) + ";" : ";";
    }

    std::string visitUnaryOperation(const UnaryOperation* unOp) override {
        auto* operand = unOp->getChild("operand");
        return unOp->op + (operand ? generate(operand) : "");
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        oss << call->functionName << "(";
        auto args = call->getChildren("arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(args[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::ostringstream oss;
        for (const auto* s : block->getChildren("statements")) {
            oss << generate(s) << "\n";
        }
        return trimTrailingNewlines(oss.str());
    }

    std::string visitListLiteral(const ListLiteral* lit) override {
        std::ostringstream oss;
        oss << "ARRAY[";
        auto elems = lit->getChildren("elements");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elems[i]);
        }
        oss << "]";
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* access) override {
        auto* t = access->getChild("target");
        auto* i = access->getChild("index");
        return (t ? generate(t) : "") + "[" + (i ? generate(i) : "1") + "]";
    }

    std::string visitMemberAccess(const MemberAccess* access) override {
        auto* target = access->getChild("target");
        return (target ? generate(target) : "") + "." + access->memberName;
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        if (type->kind == "int") return "INTEGER";
        if (type->kind == "float" || type->kind == "double") return "DOUBLE PRECISION";
        if (type->kind == "string") return "TEXT";
        if (type->kind == "bool") return "BOOLEAN";
        if (type->kind == "void") return "VOID";
        return type->kind;
    }

    std::string visitListType(const ListType* type) override {
        auto* e = type->getChild("elementType");
        return (e ? generate(e) : "TEXT") + "[]";
    }

    std::string visitSetType(const SetType* type) override {
        auto* e = type->getChild("elementType");
        return (e ? generate(e) : "TEXT") + "[]";
    }

    std::string visitMapType(const MapType*) override {
        return "JSONB";
    }

    std::string visitTupleType(const TupleType*) override {
        return "RECORD";
    }

    std::string visitArrayType(const ArrayType* type) override {
        auto* e = type->getChild("elementType");
        return (e ? generate(e) : "TEXT") + "[]";
    }

    std::string visitOptionalType(const OptionalType* type) override {
        auto* inner = type->getChild("innerType");
        return inner ? generate(inner) : "TEXT";
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    std::string visitDerefStrategy(const DerefStrategy* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitOptimizationLock(const OptimizationLock* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitLangSpecific(const LangSpecific* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitDeallocateAnnotation(const DeallocateAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitLifetimeAnnotation(const LifetimeAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitReclaimAnnotation(const ReclaimAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitOwnerAnnotation(const OwnerAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitAllocateAnnotation(const AllocateAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitHotColdAnnotation(const HotColdAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitInlineAnnotation(const InlineAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitPureAnnotation(const PureAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitConstExprAnnotation(const ConstExprAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }

    std::string visitTableDeclaration(const ASTNode* node) override {
        auto* table = static_cast<const TableDeclaration*>(node);
        std::ostringstream oss;
        oss << "CREATE TABLE " << table->name << " (";

        auto cols = table->getChildren("columns");
        if (cols.empty()) {
            oss << "\n    id SERIAL PRIMARY KEY";
        } else {
            for (size_t i = 0; i < cols.size(); ++i) {
                oss << "\n    " << generate(cols[i]);
                if (i + 1 < cols.size()) oss << ",";
            }
        }
        oss << "\n);";

        for (const auto* idx : table->getChildren("indexes")) {
            oss << "\n" << generate(idx);
        }
        return oss.str();
    }

    std::string visitColumnDefinition(const ASTNode* node) override {
        auto* col = static_cast<const ColumnDefinition*>(node);
        std::ostringstream oss;
        oss << col->name << " " << (col->dataType.empty() ? "TEXT" : col->dataType);
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
        if (from) oss << " FROM " << generate(from);
        for (const auto* join : q->getChildren("joins")) {
            oss << " " << generate(join);
        }
        auto* where = q->getChild("where");
        if (where) oss << " " << generate(where);
        oss << ";";
        return oss.str();
    }

    std::string visitInsertStatement(const ASTNode* node) override {
        auto* ins = static_cast<const InsertStatement*>(node);
        std::ostringstream oss;
        oss << "INSERT INTO " << ins->tableName;

        auto cols = ins->getChildren("columns");
        if (!cols.empty()) {
            oss << " (";
            for (size_t i = 0; i < cols.size(); ++i) {
                if (i > 0) oss << ", ";
                auto* col = static_cast<const ColumnDefinition*>(cols[i]);
                oss << col->name;
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
        oss << "UPDATE " << up->tableName << " SET ";

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
        oss << "DELETE FROM " << del->tableName;
        auto* where = del->getChild("where");
        if (where) oss << " " << generate(where);
        oss << ";";
        return oss.str();
    }

    std::string visitJoinClause(const ASTNode* node) override {
        auto* join = static_cast<const JoinClause*>(node);
        std::ostringstream oss;
        oss << (join->joinType.empty() ? "INNER" : join->joinType)
            << " JOIN " << join->tableName;
        auto* on = join->getChild("on");
        if (on) oss << " ON " << generate(on);
        return oss.str();
    }

    std::string visitWhereClause(const ASTNode* node) override {
        auto* where = static_cast<const WhereClause*>(node);
        return "WHERE " + where->expression;
    }

    std::string visitIndexDefinition(const ASTNode* node) override {
        auto* idx = static_cast<const IndexDefinition*>(node);
        std::ostringstream oss;
        oss << "CREATE ";
        if (idx->unique) oss << "UNIQUE ";
        oss << "INDEX " << idx->name << " ON " << idx->tableName;
        auto cols = idx->getChildren("columns");
        if (!cols.empty()) {
            oss << " (";
            for (size_t i = 0; i < cols.size(); ++i) {
                if (i > 0) oss << ", ";
                auto* col = static_cast<const ColumnDefinition*>(cols[i]);
                oss << col->name;
            }
            oss << ")";
        }
        oss << ";";
        return oss.str();
    }

private:
    static std::string trimTrailingNewlines(std::string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
        return s;
    }
};
