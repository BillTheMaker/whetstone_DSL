// Step 417: MCP Server Configuration + Discovery Tests (12 tests)

#include "MCPServerConfig.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string makeTempDir(const std::string& name) {
    auto dir = std::filesystem::temp_directory_path() / ("whetstone_" + name);
    std::filesystem::create_directories(dir);
    return dir.string();
}

void test_resolve_binary_path_from_candidate() {
    TEST(resolve_binary_path_from_candidate);
    std::string path = MCPServerConfig::resolveBinaryPath({"editor/build-native/whetstone_mcp"});
    CHECK(!path.empty(), "expected binary path from explicit candidate");
    PASS();
}

void test_resolve_binary_path_from_defaults() {
    TEST(resolve_binary_path_from_defaults);
    std::string path = MCPServerConfig::resolveBinaryPath();
    CHECK(!path.empty(), "expected binary path from default discovery");
    PASS();
}

void test_detect_workspace_root_from_repo() {
    TEST(detect_workspace_root_from_repo);
    std::string root = MCPServerConfig::detectWorkspaceRoot(".");
    CHECK(!root.empty(), "workspace root empty");
    CHECK(std::filesystem::exists(std::filesystem::path(root) / "progress.md"),
          "expected progress.md at workspace root");
    PASS();
}

void test_detect_primary_language_cpp_majority() {
    TEST(detect_primary_language_cpp_majority);
    auto dir = std::filesystem::path(makeTempDir("lang_cpp"));
    std::ofstream(dir / "a.cpp") << "int main() {}\n";
    std::ofstream(dir / "b.cpp") << "int add(int a,int b){return a+b;}\n";
    std::ofstream(dir / "c.py") << "print('x')\n";
    std::string lang = MCPServerConfig::detectPrimaryLanguage(dir.string());
    CHECK(lang == "cpp", "expected cpp primary language");
    PASS();
}

void test_detect_primary_language_python_majority() {
    TEST(detect_primary_language_python_majority);
    auto dir = std::filesystem::path(makeTempDir("lang_py"));
    std::ofstream(dir / "a.py") << "print('a')\n";
    std::ofstream(dir / "b.py") << "print('b')\n";
    std::ofstream(dir / "c.cpp") << "int main(){}\n";
    std::string lang = MCPServerConfig::detectPrimaryLanguage(dir.string());
    CHECK(lang == "python", "expected python primary language");
    PASS();
}

void test_build_mcp_config_structure() {
    TEST(build_mcp_config_structure);
    json cfg = MCPServerConfig::buildMcpConfig("/bin/whetstone_mcp", "/repo", "cpp");
    CHECK(cfg.contains("mcpServers"), "missing mcpServers");
    CHECK(cfg["mcpServers"].contains("whetstone"), "missing whetstone server key");
    CHECK(cfg["mcpServers"]["whetstone"]["command"] == "/bin/whetstone_mcp",
          "wrong command field");
    PASS();
}

void test_build_mcp_config_args_contains_workspace_language() {
    TEST(build_mcp_config_args_contains_workspace_language);
    json cfg = MCPServerConfig::buildMcpConfig("/bin/whetstone_mcp", "/repo", "python");
    auto args = cfg["mcpServers"]["whetstone"]["args"];
    std::string dumped = args.dump();
    CHECK(dumped.find("--workspace") != std::string::npos, "missing workspace arg");
    CHECK(dumped.find("--language") != std::string::npos, "missing language arg");
    CHECK(dumped.find("python") != std::string::npos, "missing language value");
    PASS();
}

void test_write_mcp_config_file() {
    TEST(write_mcp_config_file);
    auto dir = std::filesystem::path(makeTempDir("mcp_cfg"));
    auto path = (dir / ".mcp.json").string();
    json cfg = MCPServerConfig::buildMcpConfig("/bin/whetstone_mcp", "/repo", "cpp");
    CHECK(MCPServerConfig::writeMcpConfigFile(path, cfg), "write config failed");
    CHECK(std::filesystem::exists(path), "config file not created");
    PASS();
}

void test_manifest_tool_count_at_least_68() {
    TEST(manifest_tool_count_at_least_68);
    MCPServer server;
    json manifest = MCPServerConfig::buildServerManifest(server);
    CHECK(manifest["toolCount"].get<int>() >= 68, "expected at least 68 tools");
    PASS();
}

void test_manifest_required_categories_present() {
    TEST(manifest_required_categories_present);
    MCPServer server;
    json manifest = MCPServerConfig::buildServerManifest(server);
    auto cats = manifest["categories"];
    CHECK(cats.contains("AST"), "missing AST category");
    CHECK(cats.contains("Diagnostics"), "missing Diagnostics category");
    CHECK(cats.contains("Project"), "missing Project category");
    CHECK(cats.contains("FileOps"), "missing FileOps category");
    PASS();
}

void test_categorize_tool_names() {
    TEST(categorize_tool_names);
    CHECK(MCPServerConfig::categorizeTool("whetstone_get_ast") == "AST", "ast category");
    CHECK(MCPServerConfig::categorizeTool("whetstone_get_diagnostics") == "Diagnostics",
          "diagnostics category");
    CHECK(MCPServerConfig::categorizeTool("whetstone_workspace_list") == "Project",
          "project category");
    CHECK(MCPServerConfig::categorizeTool("whetstone_file_write") == "FileOps",
          "fileops category");
    PASS();
}

void test_discover_end_to_end() {
    TEST(discover_end_to_end);
    auto d = MCPServerConfig::discover(".");
    CHECK(!d.workspaceRoot.empty(), "workspace missing");
    CHECK(!d.binaryPath.empty(), "binary path missing");
    CHECK(d.toolCount >= 68, "expected at least 68 tools in discovery");
    CHECK(!d.primaryLanguage.empty(), "primary language missing");
    PASS();
}

int main() {
    std::cout << "Step 417: MCP Server Configuration + Discovery Tests\n";

    test_resolve_binary_path_from_candidate();             // 1
    test_resolve_binary_path_from_defaults();              // 2
    test_detect_workspace_root_from_repo();                // 3
    test_detect_primary_language_cpp_majority();           // 4
    test_detect_primary_language_python_majority();        // 5
    test_build_mcp_config_structure();                     // 6
    test_build_mcp_config_args_contains_workspace_language(); // 7
    test_write_mcp_config_file();                          // 8
    test_manifest_tool_count_at_least_68();               // 9
    test_manifest_required_categories_present();           // 10
    test_categorize_tool_names();                          // 11
    test_discover_end_to_end();                            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
