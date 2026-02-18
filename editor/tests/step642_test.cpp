// Step 642: Job dispatch table generator (12 tests)

#include "JobDispatchTableGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static std::vector<JobDispatchEntrySpec> sampleEntries() {
    return {
        {"compile", {"cpp", "build"}, "CompilePayload", "runCompile"},
        {"test", {"cpp", "test"}, "TestPayload", "runTests"}
    };
}

void test_generate_rejects_empty_entries() {
    TEST(generate_rejects_empty_entries);
    auto out = JobDispatchTableGenerator::generate({});
    CHECK(!out.success, "empty entries should fail");
    PASS();
}

void test_generate_succeeds_with_entries() {
    TEST(generate_succeeds_with_entries);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.success, "generation should succeed");
    PASS();
}

void test_header_contains_executor_alias() {
    TEST(header_contains_executor_alias);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("using ExecutorFn") != std::string::npos, "ExecutorFn alias missing");
    PASS();
}

void test_header_contains_dispatch_table_factory() {
    TEST(header_contains_dispatch_table_factory);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("makeDispatchTable") != std::string::npos, "factory missing");
    PASS();
}

void test_header_contains_compile_job_entry() {
    TEST(header_contains_compile_job_entry);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("table[\"compile\"] = runCompile") != std::string::npos, "compile entry missing");
    PASS();
}

void test_header_contains_test_job_entry() {
    TEST(header_contains_test_job_entry);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("table[\"test\"] = runTests") != std::string::npos, "test entry missing");
    PASS();
}

void test_header_contains_payload_marker_for_compile() {
    TEST(header_contains_payload_marker_for_compile);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("payload=CompilePayload") != std::string::npos, "compile payload marker missing");
    PASS();
}

void test_header_contains_caps_marker_for_compile() {
    TEST(header_contains_caps_marker_for_compile);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("caps=cpp,build") != std::string::npos, "compile caps marker missing");
    PASS();
}

void test_header_contains_caps_marker_for_test() {
    TEST(header_contains_caps_marker_for_test);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("caps=cpp,test") != std::string::npos, "test caps marker missing");
    PASS();
}

void test_header_is_header_only() {
    TEST(header_is_header_only);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    CHECK(out.headerCode.find("#pragma once") != std::string::npos, "pragma missing");
    PASS();
}

void test_generate_handles_entry_with_no_caps() {
    TEST(generate_handles_entry_with_no_caps);
    auto out = JobDispatchTableGenerator::generate({{"lint", {}, "LintPayload", "runLint"}});
    CHECK(out.headerCode.find("caps=none") != std::string::npos, "none caps marker missing");
    PASS();
}

void test_entry_order_is_preserved() {
    TEST(entry_order_is_preserved);
    auto out = JobDispatchTableGenerator::generate(sampleEntries());
    std::size_t compilePos = out.headerCode.find("table[\"compile\"]");
    std::size_t testPos = out.headerCode.find("table[\"test\"]");
    CHECK(compilePos != std::string::npos && testPos != std::string::npos && compilePos < testPos,
          "entry order mismatch");
    PASS();
}

int main() {
    std::cout << "Step 642: Job dispatch table generator\n";

    test_generate_rejects_empty_entries();
    test_generate_succeeds_with_entries();
    test_header_contains_executor_alias();
    test_header_contains_dispatch_table_factory();
    test_header_contains_compile_job_entry();
    test_header_contains_test_job_entry();
    test_header_contains_payload_marker_for_compile();
    test_header_contains_caps_marker_for_compile();
    test_header_contains_caps_marker_for_test();
    test_header_is_header_only();
    test_generate_handles_entry_with_no_caps();
    test_entry_order_is_preserved();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
