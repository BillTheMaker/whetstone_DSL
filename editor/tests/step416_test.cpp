// Step 416: Phase 17b Integration + Sprint Summary (8 tests)

#include "Pipeline.h"
#include "SqlAnnotationMapper.h"
#include "ast/ClassDeclaration.h"
#include "ast/Variable.h"
#include "ast/SqlNodes.h"
#include "ast/PostgreSQLGenerator.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_sql_dialect_parse_routes() {
    TEST(sql_dialect_parse_routes);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3, d4, d5;
    auto pg = p.parse("SELECT id FROM users;\n", "postgresql", d1);
    auto ts = p.parse("SELECT TOP 1 id FROM users;\n", "tsql", d2);
    auto ss = p.parse("SELECT TOP 1 id FROM users;\n", "sqlserver", d3);
    auto my = p.parse("SELECT id FROM users LIMIT 1;\n", "mysql", d4);
    auto ma = p.parse("SELECT id FROM users LIMIT 1;\n", "mariadb", d5);
    CHECK(pg && ts && ss && my && ma, "expected parse success for all SQL routes");
    CHECK(pg->targetLanguage == "postgresql", "postgresql target mismatch");
    CHECK(ts->targetLanguage == "tsql", "tsql target mismatch");
    CHECK(my->targetLanguage == "mysql", "mysql target mismatch");
    PASS();
}

void test_sql_dialect_generate_routes() {
    TEST(sql_dialect_generate_routes);
    DeleteStatement del("d1", "audit");
    Pipeline p;
    std::string pg = p.generate(&del, "postgresql");
    std::string ts = p.generate(&del, "tsql");
    std::string my = p.generate(&del, "mysql");
    CHECK(pg == "DELETE FROM audit;", "postgres delete format mismatch");
    CHECK(ts == "DELETE FROM audit;", "tsql delete format mismatch");
    CHECK(my == "DELETE FROM `audit`;", "mysql delete format mismatch");
    PASS();
}

void test_sql_risk_inference_delete_without_where() {
    TEST(sql_risk_inference_delete_without_where);
    Pipeline p;
    std::vector<ParseDiagnostic> diags;
    auto mod = p.parse("DELETE FROM users;\n", "postgresql", diags);
    CHECK(mod != nullptr, "parse failed");
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(mod.get());
    bool found = false;
    for (const auto& a : inf) {
        if (a.annotationType == "RiskAnnotation" && a.value == "high") {
            found = true;
            break;
        }
    }
    CHECK(found, "expected high-risk inference");
    PASS();
}

void test_sql_complexity_inference_join_depth() {
    TEST(sql_complexity_inference_join_depth);
    Pipeline p;
    std::vector<ParseDiagnostic> diags;
    auto mod = p.parse(
        "SELECT u.id FROM users u "
        "LEFT JOIN orders o ON u.id = o.user_id "
        "LEFT JOIN profiles p ON p.user_id = u.id;\n",
        "postgresql", diags);
    CHECK(mod != nullptr, "parse failed");
    SqlAnnotationMapper mapper;
    auto inf = mapper.inferForModule(mod.get());
    bool found = false;
    for (const auto& a : inf) {
        if (a.annotationType == "ComplexityAnnotation" &&
            a.value.find("O(n^2)") != std::string::npos) {
            found = true;
            break;
        }
    }
    CHECK(found, "expected O(n^2) complexity inference");
    PASS();
}

void test_sql_boundscheck_inference_limit_top() {
    TEST(sql_boundscheck_inference_limit_top);
    SqlAnnotationMapper mapper;
    auto tInf = mapper.inferForSqlText("SELECT TOP 5 id FROM users;");
    auto lInf = mapper.inferForSqlText("SELECT id FROM users LIMIT 10;");
    bool topFound = false, limitFound = false;
    for (const auto& a : tInf) if (a.annotationType == "BoundsCheckAnnotation") topFound = true;
    for (const auto& a : lInf) if (a.annotationType == "BoundsCheckAnnotation") limitFound = true;
    CHECK(topFound, "expected TOP bounds check");
    CHECK(limitFound, "expected LIMIT bounds check");
    PASS();
}

