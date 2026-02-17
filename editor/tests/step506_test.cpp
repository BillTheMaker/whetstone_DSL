// Step 506: Edge Case Cleanup Tests (12 tests)

#include "EdgeCaseCleanup.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_empty_file_returns_meaningful_error() {
    TEST(empty_file_returns_meaningful_error);
    auto r = EdgeCaseCleanup::inspectFileContent("");
    CHECK(!r.ok, "empty file should fail");
    CHECK(r.code == "E_EMPTY_FILE", "unexpected empty-file code");
    PASS();
}

void test_binary_file_returns_meaningful_error() {
    TEST(binary_file_returns_meaningful_error);
    std::string s = "abc";
    s.push_back('\0');
    s += "def";
    auto r = EdgeCaseCleanup::inspectFileContent(s);
    CHECK(!r.ok, "binary file should fail");
    CHECK(r.code == "E_BINARY_FILE", "unexpected binary-file code");
    PASS();
}

void test_large_file_returns_meaningful_error() {
    TEST(large_file_returns_meaningful_error);
    std::string s(2048, 'x');
    auto r = EdgeCaseCleanup::inspectFileContent(s, 1024);
    CHECK(!r.ok, "large file should fail");
    CHECK(r.code == "E_FILE_TOO_LARGE", "unexpected large-file code");
    PASS();
}

void test_malformed_annotations_return_meaningful_error() {
    TEST(malformed_annotations_return_meaningful_error);
    auto r = EdgeCaseCleanup::validateAnnotations({"Intent(no_prefix)"});
    CHECK(!r.ok, "malformed annotation should fail");
    CHECK(r.code == "E_MALFORMED_ANNOTATION", "unexpected annotation error code");
    PASS();
}

void test_circular_dependencies_return_meaningful_error() {
    TEST(circular_dependencies_return_meaningful_error);
    std::map<std::string, std::vector<std::string>> g = {
        {"a", {"b"}}, {"b", {"c"}}, {"c", {"a"}}
    };
    auto r = EdgeCaseCleanup::detectCircularDependencies(g);
    CHECK(!r.ok, "circular dependency should fail");
    CHECK(r.code == "E_CIRCULAR_DEPENDENCY", "unexpected circular dependency code");
    PASS();
}

void test_concurrent_workflow_modifications_return_conflict_error() {
    TEST(concurrent_workflow_modifications_return_conflict_error);
    auto r = EdgeCaseCleanup::checkConcurrentModification(5, 6);
    CHECK(!r.ok, "concurrent modification should fail");
    CHECK(r.code == "E_CONCURRENT_MODIFICATION", "unexpected conflict error code");
    PASS();
}

void test_interrupted_workflow_can_save_and_resume() {
    TEST(interrupted_workflow_can_save_and_resume);
    fs::path p = fs::temp_directory_path() / "whetstone_step506" / "wf.json";
    fs::remove_all(p.parent_path());

    auto save = EdgeCaseCleanup::saveInterruptedWorkflow(p.string(), "{\"state\":\"paused\"}");
    CHECK(save.ok, "save should succeed");

    std::string resumed;
    auto resume = EdgeCaseCleanup::resumeInterruptedWorkflow(p.string(), &resumed);
    CHECK(resume.ok, "resume should succeed");
    CHECK(resumed.find("paused") != std::string::npos, "resumed state mismatch");

    fs::remove_all(p.parent_path());
    PASS();
}

void test_resume_missing_workflow_returns_meaningful_error() {
    TEST(resume_missing_workflow_returns_meaningful_error);
    fs::path p = fs::temp_directory_path() / "whetstone_step506_missing" / "wf.json";
    fs::remove_all(p.parent_path());
    auto r = EdgeCaseCleanup::resumeInterruptedWorkflow(p.string(), nullptr);
    CHECK(!r.ok, "resume should fail for missing file");
    CHECK(r.code == "E_WORKFLOW_RESUME_FAILED", "unexpected resume error code");
    PASS();
}

void test_unknown_language_falls_back_to_plain_text() {
    TEST(unknown_language_falls_back_to_plain_text);
    auto lang = EdgeCaseCleanup::normalizeLanguage("kotlin");
    CHECK(lang == "plain_text", "unknown language should fallback to plain_text");
    PASS();
}

void test_mcp_malformed_request_returns_meaningful_error() {
    TEST(mcp_malformed_request_returns_meaningful_error);
    auto r = EdgeCaseCleanup::validateMcpRequest("[]");
    CHECK(!r.ok, "malformed request should fail");
    CHECK(r.code == "E_MCP_MALFORMED_REQUEST", "unexpected MCP error code");
    PASS();
}

void test_valid_input_path_returns_ok_without_crash() {
    TEST(valid_input_path_returns_ok_without_crash);
    auto fileCheck = EdgeCaseCleanup::inspectFileContent("int main(){}");
    auto annCheck = EdgeCaseCleanup::validateAnnotations({"@Intent(parse)"});
    auto reqCheck = EdgeCaseCleanup::validateMcpRequest("{\"method\":\"ping\"}");
    CHECK(fileCheck.ok && annCheck.ok && reqCheck.ok, "valid path should succeed");
    PASS();
}

void test_all_error_paths_include_nonempty_messages() {
    TEST(all_error_paths_include_nonempty_messages);
    auto a = EdgeCaseCleanup::inspectFileContent("");
    auto b = EdgeCaseCleanup::validateMcpRequest("");
    CHECK(!a.message.empty(), "empty-file message missing");
    CHECK(!b.message.empty(), "mcp-error message missing");
    PASS();
}

int main() {
    std::cout << "Step 506: Edge Case Cleanup Tests\n";

    test_empty_file_returns_meaningful_error();                         // 1
    test_binary_file_returns_meaningful_error();                        // 2
    test_large_file_returns_meaningful_error();                         // 3
    test_malformed_annotations_return_meaningful_error();               // 4
    test_circular_dependencies_return_meaningful_error();               // 5
    test_concurrent_workflow_modifications_return_conflict_error();     // 6
    test_interrupted_workflow_can_save_and_resume();                    // 7
    test_resume_missing_workflow_returns_meaningful_error();            // 8
    test_unknown_language_falls_back_to_plain_text();                   // 9
    test_mcp_malformed_request_returns_meaningful_error();              // 10
    test_valid_input_path_returns_ok_without_crash();                   // 11
    test_all_error_paths_include_nonempty_messages();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
