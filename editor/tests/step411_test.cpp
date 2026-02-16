// Step 411: PostgreSQL Parser Tests (12 tests)

#include "ast/PostgreSQLParser.h"
#include "Pipeline.h"
#include "ast/SqlNodes.h"
#include "ast/Function.h"

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

void test_parse_create_table() {
    TEST(parse_create_table);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "CREATE TABLE users (id SERIAL PRIMARY KEY, name TEXT NOT NULL);\n");
    CHECK(mod != nullptr, "module is null");
    auto* t = firstStatementOf<TableDeclaration>(mod.get(), "TableDeclaration");
    CHECK(t != nullptr, "expected table declaration");
    CHECK(t->name == "users", "expected table users");
    CHECK(t->getChildren("columns").size() >= 2, "expected parsed columns");
    PASS();
}

void test_parse_select_query() {
    TEST(parse_select_query);
    auto mod = PostgreSQLParser::parsePostgreSQL("SELECT id, name FROM users;\n");
    auto* q = firstStatementOf<SelectQuery>(mod.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select query");
    CHECK(q->getChild("from") != nullptr, "expected from child");
    PASS();
}

void test_parse_insert_statement() {
    TEST(parse_insert_statement);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "INSERT INTO users (id, name) VALUES (1, 'a');\n");
    auto* ins = firstStatementOf<InsertStatement>(mod.get(), "InsertStatement");
    CHECK(ins != nullptr, "expected insert");
    CHECK(ins->tableName == "users", "expected insert table users");
    PASS();
}

void test_parse_update_statement() {
    TEST(parse_update_statement);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "UPDATE users SET name = 'b' WHERE id = 1;\n");
    auto* up = firstStatementOf<UpdateStatement>(mod.get(), "UpdateStatement");
    CHECK(up != nullptr, "expected update");
    CHECK(up->tableName == "users", "expected update table users");
    CHECK(up->getChild("where") != nullptr, "expected where clause");
    PASS();
}

void test_parse_delete_statement() {
    TEST(parse_delete_statement);
    auto mod = PostgreSQLParser::parsePostgreSQL("DELETE FROM users WHERE id = 2;\n");
    auto* del = firstStatementOf<DeleteStatement>(mod.get(), "DeleteStatement");
    CHECK(del != nullptr, "expected delete");
    CHECK(del->tableName == "users", "expected delete table users");
    CHECK(del->getChild("where") != nullptr, "expected where");
    PASS();
}

void test_parse_join_and_where() {
    TEST(parse_join_and_where);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "SELECT u.id FROM users u LEFT JOIN orders o ON u.id = o.user_id WHERE u.active = true;\n");
    auto* q = firstStatementOf<SelectQuery>(mod.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select");
    CHECK(!q->getChildren("joins").empty(), "expected join clause");
    CHECK(q->getChild("where") != nullptr, "expected where clause");
    PASS();
}

void test_parse_postgres_types_serial_jsonb_array() {
    TEST(parse_postgres_types_serial_jsonb_array);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "CREATE TABLE events (id SERIAL, payload JSONB, tags TEXT[]);\n");
    auto* t = firstStatementOf<TableDeclaration>(mod.get(), "TableDeclaration");
    CHECK(t != nullptr, "expected table");
    auto cols = t->getChildren("columns");
    CHECK(cols.size() == 3, "expected 3 columns");
    CHECK(static_cast<ColumnDefinition*>(cols[0])->dataType == "SERIAL", "expected SERIAL");
    CHECK(static_cast<ColumnDefinition*>(cols[1])->dataType == "JSONB", "expected JSONB");
    CHECK(static_cast<ColumnDefinition*>(cols[2])->dataType == "TEXT[]", "expected TEXT[]");
    PASS();
}

void test_parse_schema_qualified_names() {
    TEST(parse_schema_qualified_names);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "CREATE TABLE public.accounts (id BIGINT, email TEXT);\n");
    auto* t = firstStatementOf<TableDeclaration>(mod.get(), "TableDeclaration");
    CHECK(t != nullptr, "expected table");
    CHECK(t->name == "public.accounts", "expected schema-qualified name");
    CHECK(t->schema == "public", "expected schema public");
    PASS();
}

void test_parse_create_index() {
    TEST(parse_create_index);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "CREATE UNIQUE INDEX idx_users_email ON users (email);\n");
    auto* idx = firstStatementOf<IndexDefinition>(mod.get(), "IndexDefinition");
    CHECK(idx != nullptr, "expected index");
    CHECK(idx->name == "idx_users_email", "expected index name");
    CHECK(idx->tableName == "users", "expected table name");
    CHECK(idx->unique, "expected unique index");
    PASS();
}

void test_parse_do_block_and_plpgsql_function_markers() {
    TEST(parse_do_block_and_plpgsql_function_markers);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "DO $$ BEGIN RAISE NOTICE 'x'; END $$;\n"
        "CREATE FUNCTION add_one(x INTEGER) RETURNS INTEGER AS $$ BEGIN RETURN x + 1; END; $$ LANGUAGE plpgsql;\n");
    auto fns = mod->getChildren("functions");
    CHECK(fns.size() >= 2, "expected function markers for DO and CREATE FUNCTION");
    bool hasDo = false, hasFn = false;
    for (auto* f : fns) {
        auto* fn = static_cast<Function*>(f);
        if (fn->name == "do_block") hasDo = true;
        if (fn->name == "add_one") hasFn = true;
    }
    CHECK(hasDo && hasFn, "expected both markers");
    PASS();
}

void test_parse_with_diagnostics_entrypoint() {
    TEST(parse_with_diagnostics_entrypoint);
    auto result = PostgreSQLParser::parsePostgreSQLWithDiagnostics(
        "SELECT DISTINCT id FROM users;\n");
    CHECK(result.module != nullptr, "module null");
    auto* q = firstStatementOf<SelectQuery>(result.module.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select");
    CHECK(q->distinct, "expected distinct flag");
    PASS();
}

void test_pipeline_parse_routing_aliases() {
    TEST(pipeline_parse_routing_aliases);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2;
    auto m1 = p.parse("SELECT id FROM users;\n", "postgresql", d1);
    auto m2 = p.parse("SELECT id FROM users;\n", "postgres", d2);
    CHECK(m1 != nullptr && m2 != nullptr, "expected parse success");
    CHECK(m1->targetLanguage == "postgresql", "expected postgresql target");
    CHECK(m2->targetLanguage == "postgresql", "expected alias target");
    PASS();
}

int main() {
    std::cout << "Step 411: PostgreSQL Parser Tests\n";

    test_parse_create_table();                       // 1
    test_parse_select_query();                       // 2
    test_parse_insert_statement();                   // 3
    test_parse_update_statement();                   // 4
    test_parse_delete_statement();                   // 5
    test_parse_join_and_where();                     // 6
    test_parse_postgres_types_serial_jsonb_array(); // 7
    test_parse_schema_qualified_names();             // 8
    test_parse_create_index();                       // 9
    test_parse_do_block_and_plpgsql_function_markers(); // 10
    test_parse_with_diagnostics_entrypoint();        // 11
    test_pipeline_parse_routing_aliases();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
