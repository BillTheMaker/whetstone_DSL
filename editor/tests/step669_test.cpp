// Step 669: wire whetstone_generate_project (12 tests)

#include "ProjectSkeletonGenerator.h"
#include "MCPServer.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json callTool(MCPServer& mcp, const std::string& name, const json& args) {
    json req = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "tools/call"},
        {"params", {{"name", name}, {"arguments", args}}}
    };
    json resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return json::parse(text);
}

void t1() {
    TEST(name_required);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"description", "x"}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t2() {
    TEST(description_required);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t3() {
    TEST(valid_inputs_succeed);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    CHECK(out.value("success", false), "expected success");
    PASS();
}

void t4() {
    TEST(cmake_non_empty_on_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(!out.value("cmakeLists", "").empty(), "cmakeLists should be non-empty");
    PASS();
}

void t5() {
    TEST(maincpp_non_empty_on_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(!out.value("mainCpp", "").empty(), "mainCpp should be non-empty");
    PASS();
}

void t6() {
    TEST(module_headers_non_empty_on_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out.contains("moduleHeaders"), "moduleHeaders missing");
    CHECK(!out["moduleHeaders"].empty(), "moduleHeaders should be non-empty");
    PASS();
}

void t7() {
    TEST(single_dependency_in_cmake);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project",
                        {{"name", "MyApp"}, {"description", "Example app"}, {"dependencies", json::array({"nlohmann_json"})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out.value("cmakeLists", "").find("find_package(nlohmann_json") != std::string::npos,
          "dependency not in CMakeLists");
    PASS();
}

void t8() {
    TEST(multiple_dependencies_in_cmake);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project",
                        {{"name", "MyApp"}, {"description", "Example app"},
                         {"dependencies", json::array({"nlohmann_json", "SQLite3"})}});
    CHECK(out.value("success", false), "expected success");
    std::string cmake = out.value("cmakeLists", "");
    CHECK(cmake.find("find_package(nlohmann_json") != std::string::npos, "missing nlohmann_json");
    CHECK(cmake.find("find_package(SQLite3") != std::string::npos, "missing SQLite3");
    PASS();
}

void t9() {
    TEST(tool_name_matches);
    CHECK(ProjectSkeletonGenerator::toolName() == "whetstone_generate_project", "wrong tool name");
    PASS();
}

void t10() {
    TEST(success_false_on_empty_name);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", ""}, {"description", "Example app"}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t11() {
    TEST(errors_non_empty_on_failure);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", ""}, {"description", "Example app"}});
    CHECK(out.contains("errors"), "errors missing");
    CHECK(out["errors"].is_array(), "errors should be array");
    CHECK(!out["errors"].empty(), "errors should be non-empty");
    PASS();
}

void t12() {
    TEST(success_true_on_minimal_valid);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    CHECK(out.value("success", false), "expected success");
    PASS();
}

int main() {
    std::cout << "Step 669: whetstone_generate_project wiring\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
