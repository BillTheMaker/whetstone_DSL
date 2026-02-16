#pragma once

#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/ClassDeclaration.h"
#include "ast/Variable.h"
#include "ast/SqlNodes.h"
#include "ast/Annotation.h"
#include "ast/Expression.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_set>
#include <vector>

class SqlAnnotationMapper {
public:
    struct InferredAnnotation {
        std::string nodeId;
        std::string annotationType;
        std::string key;
        std::string value;
        std::string reason;
        double confidence = 0.0;
    };

    std::vector<InferredAnnotation> inferForModule(const Module* module) const {
        std::vector<InferredAnnotation> out;
        if (!module) return out;

        for (const auto* stmt : module->getChildren("statements")) {
            inferForNode(stmt, out);
        }
        return out;
    }

    std::vector<InferredAnnotation> inferForSqlText(
            const std::string& sqlText,
            const std::string& nodeId = "sql_text") const {
        std::vector<InferredAnnotation> out;
        std::string lower = toLower(sqlText);

        if (containsPhrase(lower, "drop table")) {
            out.push_back({nodeId, "RiskAnnotation", "level", "high",
                           "DROP TABLE is destructive and high-risk", 0.95});
        }
        if (containsPhrase(lower, "delete from") && !containsPhrase(lower, " where ")) {
            out.push_back({nodeId, "RiskAnnotation", "level", "high",
                           "DELETE without WHERE can remove all rows", 0.95});
        }
        if (containsPhrase(lower, " limit ") || containsPhrase(lower, " top ")) {
            out.push_back({nodeId, "BoundsCheckAnnotation", "enabled", "true",
                           "LIMIT/TOP bounds result set size", 0.85});
        }
        return out;
    }

    std::vector<InferredAnnotation> inferCrossStackOrmMapping(
            const ClassDeclaration* ormClass,
            const TableDeclaration* table) const {
        std::vector<InferredAnnotation> out;
        if (!ormClass || !table) return out;

        auto classFields = collectClassFields(ormClass);
        auto tableColumns = collectTableColumns(table);
        const int matched = countIntersection(classFields, tableColumns);
        const int denom = std::max(1, std::max((int)classFields.size(), (int)tableColumns.size()));

        out.push_back({ormClass->id, "ContractAnnotation", "preconditions",
                       "sql table " + table->name + " exists", "ORM class depends on backing SQL table", 0.88});
        out.push_back({table->id, "ContractAnnotation", "preconditions",
                       "orm class " + ormClass->name + " stays schema-compatible",
                       "SQL table participates in ORM mapping contract", 0.84});

        if (matched * 2 < denom) {
            out.push_back({ormClass->id, "RiskAnnotation", "level", "medium",
                           "ORM fields diverge from SQL columns; migration risk", 0.82});
        }

        return out;
    }

    void applyInferredAnnotations(
            ASTNode* root,
            const std::vector<InferredAnnotation>& inferred) const {
        if (!root) return;
        for (const auto& inf : inferred) {
            ASTNode* node = findNodeById(root, inf.nodeId);
            if (!node) continue;
            Annotation* anno = createAnnotationNode(inf);
            if (!anno) continue;
            if (anno->id.empty()) anno->id = "sql_inferred_" + inf.nodeId + "_" + inf.annotationType;
            node->addChild("annotations", anno);
        }
    }

private:
    void inferForNode(const ASTNode* node, std::vector<InferredAnnotation>& out) const {
        if (!node) return;

        if (node->conceptType == "DeleteStatement") {
            auto* del = static_cast<const DeleteStatement*>(node);
            if (del->getChild("where") == nullptr) {
                out.push_back({node->id, "RiskAnnotation", "level", "high",
                               "DELETE without WHERE can remove all rows", 0.95});
            }
        } else if (node->conceptType == "UpdateStatement") {
            auto* up = static_cast<const UpdateStatement*>(node);
            if (up->getChild("where") == nullptr) {
                out.push_back({node->id, "RiskAnnotation", "level", "medium",
                               "UPDATE without WHERE can overwrite all rows", 0.86});
            }
        } else if (node->conceptType == "SelectQuery") {
            auto* q = static_cast<const SelectQuery*>(node);
            const int joinDepth = (int)q->getChildren("joins").size();
            const int subqueries = countSubqueryHints(q);

            out.push_back({node->id, "ComplexityAnnotation", "timeComplexity",
                           complexityFromQuery(joinDepth, subqueries),
                           "JOIN depth and subquery nesting estimated from query structure",
                           confidenceFromComplexity(joinDepth, subqueries)});

            if (hasBoundsClause(q)) {
                out.push_back({node->id, "BoundsCheckAnnotation", "enabled", "true",
                               "LIMIT/TOP bounds result set size", 0.86});
            }

            if (dependsOnIndexes(q)) {
                out.push_back({node->id, "ContractAnnotation", "preconditions",
                               "index exists on predicate/join columns",
                               "Query shape indicates index dependency for expected performance",
                               0.78});
            }
        }
    }

