// Step 412: PostgreSQL Generator Tests (12 tests)

#include "ast/PostgreSQLGenerator.h"
#include "ast/PostgreSQLParser.h"
#include "Pipeline.h"
#include "ast/SqlNodes.h"
#include "ast/Annotation.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_generate_create_table_with_columns() {
    TEST(generate_create_table_with_columns);
    TableDeclaration t("t1", "users");
    t.addChild("columns", new ColumnDefinition("c1", "id", "SERIAL", false));
    t.addChild("columns", new ColumnDefinition("c2", "name", "TEXT", false));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&t);
    CHECK(out.find("CREATE TABLE users") != std::string::npos, "expected create table");
    CHECK(out.find("id SERIAL NOT NULL") != std::string::npos, "expected id column");
    CHECK(out.find("name TEXT NOT NULL") != std::string::npos, "expected name column");
    PASS();
}

void test_generate_select_distinct_join_where() {
    TEST(generate_select_distinct_join_where);
    SelectQuery q;
    q.id = "q1";
    q.distinct = true;
    q.addChild("columns", new ColumnDefinition("c1", "id", "INTEGER"));
    q.setChild("from", new TableDeclaration("t1", "users"));
    auto* j = new JoinClause("j1", "LEFT", "orders");
    j->setChild("on", new WhereClause("w1", "users.id = orders.user_id"));
    q.addChild("joins", j);
    q.setChild("where", new WhereClause("w2", "users.active = TRUE"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&q);
    CHECK(out.find("SELECT DISTINCT") != std::string::npos, "expected distinct");
    CHECK(out.find("LEFT JOIN orders") != std::string::npos, "expected join");
    CHECK(out.find("WHERE users.active = TRUE") != std::string::npos, "expected where");
    PASS();
}

void test_generate_insert_statement() {
    TEST(generate_insert_statement);
    InsertStatement ins("i1", "users");
    ins.addChild("columns", new ColumnDefinition("c1", "id", "INTEGER"));
    ins.addChild("columns", new ColumnDefinition("c2", "name", "TEXT"));
    ins.addChild("values", new IntegerLiteral("v1", 1));
    ins.addChild("values", new StringLiteral("v2", "alice"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&ins);
    CHECK(out.find("INSERT INTO users") != std::string::npos, "expected insert target");
    CHECK(out.find("(id, name)") != std::string::npos, "expected column list");
    CHECK(out.find("VALUES (1, 'alice')") != std::string::npos, "expected values list");
    PASS();
}

void test_generate_update_statement() {
    TEST(generate_update_statement);
    UpdateStatement up("u1", "users");
    auto* set = new Assignment();
    set->id = "a1";
    set->setChild("target", new VariableReference("vr1", "name"));
    set->setChild("value", new StringLiteral("s1", "bob"));
    up.addChild("set", set);
    up.setChild("where", new WhereClause("w1", "id = 1"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&up);
    CHECK(out.find("UPDATE users SET") != std::string::npos, "expected update");
    CHECK(out.find("name = 'bob'") != std::string::npos, "expected set assignment");
    CHECK(out.find("WHERE id = 1") != std::string::npos, "expected where");
    PASS();
}

void test_generate_delete_statement() {
    TEST(generate_delete_statement);
    DeleteStatement del("d1", "users");
    del.setChild("where", new WhereClause("w1", "id = 7"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&del);
    CHECK(out == "DELETE FROM users WHERE id = 7;", "expected delete sql");
    PASS();
}

void test_generate_index_definition() {
    TEST(generate_index_definition);
    IndexDefinition idx("x1", "idx_users_email", "users", true);
    idx.addChild("columns", new ColumnDefinition("c1", "email", "TEXT"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&idx);
    CHECK(out.find("CREATE UNIQUE INDEX idx_users_email ON users") != std::string::npos,
          "expected unique index");
    CHECK(out.find("(email)") != std::string::npos, "expected index column");
    PASS();
}

void test_generate_module_with_statements() {
    TEST(generate_module_with_statements);
    Module mod;
    mod.id = "m1";
    mod.name = "sql_mod";
    mod.addChild("statements", new DeleteStatement("d1", "logs"));

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&mod);
    CHECK(out.find("-- Module: sql_mod") != std::string::npos, "expected module header");
    CHECK(out.find("DELETE FROM logs;") != std::string::npos, "expected statement content");
    PASS();
}

void test_generate_where_clause_passthrough() {
    TEST(generate_where_clause_passthrough);
    WhereClause where("w1", "status = 'ok'");
    PostgreSQLGenerator gen;
    std::string out = gen.generate(&where);
    CHECK(out == "WHERE status = 'ok'", "expected where passthrough");
    PASS();
}

void test_comment_prefix_and_semanno_output() {
    TEST(comment_prefix_and_semanno_output);
    Function fn("f1", "process");
    auto* risk = new RiskAnnotation();
    risk->id = "r1";
    risk->level = "medium";
    fn.addChild("annotations", risk);

    PostgreSQLGenerator gen;
    std::string out = gen.generate(&fn);
    CHECK(gen.commentPrefix() == "-- ", "expected postgres comment prefix");
    CHECK(out.find("@semanno:risk") != std::string::npos, "expected semanno output");
    PASS();
}

void test_pipeline_generate_routing_aliases() {
    TEST(pipeline_generate_routing_aliases);
    DeleteStatement del("d1", "audit");
    Pipeline p;
    std::string o1 = p.generate(&del, "postgresql");
    std::string o2 = p.generate(&del, "postgres");
    CHECK(o1 == "DELETE FROM audit;", "postgresql route");
    CHECK(o2 == "DELETE FROM audit;", "postgres route");
    PASS();
}

void test_parser_to_generator_create_table_flow() {
    TEST(parser_to_generator_create_table_flow);
    auto mod = PostgreSQLParser::parsePostgreSQL(
        "CREATE TABLE events (id SERIAL, payload JSONB, tags TEXT[]);\n");
    CHECK(mod != nullptr, "module null");

    PostgreSQLGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("CREATE TABLE events") != std::string::npos, "expected create table");
    CHECK(out.find("payload JSONB") != std::string::npos, "expected jsonb column");
    CHECK(out.find("tags TEXT[]") != std::string::npos, "expected array column");
    PASS();
}

void test_generate_select_edge_case_defaults_to_star() {
    TEST(generate_select_edge_case_defaults_to_star);
    SelectQuery q;
    q.id = "q2";
    PostgreSQLGenerator gen;
    std::string out = gen.generate(&q);
    CHECK(out == "SELECT *;", "expected default select star");
    PASS();
}

int main() {
    std::cout << "Step 412: PostgreSQL Generator Tests\n";

    test_generate_create_table_with_columns();         // 1
    test_generate_select_distinct_join_where();        // 2
    test_generate_insert_statement();                  // 3
    test_generate_update_statement();                  // 4
    test_generate_delete_statement();                  // 5
    test_generate_index_definition();                  // 6
    test_generate_module_with_statements();            // 7
    test_generate_where_clause_passthrough();          // 8
    test_comment_prefix_and_semanno_output();          // 9
    test_pipeline_generate_routing_aliases();          // 10
    test_parser_to_generator_create_table_flow();      // 11
    test_generate_select_edge_case_defaults_to_star(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
