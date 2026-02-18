// Step 620: Config auto-discovery (walk up from CWD) (12 tests)

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
    std::filesystem::path root = std::filesystem::temp_directory_path() / "whetstone_step620";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

static void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path);
    out << content;
}

void test_discover_from_exact_directory() {
    TEST(discover_from_exact_directory);
    auto root = tempRoot() / "exact";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", R"({"defaultLanguage":"go"})");
    auto result = MCPProjectConfig::discoverFromCwd(root.string());
    CHECK(result.found, "config should be found");
    CHECK(result.sourceWorkspaceRoot == root.string(), "source workspace mismatch");
    CHECK(result.config.defaultLanguage == "go", "language mismatch");
    PASS();
}

void test_discover_walks_to_parent() {
    TEST(discover_walks_to_parent);
    auto root = tempRoot() / "parent";
    auto child = root / "src" / "feature";
    std::filesystem::create_directories(child);
    writeFile(root / ".whetstone.json", R"({"defaultLanguage":"rust"})");
    auto result = MCPProjectConfig::discoverFromCwd(child.string());
    CHECK(result.found, "config should be found");
    CHECK(result.sourceWorkspaceRoot == root.string(), "should resolve parent workspace");
    PASS();
}

void test_discover_prefers_nearest_config() {
    TEST(discover_prefers_nearest_config);
    auto root = tempRoot() / "nearest";
    auto mid = root / "apps";
    auto leaf = mid / "api";
    std::filesystem::create_directories(leaf);
    writeFile(root / ".whetstone.json", R"({"mcpWorkspaceAlias":"root"})");
    writeFile(mid / ".whetstone.json", R"({"mcpWorkspaceAlias":"mid"})");
    auto result = MCPProjectConfig::discoverFromCwd(leaf.string());
    CHECK(result.found, "config should be found");
    CHECK(result.sourceWorkspaceRoot == mid.string(), "should choose nearest parent");
    CHECK(result.config.mcpWorkspaceAlias == "mid", "alias should come from nearest file");
    PASS();
}

void test_discover_returns_not_found_when_missing() {
    TEST(discover_returns_not_found_when_missing);
    auto root = tempRoot() / "missing";
    auto child = root / "sub";
    std::filesystem::create_directories(child);
    auto result = MCPProjectConfig::discoverFromCwd(child.string());
    CHECK(!result.found, "should not find config");
    CHECK(result.error.empty(), "error should be empty");
    PASS();
}

void test_discover_reports_invalid_start_directory() {
    TEST(discover_reports_invalid_start_directory);
    auto root = tempRoot() / "nope" / "does-not-exist";
    auto result = MCPProjectConfig::discoverFromCwd(root.string());
    CHECK(!result.found, "should not find config");
    CHECK(result.error == "start_dir_not_found", "wrong error");
    PASS();
}

void test_discover_reports_parse_error_from_nearest_file() {
    TEST(discover_reports_parse_error_from_nearest_file);
    auto root = tempRoot() / "bad";
    auto child = root / "src";
    std::filesystem::create_directories(child);
    writeFile(root / ".whetstone.json", "{bad");
    auto result = MCPProjectConfig::discoverFromCwd(child.string());
    CHECK(result.found, "file exists so found should be true");
    CHECK(result.error == "config_json_parse_failed", "wrong error");
    PASS();
}

void test_discover_parses_workspace_override() {
    TEST(discover_parses_workspace_override);
    auto root = tempRoot() / "workspace-override";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", R"({"workspace":"/opt/project"})");
    auto result = MCPProjectConfig::discoverFromCwd(root.string());
    CHECK(result.found, "should find config");
    CHECK(result.config.workspace == "/opt/project", "workspace override mismatch");
    PASS();
}

void test_discover_parses_agent_role() {
    TEST(discover_parses_agent_role);
    auto root = tempRoot() / "role";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", R"({"agentRole":"generator"})");
    auto result = MCPProjectConfig::discoverFromCwd(root.string());
    CHECK(result.found, "should find config");
    CHECK(result.config.agentRole == "generator", "agent role mismatch");
    PASS();
}

void test_discover_parses_workspace_alias() {
    TEST(discover_parses_workspace_alias);
    auto root = tempRoot() / "alias";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", R"({"mcpWorkspaceAlias":"payments"})");
    auto result = MCPProjectConfig::discoverFromCwd(root.string());
    CHECK(result.found, "should find config");
    CHECK(result.config.mcpWorkspaceAlias == "payments", "alias mismatch");
    PASS();
}

void test_discover_with_no_argument_uses_current_path() {
    TEST(discover_with_no_argument_uses_current_path);
    auto root = tempRoot() / "cwd-default";
    std::filesystem::create_directories(root);
    writeFile(root / ".whetstone.json", R"({"defaultLanguage":"cpp"})");

    auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    auto result = MCPProjectConfig::discoverFromCwd();
    std::filesystem::current_path(previous);

    CHECK(result.found, "should find config from cwd");
    CHECK(result.config.defaultLanguage == "cpp", "language mismatch");
    PASS();
}

void test_discover_ignores_grandparent_when_parent_has_invalid_file() {
    TEST(discover_ignores_grandparent_when_parent_has_invalid_file);
    auto root = tempRoot() / "invalid-nearest";
    auto parent = root / "service";
    auto leaf = parent / "api";
    std::filesystem::create_directories(leaf);
    writeFile(root / ".whetstone.json", R"({"defaultLanguage":"python"})");
    writeFile(parent / ".whetstone.json", "{bad");
    auto result = MCPProjectConfig::discoverFromCwd(leaf.string());
    CHECK(result.found, "nearest file exists");
    CHECK(result.error == "config_json_parse_failed", "should fail on nearest invalid file");
    PASS();
}

void test_discover_preserves_source_workspace_root_for_nested_start() {
    TEST(discover_preserves_source_workspace_root_for_nested_start);
    auto root = tempRoot() / "source-root";
    auto nested = root / "a" / "b" / "c";
    std::filesystem::create_directories(nested);
    writeFile(root / ".whetstone.json", R"({"defaultLanguage":"typescript"})");
    auto result = MCPProjectConfig::discoverFromCwd(nested.string());
    CHECK(result.found, "should find config");
    CHECK(result.sourceWorkspaceRoot == root.string(), "source root mismatch");
    PASS();
}

int main() {
    std::cout << "Step 620: Config auto-discovery walk-up\n";

    test_discover_from_exact_directory();                         // 1
    test_discover_walks_to_parent();                              // 2
    test_discover_prefers_nearest_config();                       // 3
    test_discover_returns_not_found_when_missing();               // 4
    test_discover_reports_invalid_start_directory();              // 5
    test_discover_reports_parse_error_from_nearest_file();        // 6
    test_discover_parses_workspace_override();                    // 7
    test_discover_parses_agent_role();                            // 8
    test_discover_parses_workspace_alias();                       // 9
    test_discover_with_no_argument_uses_current_path();           // 10
    test_discover_ignores_grandparent_when_parent_has_invalid_file();// 11
    test_discover_preserves_source_workspace_root_for_nested_start();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
