// Step 505: MCP Tool Documentation Tests (12 tests)

#include "MCPToolDocumentation.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static MCPDocumentationResource buildDocs() {
    return MCPToolDocumentation::buildDefault();
}

void test_default_docs_include_multiple_tools() {
    TEST(default_docs_include_multiple_tools);
    auto docs = buildDocs();
    CHECK(docs.tools.size() >= 5, "expected at least 5 documented tools");
    PASS();
}

void test_each_tool_has_required_documentation_sections() {
    TEST(each_tool_has_required_documentation_sections);
    auto docs = buildDocs();
    for (const auto& t : docs.tools) {
        CHECK(t.isComplete(), "tool doc is incomplete");
    }
    PASS();
}

void test_completeness_validator_passes_for_default_docs() {
    TEST(completeness_validator_passes_for_default_docs);
    auto docs = buildDocs();
    std::string err;
    CHECK(MCPToolDocumentation::validateCompleteness(docs, &err), err.c_str());
    PASS();
}

void test_required_categories_are_present() {
    TEST(required_categories_are_present);
    auto docs = buildDocs();
    CHECK(docs.categories.count("AST") == 1, "missing AST category");
    CHECK(docs.categories.count("Diagnostics") == 1, "missing Diagnostics category");
    CHECK(docs.categories.count("Workflow") == 1, "missing Workflow category");
    CHECK(docs.categories.count("Routing") == 1, "missing Routing category");
    CHECK(docs.categories.count("Security") == 1, "missing Security category");
    PASS();
}

void test_error_codes_are_documented_for_all_tools() {
    TEST(error_codes_are_documented_for_all_tools);
    auto docs = buildDocs();
    for (const auto& t : docs.tools) {
        CHECK(!t.errorCodes.empty(), "missing error codes");
        CHECK(!t.errorCodes[0].code.empty(), "error code missing code");
        CHECK(!t.errorCodes[0].meaning.empty(), "error code missing meaning");
    }
    PASS();
}

void test_markdown_export_contains_tool_names_and_sections() {
    TEST(markdown_export_contains_tool_names_and_sections);
    auto docs = buildDocs();
    auto md = MCPToolDocumentation::toMarkdown(docs);
    CHECK(md.find("# MCP Tool Documentation") != std::string::npos, "missing markdown header");
    CHECK(md.find("whetstone_parse_ast") != std::string::npos, "missing AST tool section");
    CHECK(md.find("Errors:") != std::string::npos, "missing error section");
    PASS();
}

void test_json_export_contains_tool_entries() {
    TEST(json_export_contains_tool_entries);
    auto docs = buildDocs();
    auto js = MCPToolDocumentation::toJson(docs);
    CHECK(js.find("\"tools\"") != std::string::npos, "missing tools array");
    CHECK(js.find("\"name\": \"whetstone_security_audit\"") != std::string::npos,
          "missing security tool entry");
    PASS();
}

void test_resource_write_materializes_markdown_and_json_files() {
    TEST(resource_write_materializes_markdown_and_json_files);
    auto docs = buildDocs();
    fs::path root = fs::temp_directory_path() / "whetstone_step505_docs";
    fs::remove_all(root);

    std::string mdPath, jsPath;
    CHECK(MCPToolDocumentation::writeResource(docs, root.string(), &mdPath, &jsPath),
          "resource write failed");
    CHECK(fs::exists(mdPath), "markdown resource file missing");
    CHECK(fs::exists(jsPath), "json resource file missing");

    fs::remove_all(root);
    PASS();
}

void test_written_markdown_resource_is_readable_by_client() {
    TEST(written_markdown_resource_is_readable_by_client);
    auto docs = buildDocs();
    fs::path root = fs::temp_directory_path() / "whetstone_step505_read";
    fs::remove_all(root);

    std::string mdPath;
    CHECK(MCPToolDocumentation::writeResource(docs, root.string(), &mdPath, nullptr),
          "resource write failed");
    std::ifstream in(mdPath);
    CHECK(in.good(), "markdown resource not readable");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(content.find("whetstone_create_workflow") != std::string::npos,
          "workflow tool missing from markdown resource");

    fs::remove_all(root);
    PASS();
}

void test_invalid_docs_are_rejected_by_completeness_validator() {
    TEST(invalid_docs_are_rejected_by_completeness_validator);
    MCPDocumentationResource docs;
    docs.tools.push_back({"broken_tool", "", "", "", "", "", "", {}});
    std::string err;
    CHECK(!MCPToolDocumentation::validateCompleteness(docs, &err), "invalid docs should fail");
    CHECK(!err.empty(), "expected error message for invalid docs");
    PASS();
}

void test_category_index_points_to_documented_tools() {
    TEST(category_index_points_to_documented_tools);
    auto docs = buildDocs();
    CHECK(!docs.categories["AST"].empty(), "AST category index empty");
    CHECK(!docs.categories["Workflow"].empty(), "Workflow category index empty");
    PASS();
}

void test_notes_include_generation_summary() {
    TEST(notes_include_generation_summary);
    auto docs = buildDocs();
    CHECK(!docs.notes.empty(), "missing documentation notes");
    CHECK(docs.notes[0].find("generated") != std::string::npos, "missing generation summary note");
    PASS();
}

int main() {
    std::cout << "Step 505: MCP Tool Documentation Tests\n";

    test_default_docs_include_multiple_tools();                          // 1
    test_each_tool_has_required_documentation_sections();                // 2
    test_completeness_validator_passes_for_default_docs();               // 3
    test_required_categories_are_present();                              // 4
    test_error_codes_are_documented_for_all_tools();                     // 5
    test_markdown_export_contains_tool_names_and_sections();             // 6
    test_json_export_contains_tool_entries();                            // 7
    test_resource_write_materializes_markdown_and_json_files();          // 8
    test_written_markdown_resource_is_readable_by_client();              // 9
    test_invalid_docs_are_rejected_by_completeness_validator();          // 10
    test_category_index_points_to_documented_tools();                    // 11
    test_notes_include_generation_summary();                             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
