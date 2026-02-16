// Step 413: T-SQL Parser + Generator Tests (12 tests)

#include "ast/TSQLParser.h"
#include "ast/TSQLGenerator.h"
#include "ast/SqlNodes.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "Pipeline.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

template <typename T>
T* firstStatementOf(const Module* mod, const std::string& conceptName) {
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == conceptName) return static_cast<T*>(s);
    }
    return nullptr;
}

void test_parse_create_table_identity_nvarchar() {
    TEST(parse_create_table_identity_nvarchar);
    auto mod = TSQLParser::parseTSQL(
        "CREATE TABLE users (id INT IDENTITY(1,1), name NVARCHAR(100) NOT NULL);\n");
    CHECK(mod != nullptr, "module null");
    auto* t = firstStatementOf<TableDeclaration>(mod.get(), "TableDeclaration");
    CHECK(t != nullptr, "expected table declaration");
    CHECK(t->name == "users", "expected users table");
    auto cols = t->getChildren("columns");
    CHECK(cols.size() == 2, "expected two columns");
    CHECK(static_cast<ColumnDefinition*>(cols[0])->dataType.find("IDENTITY") != std::string::npos,
          "expected identity type");
    CHECK(static_cast<ColumnDefinition*>(cols[1])->dataType.find("NVARCHAR") != std::string::npos,
          "expected nvarchar type");
    PASS();
}

void test_parse_select_top_where() {
    TEST(parse_select_top_where);
    auto mod = TSQLParser::parseTSQL("SELECT TOP 5 id FROM users WHERE active = 1;\n");
    auto* q = firstStatementOf<SelectQuery>(mod.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select query");
    CHECK(q->getChild("top") != nullptr, "expected top child");
    CHECK(q->getChild("where") != nullptr, "expected where child");
    PASS();
}

void test_parse_create_procedure_marker() {
    TEST(parse_create_procedure_marker);
    auto mod = TSQLParser::parseTSQL(
        "CREATE PROCEDURE usp_cleanup AS BEGIN DELETE FROM logs; END;\n");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() == 1, "expected one procedure marker");
    auto* fn = static_cast<Function*>(fns.front());
    CHECK(fn->name == "usp_cleanup", "expected procedure name");
    PASS();
}

void test_parse_declare_variable() {
    TEST(parse_declare_variable);
    auto mod = TSQLParser::parseTSQL("DECLARE @count INT = 10;\n");
    auto vars = mod->getChildren("variables");
    CHECK(vars.size() == 1, "expected one variable");
    auto* v = static_cast<Variable*>(vars.front());
    CHECK(v->name == "@count", "expected variable name");
    CHECK(v->getChild("initializer") != nullptr, "expected initializer");
    PASS();
}

void test_parse_set_assignment() {
    TEST(parse_set_assignment);
    auto mod = TSQLParser::parseTSQL("SET @count = 11;\n");
    auto* a = firstStatementOf<Assignment>(mod.get(), "Assignment");
    CHECK(a != nullptr, "expected assignment statement");
    auto* target = static_cast<VariableReference*>(a->getChild("target"));
    CHECK(target != nullptr && target->variableName == "@count", "expected assignment target");
    PASS();
}

void test_parse_print_statement() {
    TEST(parse_print_statement);
    auto mod = TSQLParser::parseTSQL("PRINT 'done';\n");
    auto* es = firstStatementOf<ExpressionStatement>(mod.get(), "ExpressionStatement");
    CHECK(es != nullptr, "expected expression statement");
    auto* call = static_cast<FunctionCall*>(es->getChild("expression"));
    CHECK(call != nullptr && call->functionName == "PRINT", "expected print call");
    PASS();
}

void test_generate_select_top_clause() {
    TEST(generate_select_top_clause);
    SelectQuery q;
    q.id = "q1";
    q.setChild("top", new IntegerLiteral("i1", 3));
    q.setChild("from", new TableDeclaration("t1", "users"));
    TSQLGenerator gen;
    std::string out = gen.generate(&q);
    CHECK(out.find("SELECT TOP 3") != std::string::npos, "expected top syntax");
    CHECK(out.find("FROM users") != std::string::npos, "expected from clause");
    PASS();
}

