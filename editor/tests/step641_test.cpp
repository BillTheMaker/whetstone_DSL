// Step 641: SQLite schema -> C++ data layer generator (12 tests)

#include "SQLiteDataLayerGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static std::vector<SQLiteTableSpec> sampleTables() {
    return {
        {"jobs", {"id", "status", "payload"}},
        {"executions", {"id", "job_id", "started_at", "finished_at"}}
    };
}

void test_generate_rejects_empty_tables() {
    TEST(generate_rejects_empty_tables);
    auto out = SQLiteDataLayerGenerator::generate({});
    CHECK(!out.success, "empty tables should fail");
    PASS();
}

void test_generate_succeeds_with_tables() {
    TEST(generate_succeeds_with_tables);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.success, "generation should succeed");
    PASS();
}

void test_header_contains_nexus_db_class() {
    TEST(header_contains_nexus_db_class);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.headerCode.find("class NexusDb") != std::string::npos, "NexusDb class missing");
    PASS();
}

void test_header_contains_transaction_begin() {
    TEST(header_contains_transaction_begin);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.headerCode.find("beginTx") != std::string::npos, "beginTx missing");
    PASS();
}

void test_header_contains_transaction_commit() {
    TEST(header_contains_transaction_commit);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.headerCode.find("commitTx") != std::string::npos, "commitTx missing");
    PASS();
}

void test_source_contains_begin_tx_impl() {
    TEST(source_contains_begin_tx_impl);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("NexusDb::beginTx") != std::string::npos, "beginTx impl missing");
    PASS();
}

void test_source_contains_commit_tx_impl() {
    TEST(source_contains_commit_tx_impl);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("NexusDb::commitTx") != std::string::npos, "commitTx impl missing");
    PASS();
}

void test_source_contains_jobs_table_marker() {
    TEST(source_contains_jobs_table_marker);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("table: jobs") != std::string::npos, "jobs table marker missing");
    PASS();
}

void test_source_contains_executions_table_marker() {
    TEST(source_contains_executions_table_marker);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("table: executions") != std::string::npos, "executions table marker missing");
    PASS();
}

void test_source_mentions_prepared_statements() {
    TEST(source_mentions_prepared_statements);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("prepared statements") != std::string::npos, "prepared statement marker missing");
    PASS();
}

void test_source_mentions_crud_wrappers() {
    TEST(source_mentions_crud_wrappers);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    CHECK(out.sourceCode.find("INSERT/SELECT/UPDATE/DELETE") != std::string::npos, "crud marker missing");
    PASS();
}

void test_multiple_tables_append_multiple_blocks() {
    TEST(multiple_tables_append_multiple_blocks);
    auto out = SQLiteDataLayerGenerator::generate(sampleTables());
    std::size_t first = out.sourceCode.find("table: jobs");
    std::size_t second = out.sourceCode.find("table: executions");
    CHECK(first != std::string::npos && second != std::string::npos && first < second,
          "table block ordering mismatch");
    PASS();
}

int main() {
    std::cout << "Step 641: SQLite schema -> C++ data layer generator\n";

    test_generate_rejects_empty_tables();
    test_generate_succeeds_with_tables();
    test_header_contains_nexus_db_class();
    test_header_contains_transaction_begin();
    test_header_contains_transaction_commit();
    test_source_contains_begin_tx_impl();
    test_source_contains_commit_tx_impl();
    test_source_contains_jobs_table_marker();
    test_source_contains_executions_table_marker();
    test_source_mentions_prepared_statements();
    test_source_mentions_crud_wrappers();
    test_multiple_tables_append_multiple_blocks();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