    static bool hasBoundsClause(const SelectQuery* q) {
        return q->getChild("limit") != nullptr || q->getChild("top") != nullptr;
    }

    static bool dependsOnIndexes(const SelectQuery* q) {
        return !q->getChildren("joins").empty() || q->getChild("where") != nullptr;
    }

    static int countSubqueryHints(const SelectQuery* q) {
        int hints = 0;
        auto* where = q->getChild("where");
        if (where && where->conceptType == "WhereClause") {
            const auto* w = static_cast<const WhereClause*>(where);
            if (containsPhrase(toLower(w->expression), "(select")) hints++;
        }
        auto* from = q->getChild("from");
        if (from && from->conceptType == "TableDeclaration") {
            const auto* t = static_cast<const TableDeclaration*>(from);
            if (containsPhrase(toLower(t->name), "(select")) hints++;
        }
        for (const auto* j : q->getChildren("joins")) {
            auto* jc = static_cast<const JoinClause*>(j);
            if (containsPhrase(toLower(jc->tableName), "(select")) hints++;
        }
        return hints;
    }

    static std::string complexityFromQuery(int joinDepth, int subqueries) {
        if (joinDepth >= 3 || subqueries >= 2) return "O(n^3)";
        if (joinDepth >= 2 || subqueries >= 1) return "O(n^2)";
        if (joinDepth >= 1) return "O(n log n)";
        return "O(n)";
    }

    static double confidenceFromComplexity(int joinDepth, int subqueries) {
        return std::min(0.95, 0.55 + 0.1 * joinDepth + 0.15 * subqueries);
    }

    static std::unordered_set<std::string> collectClassFields(const ClassDeclaration* cls) {
        std::unordered_set<std::string> out;
        for (const auto* f : cls->getChildren("fields")) {
            if (f->conceptType != "Variable") continue;
            out.insert(toLower(static_cast<const Variable*>(f)->name));
        }
        return out;
    }

    static std::unordered_set<std::string> collectTableColumns(const TableDeclaration* table) {
        std::unordered_set<std::string> out;
        for (const auto* c : table->getChildren("columns")) {
            if (c->conceptType != "ColumnDefinition") continue;
            out.insert(toLower(static_cast<const ColumnDefinition*>(c)->name));
        }
        return out;
    }

    static int countIntersection(const std::unordered_set<std::string>& a,
                                 const std::unordered_set<std::string>& b) {
        int count = 0;
        for (const auto& v : a) if (b.count(v)) ++count;
        return count;
    }

    static Annotation* createAnnotationNode(const InferredAnnotation& inf) {
        if (inf.annotationType == "RiskAnnotation") {
            auto* a = new RiskAnnotation();
            a->level = inf.value;
            a->reason = inf.reason;
            return a;
        }
        if (inf.annotationType == "ComplexityAnnotation") {
            auto* a = new ComplexityAnnotation();
            a->timeComplexity = inf.value;
            return a;
        }
        if (inf.annotationType == "BoundsCheckAnnotation") {
            auto* a = new BoundsCheckAnnotation();
            a->enabled = (toLower(inf.value) != "false");
            return a;
        }
        if (inf.annotationType == "ContractAnnotation") {
            auto* a = new ContractAnnotation();
            if (inf.key == "preconditions") a->preconditions = inf.value;
            else a->preconditions = inf.key + "=" + inf.value;
            return a;
        }
        return nullptr;
    }

    static ASTNode* findNodeById(ASTNode* node, const std::string& id) {
        if (!node) return nullptr;
        if (node->id == id) return node;
        for (auto* c : node->allChildren()) {
            if (auto* hit = findNodeById(c, id)) return hit;
        }
        return nullptr;
    }

    static bool containsPhrase(const std::string& text, const std::string& phrase) {
        return text.find(phrase) != std::string::npos;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return out;
    }
};