void test_generate_create_table_identity() {
    TEST(generate_create_table_identity);
    TableDeclaration t("t1", "users");
    t.addChild("columns", new ColumnDefinition("c1", "id", "INT IDENTITY(1,1)", false));
    t.addChild("columns", new ColumnDefinition("c2", "name", "NVARCHAR(100)", false));
    TSQLGenerator gen;
    std::string out = gen.generate(&t);
    CHECK(out.find("CREATE TABLE users") != std::string::npos, "expected create table");
    CHECK(out.find("INT IDENTITY(1,1)") != std::string::npos, "expected identity");
    CHECK(out.find("NVARCHAR(100)") != std::string::npos, "expected nvarchar");
    PASS();
}

void test_generate_procedure_begin_end() {
    TEST(generate_procedure_begin_end);
    Function fn("f1", "usp_sync");
    fn.addChild("body", new Return());
    TSQLGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(out.find("CREATE PROCEDURE usp_sync") != std::string::npos, "expected procedure keyword");
    CHECK(out.find("BEGIN") != std::string::npos, "expected begin block");
    CHECK(out.find("END;") != std::string::npos, "expected end block");
    PASS();
}

void test_generate_declare_set_print() {
    TEST(generate_declare_set_print);
    Variable v("v1", "@n");
    v.setChild("type", new PrimitiveType("t1", "int"));
    v.setChild("initializer", new IntegerLiteral("i1", 1));

    Assignment a;
    a.id = "a1";
    a.setChild("target", new VariableReference("r1", "@n"));
    a.setChild("value", new IntegerLiteral("i2", 2));

    FunctionCall printCall;
    printCall.id = "p1";
    printCall.functionName = "PRINT";
    printCall.addChild("arguments", new StringLiteral("s1", "ok"));

    TSQLGenerator gen;
    std::string d = gen.generate(&v);
    std::string s = gen.generate(&a);
    std::string p = gen.generate(&printCall);
    CHECK(d.find("DECLARE @n INT = 1;") != std::string::npos, "expected declare");
    CHECK(s.find("SET @n = 2;") != std::string::npos, "expected set");
    CHECK(p == "PRINT 'ok';", "expected print syntax");
    PASS();
}

void test_pipeline_parse_and_generate_aliases() {
    TEST(pipeline_parse_and_generate_aliases);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2, d3;
    auto m1 = p.parse("SELECT TOP 1 id FROM users;\n", "tsql", d1);
    auto m2 = p.parse("SELECT TOP 1 id FROM users;\n", "sqlserver", d2);
    auto m3 = p.parse("SELECT TOP 1 id FROM users;\n", "mssql", d3);
    CHECK(m1 && m2 && m3, "expected parse via aliases");
    CHECK(m1->targetLanguage == "tsql", "expected tsql target");

    DeleteStatement del("d1", "audit");
    std::string o1 = p.generate(&del, "tsql");
    std::string o2 = p.generate(&del, "sqlserver");
    std::string o3 = p.generate(&del, "mssql");
    CHECK(o1.find("DELETE FROM audit;") != std::string::npos, "tsql route");
    CHECK(o2.find("DELETE FROM audit;") != std::string::npos, "sqlserver route");
    CHECK(o3.find("DELETE FROM audit;") != std::string::npos, "mssql route");
    PASS();
}

void test_parser_to_generator_tsql_flow() {
    TEST(parser_to_generator_tsql_flow);
    auto mod = TSQLParser::parseTSQL(
        "DECLARE @limit INT = 10;\n"
        "SELECT TOP 10 id FROM users WHERE active = 1;\n");
    CHECK(mod != nullptr, "module null");
    TSQLGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("DECLARE @limit INT = 10;") != std::string::npos, "expected declare output");
    CHECK(out.find("SELECT TOP 10") != std::string::npos, "expected top output");
    PASS();
}

int main() {
    std::cout << "Step 413: T-SQL Parser + Generator Tests\n";

    test_parse_create_table_identity_nvarchar(); // 1
    test_parse_select_top_where();               // 2
    test_parse_create_procedure_marker();        // 3
    test_parse_declare_variable();               // 4
    test_parse_set_assignment();                 // 5
    test_parse_print_statement();                // 6
    test_generate_select_top_clause();           // 7
    test_generate_create_table_identity();       // 8
    test_generate_procedure_begin_end();         // 9
    test_generate_declare_set_print();           // 10
    test_pipeline_parse_and_generate_aliases();  // 11
    test_parser_to_generator_tsql_flow();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
