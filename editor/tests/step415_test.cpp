// Step 415: SQL Annotation Mapping Tests (12 tests)

#include "SqlAnnotationMapper.h"
#include "ast/Module.h"
#include "ast/SqlNodes.h"
#include "ast/ClassDeclaration.h"
#include "ast/Variable.h"
#include "ast/Annotation.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasInferred(const std::vector<SqlAnnotationMapper::InferredAnnotation>& v,
                        const std::string& nodeId,
                        const std::string& annoType,
                        const std::string& valueContains = "") {
    for (const auto& a : v) {
        if (a.nodeId == nodeId && a.annotationType == annoType) {
            if (valueContains.empty()) return true;
            if (a.value.find(valueContains) != std::string::npos ||
                a.reason.find(valueContains) != std::string::npos) return true;
        }
    }
    return false;
}

void test_risk_high_delete_without_where() {
    TEST(risk_high_delete_without_where);
    Module mod;
    mod.id = "m1";
    auto* d = new DeleteStatement("del1", "users");
    mod.addChild("statements", d);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "del1", "RiskAnnotation", "high"), "expected high risk");
    PASS();
}

void test_no_high_risk_delete_with_where() {
    TEST(no_high_risk_delete_with_where);
    Module mod;
    mod.id = "m1";
    auto* d = new DeleteStatement("del1", "users");
    d->setChild("where", new WhereClause("w1", "id = 1"));
    mod.addChild("statements", d);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(!hasInferred(inf, "del1", "RiskAnnotation", "high"), "should not flag high risk");
    PASS();
}

void test_risk_high_drop_table_text() {
    TEST(risk_high_drop_table_text);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForSqlText("DROP TABLE users;");
    CHECK(hasInferred(inf, "sql_text", "RiskAnnotation", "high"), "expected drop table high risk");
    PASS();
}

void test_complexity_from_join_depth() {
    TEST(complexity_from_join_depth);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->addChild("joins", new JoinClause("j1", "LEFT", "orders"));
    q->addChild("joins", new JoinClause("j2", "LEFT", "profiles"));
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "q1", "ComplexityAnnotation", "O(n^2)"),
          "expected O(n^2) for multi-join query");
    PASS();
}

void test_complexity_from_subquery_hint() {
    TEST(complexity_from_subquery_hint);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->setChild("where", new WhereClause("w1", "id IN (SELECT user_id FROM orders)"));
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "q1", "ComplexityAnnotation", "O(n^2)"),
          "expected O(n^2) for subquery");
    PASS();
}

void test_boundscheck_from_limit_node() {
    TEST(boundscheck_from_limit_node);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->setChild("limit", new IntegerLiteral("i1", 10));
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "q1", "BoundsCheckAnnotation"), "expected bounds check");
    PASS();
}

void test_boundscheck_from_top_node() {
    TEST(boundscheck_from_top_node);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->setChild("top", new IntegerLiteral("i1", 5));
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "q1", "BoundsCheckAnnotation"), "expected bounds check");
    PASS();
}

void test_no_boundscheck_without_limit_or_top() {
    TEST(no_boundscheck_without_limit_or_top);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(!hasInferred(inf, "q1", "BoundsCheckAnnotation"), "unexpected bounds check");
    PASS();
}

void test_contract_for_index_dependent_query() {
    TEST(contract_for_index_dependent_query);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->setChild("where", new WhereClause("w1", "email = 'x'"));
    mod.addChild("statements", q);
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    CHECK(hasInferred(inf, "q1", "ContractAnnotation", "index exists"),
          "expected index precondition contract");
    PASS();
}

void test_cross_stack_orm_to_sql_contract() {
    TEST(cross_stack_orm_to_sql_contract);
    ClassDeclaration cls("c1", "User");
    cls.addChild("fields", new Variable("f1", "id"));
    cls.addChild("fields", new Variable("f2", "email"));
    TableDeclaration table("t1", "users");
    table.addChild("columns", new ColumnDefinition("col1", "id", "INT"));
    table.addChild("columns", new ColumnDefinition("col2", "email", "TEXT"));

    SqlAnnotationMapper mapper;
    auto inf = mapper.inferCrossStackOrmMapping(&cls, &table);
    CHECK(hasInferred(inf, "c1", "ContractAnnotation", "sql table users exists"),
          "expected class contract");
    CHECK(hasInferred(inf, "t1", "ContractAnnotation", "orm class User"),
          "expected table contract");
    PASS();
}

void test_cross_stack_mismatch_risk() {
    TEST(cross_stack_mismatch_risk);
    ClassDeclaration cls("c1", "Account");
    cls.addChild("fields", new Variable("f1", "name"));
    cls.addChild("fields", new Variable("f2", "createdAt"));
    TableDeclaration table("t1", "accounts");
    table.addChild("columns", new ColumnDefinition("col1", "id", "INT"));
    table.addChild("columns", new ColumnDefinition("col2", "email", "TEXT"));

    SqlAnnotationMapper mapper;
    auto inf = mapper.inferCrossStackOrmMapping(&cls, &table);
    CHECK(hasInferred(inf, "c1", "RiskAnnotation", "medium"), "expected mismatch risk");
    PASS();
}

void test_apply_inferred_annotations_to_ast() {
    TEST(apply_inferred_annotations_to_ast);
    Module mod;
    mod.id = "m1";
    auto* q = new SelectQuery();
    q->id = "q1";
    q->setChild("limit", new IntegerLiteral("i1", 10));
    mod.addChild("statements", q);

    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(&mod);
    mapper.applyInferredAnnotations(&mod, inf);

    auto annos = q->getChildren("annotations");
    bool hasBounds = false;
    bool hasComplex = false;
    for (auto* a : annos) {
        if (a->conceptType == "BoundsCheckAnnotation") hasBounds = true;
        if (a->conceptType == "ComplexityAnnotation") hasComplex = true;
    }
    CHECK(hasBounds, "expected applied BoundsCheckAnnotation");
    CHECK(hasComplex, "expected applied ComplexityAnnotation");
    PASS();
}

int main() {
    std::cout << "Step 415: SQL Annotation Mapping Tests\n";

    test_risk_high_delete_without_where();      // 1
    test_no_high_risk_delete_with_where();      // 2
    test_risk_high_drop_table_text();           // 3
    test_complexity_from_join_depth();          // 4
    test_complexity_from_subquery_hint();       // 5
    test_boundscheck_from_limit_node();         // 6
    test_boundscheck_from_top_node();           // 7
    test_no_boundscheck_without_limit_or_top(); // 8
    test_contract_for_index_dependent_query();  // 9
    test_cross_stack_orm_to_sql_contract();     // 10
    test_cross_stack_mismatch_risk();           // 11
    test_apply_inferred_annotations_to_ast();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