void test_cross_stack_python_to_csharp_to_sql_table() {
    TEST(cross_stack_python_to_csharp_to_sql_table);
    Pipeline p;
    std::vector<ParseDiagnostic> pyDiags;
    auto py = p.parse("class User:\n    def save(self):\n        return 1\n", "python", pyDiags);
    CHECK(py != nullptr, "python parse failed");

    ClassDeclaration cls("c1", "User");
    cls.addChild("fields", new Variable("f1", "id"));
    cls.addChild("fields", new Variable("f2", "email"));
    std::string csharp = p.generate(&cls, "csharp");
    CHECK(csharp.find("class User") != std::string::npos, "expected csharp class");

    TableDeclaration table("t1", "users");
    table.addChild("columns", new ColumnDefinition("col1", "id", "INT", false));
    table.addChild("columns", new ColumnDefinition("col2", "email", "TEXT", false));
    PostgreSQLGenerator pgGen;
    std::string ddl = pgGen.generate(&table);
    CHECK(ddl.find("CREATE TABLE users") != std::string::npos, "expected sql table");
    PASS();
}

void test_sql_contract_mapping_for_orm_schema() {
    TEST(sql_contract_mapping_for_orm_schema);
    ClassDeclaration cls("c1", "User");
    cls.addChild("fields", new Variable("f1", "id"));
    cls.addChild("fields", new Variable("f2", "email"));
    TableDeclaration table("t1", "users");
    table.addChild("columns", new ColumnDefinition("col1", "id", "INT", false));
    table.addChild("columns", new ColumnDefinition("col2", "email", "TEXT", false));

    SqlAnnotationMapper mapper;
    auto inf = mapper.inferCrossStackOrmMapping(&cls, &table);
    bool classContract = false, tableContract = false;
    for (const auto& a : inf) {
        if (a.nodeId == "c1" && a.annotationType == "ContractAnnotation") classContract = true;
        if (a.nodeId == "t1" && a.annotationType == "ContractAnnotation") tableContract = true;
    }
    CHECK(classContract && tableContract, "expected cross-stack contracts");
    PASS();
}

void test_sprint17_totals_verification() {
    TEST(sprint17_totals_verification);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3, d4, d5;
    auto fs = p.parse("let inc x = x + 1\n", "fsharp", d1);
    auto vb = p.parse("Sub Main()\nEnd Sub\n", "vbnet", d2);
    auto pg = p.parse("SELECT id FROM users;\n", "postgresql", d3);
    auto ts = p.parse("SELECT TOP 1 id FROM users;\n", "tsql", d4);
    auto my = p.parse("SELECT id FROM users LIMIT 1;\n", "mysql", d5);
    CHECK(fs && vb && pg && ts && my, "expected Sprint 17 language parse support");

    int sqlDialects = 0;
    if (pg) ++sqlDialects;
    if (ts) ++sqlDialects;
    if (my) ++sqlDialects;
    CHECK(sqlDialects == 3, "expected 3 SQL dialects in Sprint 17");
    PASS();
}

int main() {
    std::cout << "Step 416: Phase 17b Integration + Sprint Summary\n";

    test_sql_dialect_parse_routes();             // 1
    test_sql_dialect_generate_routes();          // 2
    test_sql_risk_inference_delete_without_where(); // 3
    test_sql_complexity_inference_join_depth();  // 4
    test_sql_boundscheck_inference_limit_top();  // 5
    test_cross_stack_python_to_csharp_to_sql_table(); // 6
    test_sql_contract_mapping_for_orm_schema();  // 7
    test_sprint17_totals_verification();         // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
