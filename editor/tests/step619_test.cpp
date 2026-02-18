// Step 619: .whetstone.json per-project config schema (12 tests)

#include "MCPProjectConfig.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::filesystem::path tempRoot() {
    std::filesystem::path root = std::filesystem::temp_directory_path() / "whetstone_step619";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

static void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path);
    out << content;
}

void test_parse_valid_all_fields() {
    TEST(parse_valid_all_fields);
    MCPProjectConfigData config;
    std::string error;
    std::string text =
        R"({"workspace":"/tmp/ws","defaultLanguage":"rust","agentRole":"generator","mcpWorkspaceAlias":"alpha"})";
    CHECK(MCPProjectConfig::parseText(text, &config, &error), "parse should succeed");
    CHECK(config.workspace == "/tmp/ws", "workspace mismatch");
    CHECK(config.defaultLanguage == "rust", "language mismatch");
    CHECK(config.agentRole == "generator", "role mismatch");
    CHECK(config.mcpWorkspaceAlias == "alpha", "alias mismatch");
    PASS();
}

void test_parse_valid_partial_fields() {
    TEST(parse_valid_partial_fields);
    MCPProjectConfigData config;
    std::string error;
    CHECK(MCPProjectConfig::parseText(R"({"defaultLanguage":"python"})", &config, &error),
          "parse should succeed");
    CHECK(config.defaultLanguage == "python", "language mismatch");
    CHECK(config.workspace.empty(), "workspace should be empty");
    PASS();
}

void test_parse_empty_text_fails() {
    TEST(parse_empty_text_fails);
    MCPProjectConfigData config;
    std::string error;
    CHECK(!MCPProjectConfig::parseText("", &config, &error), "parse should fail");
    CHECK(error == "config_empty", "wrong error");
    PASS();
}

void test_parse_invalid_json_fails() {
    TEST(parse_invalid_json_fails);
    MCPProjectConfigData config;
    std::string error;
    CHECK(!MCPProjectConfig::parseText("{bad", &config, &error), "parse should fail");
    CHECK(error == "config_json_parse_failed", "wrong error");
    PASS();
}

void test_parse_non_object_fails() {
    TEST(parse_non_object_fails);
    MCPProjectConfigData config;
    std::string error;
    CHECK(!MCPProjectConfig::parseText(R"(["bad"])", &config, &error), "parse should fail");
    CHECK(error == "config_not_object", "wrong error");
    PASS();
}

void test_parse_invalid_workspace_type_fails() {
    TEST(parse_invalid_workspace_type_fails);
    MCPProjectConfigData config;
    std::string error;
    CHECK(!MCPProjectConfig::parseText(R"({"workspace":42})", &config, &error), "parse should fail");
    CHECK(error == "config_field_invalid_type:workspace", "wrong error");
    PASS();
}

void test_parse_invalid_default_language_type_fails() {
    TEST(parse_invalid_default_language_type_fails);
    MCPProjectConfigData config;
    std::string error;
    CHECK(!MCPProjectConfig::parseText(R"({"defaultLanguage":42})", &config, &error), "parse should fail");
    CHECK(error == "config_field_invalid_type:defaultLanguage", "wrong error");
    PASS();
}

void test_load_from_workspace_missing_workspace_fails() {
    TEST(load_from_workspace_missing_workspace_fails);
    auto result = MCPProjectConfig::loadFromWorkspace("");
    CHECK(!result.found, "found should be false");
    CHECK(result.error == "workspace_missing", "wrong error");
    PASS();
}

void test_load_from_workspace_without_file_returns_not_found() {
    TEST(load_from_workspace_without_file_returns_not_found);
    std::filesystem::path root = tempRoot() / "ws-empty";
    std::filesystem::create_directories(root);
    auto result = MCPProjectConfig::loadFromWorkspace(root.string());
    CHECK(!result.found, "found should be false");
    CHECK(result.error.empty(), "error should be empty");
    PASS();
}

void test_load_from_workspace_reads_valid_config() {
    TEST(load_from_workspace_reads_valid_config);
    std::filesystem::path root = tempRoot() / "ws-valid";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json",
              R"({"workspace":"/tmp/alt","defaultLanguage":"go","agentRole":"refactor","mcpWorkspaceAlias":"beta"})");
    auto result = MCPProjectConfig::loadFromWorkspace(root.string());
    CHECK(result.found, "found should be true");
    CHECK(result.error.empty(), "error should be empty");
    CHECK(result.config.defaultLanguage == "go", "language mismatch");
    CHECK(result.config.mcpWorkspaceAlias == "beta", "alias mismatch");
    PASS();
}

void test_load_from_workspace_reports_parse_error() {
    TEST(load_from_workspace_reports_parse_error);
    std::filesystem::path root = tempRoot() / "ws-bad";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", "{bad");
    auto result = MCPProjectConfig::loadFromWorkspace(root.string());
    CHECK(result.found, "found should be true when file exists");
    CHECK(result.error == "config_json_parse_failed", "wrong error");
    PASS();
}

void test_parse_includes_agent_role_and_alias_fields() {
    TEST(parse_includes_agent_role_and_alias_fields);
    MCPProjectConfigData config;
    std::string error;
    CHECK(MCPProjectConfig::parseText(R"({"agentRole":"linter","mcpWorkspaceAlias":"gamma"})", &config, &error),
          "parse should succeed");
    CHECK(config.agentRole == "linter", "agentRole mismatch");
    CHECK(config.mcpWorkspaceAlias == "gamma", "alias mismatch");
    PASS();
}

int main() {
    std::cout << "Step 619: .whetstone.json per-project config schema\n";

    test_parse_valid_all_fields();                              // 1
    test_parse_valid_partial_fields();                          // 2
    test_parse_empty_text_fails();                              // 3
    test_parse_invalid_json_fails();                            // 4
    test_parse_non_object_fails();                              // 5
    test_parse_invalid_workspace_type_fails();                  // 6
    test_parse_invalid_default_language_type_fails();           // 7
    test_load_from_workspace_missing_workspace_fails();         // 8
    test_load_from_workspace_without_file_returns_not_found();  // 9
    test_load_from_workspace_reads_valid_config();              // 10
    test_load_from_workspace_reports_parse_error();             // 11
    test_parse_includes_agent_role_and_alias_fields();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
