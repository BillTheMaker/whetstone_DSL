// Step 414: MySQL Parser + Generator Tests (12 tests)

#include "ast/MySQLParser.h"
#include "ast/MySQLGenerator.h"
#include "ast/SqlNodes.h"
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

void test_parse_create_table_auto_increment_engine() {
    TEST(parse_create_table_auto_increment_engine);
    auto mod = MySQLParser::parseMySQL(
        "CREATE TABLE `users` (`id` INT AUTO_INCREMENT, `name` VARCHAR(255) NOT NULL) ENGINE=InnoDB;\n");
    auto* t = firstStatementOf<TableDeclaration>(mod.get(), "TableDeclaration");
    CHECK(t != nullptr, "expected table declaration");
    CHECK(t->name == "users", "expected stripped backtick name");
    auto cols = t->getChildren("columns");
    CHECK(cols.size() == 2, "expected columns");
    CHECK(static_cast<ColumnDefinition*>(cols[0])->dataType.find("AUTO_INCREMENT") != std::string::npos,
          "expected auto_increment");
    auto* e = t->getChild("engine");
    CHECK(e != nullptr && e->conceptType == "CustomType", "expected engine marker");
    PASS();
}

void test_parse_select_limit() {
    TEST(parse_select_limit);
    auto mod = MySQLParser::parseMySQL("SELECT id FROM `users` LIMIT 10;\n");
    auto* q = firstStatementOf<SelectQuery>(mod.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select");
    CHECK(q->getChild("limit") != nullptr, "expected limit child");
    PASS();
}

void test_parse_group_concat_projection() {
    TEST(parse_group_concat_projection);
    auto mod = MySQLParser::parseMySQL(
        "SELECT GROUP_CONCAT(name), id FROM users;\n");
    auto* q = firstStatementOf<SelectQuery>(mod.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select");
    auto cols = q->getChildren("columns");
    CHECK(cols.size() >= 2, "expected parsed columns");
    CHECK(cols[0]->conceptType == "FunctionCall", "expected group_concat call node");
    PASS();
}

void test_parse_insert_update_delete() {
    TEST(parse_insert_update_delete);
    auto mod = MySQLParser::parseMySQL(
        "INSERT INTO `users` (`id`) VALUES (1);\n"
        "UPDATE `users` SET id = 2 WHERE id = 1;\n"
        "DELETE FROM `users` WHERE id = 2;\n");
    CHECK(firstStatementOf<InsertStatement>(mod.get(), "InsertStatement") != nullptr, "insert parse");
    CHECK(firstStatementOf<UpdateStatement>(mod.get(), "UpdateStatement") != nullptr, "update parse");
    CHECK(firstStatementOf<DeleteStatement>(mod.get(), "DeleteStatement") != nullptr, "delete parse");
    PASS();
}

void test_parse_with_diagnostics_entrypoint() {
    TEST(parse_with_diagnostics_entrypoint);
    auto result = MySQLParser::parseMySQLWithDiagnostics("SELECT id FROM users LIMIT 5;\n");
    CHECK(result.module != nullptr, "module null");
    auto* q = firstStatementOf<SelectQuery>(result.module.get(), "SelectQuery");
    CHECK(q != nullptr, "expected select");
    CHECK(q->getChild("limit") != nullptr, "expected limit");
    PASS();
}

void test_generate_create_table_backticks_engine() {
    TEST(generate_create_table_backticks_engine);
    TableDeclaration t("t1", "users");
    t.addChild("columns", new ColumnDefinition("c1", "id", "INT AUTO_INCREMENT", false));
    t.addChild("columns", new ColumnDefinition("c2", "name", "VARCHAR(255)", false));
    t.setChild("engine", new CustomType("e1", "InnoDB"));
    MySQLGenerator gen;
    std::string out = gen.generate(&t);
    CHECK(out.find("CREATE TABLE `users`") != std::string::npos, "expected backtick table");
    CHECK(out.find("`id` INT AUTO_INCREMENT NOT NULL") != std::string::npos, "expected id col");
    CHECK(out.find("ENGINE=InnoDB") != std::string::npos, "expected engine");
    PASS();
}

void test_generate_select_limit() {
    TEST(generate_select_limit);
    SelectQuery q;
    q.id = "q1";
    q.addChild("columns", new ColumnDefinition("c1", "id", "INT"));
    q.setChild("from", new TableDeclaration("t1", "users"));
    q.setChild("limit", new IntegerLiteral("l1", 25));
    MySQLGenerator gen;
    std::string out = gen.generate(&q);
    CHECK(out.find("SELECT `id` INT") != std::string::npos, "expected column output");
    CHECK(out.find("FROM `users`") != std::string::npos, "expected from output");
    CHECK(out.find("LIMIT 25") != std::string::npos, "expected limit output");
    PASS();
}

void test_generate_group_concat() {
    TEST(generate_group_concat);
    FunctionCall c;
    c.id = "call1";
    c.functionName = "GROUP_CONCAT";
    MySQLGenerator gen;
    std::string out = gen.generate(&c);
    CHECK(out == "GROUP_CONCAT(*)", "expected group_concat form");
    PASS();
}

void test_generate_insert_with_backticks() {
    TEST(generate_insert_with_backticks);
    InsertStatement ins("i1", "users");
    ins.addChild("columns", new ColumnDefinition("c1", "id", "INT"));
    ins.addChild("values", new IntegerLiteral("v1", 7));
    MySQLGenerator gen;
    std::string out = gen.generate(&ins);
    CHECK(out.find("INSERT INTO `users`") != std::string::npos, "expected quoted table");
    CHECK(out.find("(`id`)") != std::string::npos, "expected quoted column");
    CHECK(out.find("VALUES (7)") != std::string::npos, "expected value");
    PASS();
}

void test_generate_update_delete_with_backticks() {
    TEST(generate_update_delete_with_backticks);
    UpdateStatement up("u1", "users");
    auto* set = new Assignment();
    set->id = "a1";
    set->setChild("target", new VariableReference("r1", "name"));
    set->setChild("value", new StringLiteral("s1", "alice"));
    up.addChild("set", set);
    up.setChild("where", new WhereClause("w1", "id = 1"));

    DeleteStatement del("d1", "users");
    del.setChild("where", new WhereClause("w2", "id = 1"));

    MySQLGenerator gen;
    std::string o1 = gen.generate(&up);
    std::string o2 = gen.generate(&del);
    CHECK(o1.find("UPDATE `users` SET") != std::string::npos, "expected update form");
    CHECK(o2.find("DELETE FROM `users` WHERE id = 1;") != std::string::npos, "expected delete form");
    PASS();
}

void test_pipeline_parse_and_generate_aliases() {
    TEST(pipeline_parse_and_generate_aliases);
    Pipeline p;
    std::vector<ParseDiagnostic> d1, d2;
    auto m1 = p.parse("SELECT id FROM users LIMIT 1;\n", "mysql", d1);
    auto m2 = p.parse("SELECT id FROM users LIMIT 1;\n", "mariadb", d2);
    CHECK(m1 != nullptr && m2 != nullptr, "expected parse success");
    CHECK(m1->targetLanguage == "mysql", "expected mysql target");

    DeleteStatement del("d1", "audit");
    std::string g1 = p.generate(&del, "mysql");
    std::string g2 = p.generate(&del, "mariadb");
    CHECK(g1.find("DELETE FROM `audit`;") != std::string::npos, "mysql generate route");
    CHECK(g2.find("DELETE FROM `audit`;") != std::string::npos, "mariadb generate route");
    PASS();
}

void test_parser_to_generator_mysql_flow() {
    TEST(parser_to_generator_mysql_flow);
    auto mod = MySQLParser::parseMySQL(
        "CREATE TABLE `events` (`id` INT AUTO_INCREMENT, `name` VARCHAR(255) NOT NULL) ENGINE=InnoDB;\n"
        "SELECT id FROM `events` LIMIT 3;\n");
    CHECK(mod != nullptr, "module null");
    MySQLGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("CREATE TABLE `events`") != std::string::npos, "expected create table");
    CHECK(out.find("ENGINE=InnoDB") != std::string::npos, "expected engine");
    CHECK(out.find("LIMIT 3") != std::string::npos, "expected limit");
    PASS();
}

int main() {
    std::cout << "Step 414: MySQL Parser + Generator Tests\n";

    test_parse_create_table_auto_increment_engine(); // 1
    test_parse_select_limit();                       // 2
    test_parse_group_concat_projection();            // 3
    test_parse_insert_update_delete();               // 4
    test_parse_with_diagnostics_entrypoint();        // 5
    test_generate_create_table_backticks_engine();   // 6
    test_generate_select_limit();                    // 7
    test_generate_group_concat();                    // 8
    test_generate_insert_with_backticks();           // 9
    test_generate_update_delete_with_backticks();    // 10
    test_pipeline_parse_and_generate_aliases();      // 11
    test_parser_to_generator_mysql_flow();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
