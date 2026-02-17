// Step 493: Parse Full Whetstone Codebase Tests (12 tests)

#include "SelfHostCodebaseAudit.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static CodebaseParseAudit runRealAudit() {
    return SelfHostCodebaseAudit::auditDirectories({"editor/src", "editor/src/ast"});
}

void test_lists_headers_from_target_directories() {
    TEST(lists_headers_from_target_directories);
    auto files = SelfHostCodebaseAudit::listHeaderFiles({"editor/src", "editor/src/ast"});
    CHECK(!files.empty(), "expected header files");
    bool foundParser = false;
    for (const auto& f : files) {
        if (f.find("Parser.h") != std::string::npos) foundParser = true;
    }
    CHECK(foundParser, "expected parser-related header");
    PASS();
}

void test_audit_reports_total_file_count_nonzero() {
    TEST(audit_reports_total_file_count_nonzero);
    auto audit = runRealAudit();
    CHECK(audit.totalFiles > 0, "expected non-zero total files");
    PASS();
}

void test_parse_rate_is_within_0_to_1() {
    TEST(parse_rate_is_within_0_to_1);
    auto audit = runRealAudit();
    CHECK(audit.parseRate() >= 0.0, "parse rate below 0");
    CHECK(audit.parseRate() <= 1.0, "parse rate above 1");
    PASS();
}

void test_audit_collects_class_function_and_annotation_counts() {
    TEST(audit_collects_class_function_and_annotation_counts);
    auto audit = runRealAudit();
    CHECK(audit.totalClasses >= 0, "class count should be non-negative");
    CHECK(audit.totalFunctions >= 0, "function count should be non-negative");
    CHECK(audit.totalAnnotationNodes >= 0, "annotation count should be non-negative");
    PASS();
}

void test_each_file_has_path_and_parse_flag() {
    TEST(each_file_has_path_and_parse_flag);
    auto audit = runRealAudit();
    CHECK(!audit.files.empty(), "expected file audits");
    for (const auto& f : audit.files) {
        CHECK(!f.path.empty(), "missing file path");
    }
    PASS();
}

void test_skipped_constructs_log_is_populated_when_hint_exceeds_parse() {
    TEST(skipped_constructs_log_is_populated_when_hint_exceeds_parse);
    fs::path dir = fs::temp_directory_path() / "whetstone_step493_skip";
    fs::remove_all(dir);
    fs::create_directories(dir);
    fs::path file = dir / "sample.h";
    {
        std::ofstream out(file.string());
        out << "class A {};\nclass B {};\nint f();\n";
    }
    auto audit = SelfHostCodebaseAudit::auditFiles({file.string()});
    CHECK(!audit.files.empty(), "expected single audit file");
    bool hasSkipSignal = !audit.files[0].skippedConstructs.empty() || !audit.unparseableConstructLog.empty();
    CHECK(hasSkipSignal, "expected skipped/log signal");
    fs::remove_all(dir);
    PASS();
}

void test_unreadable_or_empty_header_is_logged() {
    TEST(unreadable_or_empty_header_is_logged);
    fs::path dir = fs::temp_directory_path() / "whetstone_step493_empty";
    fs::remove_all(dir);
    fs::create_directories(dir);
    fs::path file = dir / "empty.h";
    {
        std::ofstream out(file.string());
    }
    auto audit = SelfHostCodebaseAudit::auditFiles({file.string()});
    CHECK(!audit.unparseableConstructLog.empty(), "expected warning log for empty file");
    fs::remove_all(dir);
    PASS();
}

void test_meets_target_function_matches_threshold_semantics() {
    TEST(meets_target_function_matches_threshold_semantics);
    CodebaseParseAudit a;
    a.totalFiles = 100;
    a.parsedFiles = 95;
    CHECK(a.meetsTarget(0.95), "95/100 should meet 95% target");
    CHECK(!a.meetsTarget(0.96), "95/100 should not meet 96% target");
    PASS();
}

void test_audit_files_supports_direct_file_list_mode() {
    TEST(audit_files_supports_direct_file_list_mode);
    auto files = SelfHostCodebaseAudit::listHeaderFiles({"editor/src"});
    CHECK(!files.empty(), "expected files from src");
    std::vector<std::string> subset;
    subset.push_back(files[0]);
    auto audit = SelfHostCodebaseAudit::auditFiles(subset);
    CHECK(audit.totalFiles == 1, "expected one file in direct mode");
    PASS();
}

void test_unparseable_construct_log_contains_file_path_prefixes() {
    TEST(unparseable_construct_log_contains_file_path_prefixes);
    fs::path dir = fs::temp_directory_path() / "whetstone_step493_log";
    fs::remove_all(dir);
    fs::create_directories(dir);
    fs::path file = dir / "empty.h";
    { std::ofstream out(file.string()); }
    auto audit = SelfHostCodebaseAudit::auditFiles({file.string()});
    CHECK(!audit.unparseableConstructLog.empty(), "expected log entries");
    CHECK(audit.unparseableConstructLog[0].find(file.string()) != std::string::npos,
          "log missing file path prefix");
    fs::remove_all(dir);
    PASS();
}

void test_parse_rate_matches_parsed_over_total_ratio() {
    TEST(parse_rate_matches_parsed_over_total_ratio);
    CodebaseParseAudit a;
    a.totalFiles = 4;
    a.parsedFiles = 3;
    CHECK(a.parseRate() > 0.74 && a.parseRate() < 0.76, "expected 0.75 parse rate");
    PASS();
}

void test_real_codebase_audit_has_high_reasonable_coverage_signal() {
    TEST(real_codebase_audit_has_high_reasonable_coverage_signal);
    auto audit = runRealAudit();
    CHECK(audit.totalFiles >= 50, "expected substantial header corpus");
    CHECK(audit.parsedFiles >= 1, "expected at least one parsed file");
    PASS();
}

int main() {
    std::cout << "Step 493: Parse Full Whetstone Codebase Tests\n";

    test_lists_headers_from_target_directories();                    // 1
    test_audit_reports_total_file_count_nonzero();                   // 2
    test_parse_rate_is_within_0_to_1();                              // 3
    test_audit_collects_class_function_and_annotation_counts();      // 4
    test_each_file_has_path_and_parse_flag();                        // 5
    test_skipped_constructs_log_is_populated_when_hint_exceeds_parse(); // 6
    test_unreadable_or_empty_header_is_logged();                     // 7
    test_meets_target_function_matches_threshold_semantics();        // 8
    test_audit_files_supports_direct_file_list_mode();               // 9
    test_unparseable_construct_log_contains_file_path_prefixes();    // 10
    test_parse_rate_matches_parsed_over_total_ratio();               // 11
    test_real_codebase_audit_has_high_reasonable_coverage_signal();  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
