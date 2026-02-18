// Step 639: whetstone_generate_project MCP tool (12 tests)

#include "ProjectSkeletonGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_tool_name_matches_spec() {
    TEST(tool_name_matches_spec);
    CHECK(ProjectSkeletonGenerator::toolName() == "whetstone_generate_project", "tool name mismatch");
    PASS();
}

void test_generate_rejects_empty_name() {
    TEST(generate_rejects_empty_name);
    auto out = ProjectSkeletonGenerator::generate("", "desc", {});
    CHECK(!out.success, "empty name should fail");
    CHECK(!out.errors.empty(), "error expected");
    PASS();
}

void test_generate_sets_project_name() {
    TEST(generate_sets_project_name);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    CHECK(out.success, "generation should succeed");
    CHECK(out.name == "drone", "name mismatch");
    PASS();
}

void test_generate_emits_cmake_project_line() {
    TEST(generate_emits_cmake_project_line);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    CHECK(out.cmakeLists.find("project(drone") != std::string::npos, "project line missing");
    PASS();
}

void test_generate_emits_executable_target() {
    TEST(generate_emits_executable_target);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    CHECK(out.cmakeLists.find("add_executable(drone") != std::string::npos, "target line missing");
    PASS();
}

void test_generate_emits_find_package_for_dependencies() {
    TEST(generate_emits_find_package_for_dependencies);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {"nlohmann_json"});
    CHECK(out.cmakeLists.find("find_package(nlohmann_json CONFIG REQUIRED)") != std::string::npos,
          "dependency line missing");
    PASS();
}

void test_generate_emits_main_cpp_with_description() {
    TEST(generate_emits_main_cpp_with_description);
    auto out = ProjectSkeletonGenerator::generate("drone", "HiveMind bootstrap", {});
    CHECK(out.mainCpp.find("HiveMind bootstrap") != std::string::npos, "description missing in main");
    PASS();
}

void test_generate_includes_module_headers() {
    TEST(generate_includes_module_headers);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    CHECK(out.moduleHeaders.size() == 2, "module header count mismatch");
    PASS();
}

void test_as_tool_response_includes_success_flag() {
    TEST(as_tool_response_includes_success_flag);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    auto response = ProjectSkeletonGenerator::asToolResponse(out);
    CHECK(response.value("success", false), "success flag missing");
    PASS();
}

void test_as_tool_response_includes_cmake_text() {
    TEST(as_tool_response_includes_cmake_text);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    auto response = ProjectSkeletonGenerator::asToolResponse(out);
    CHECK(response.value("cmakeLists", "").find("cmake_minimum_required") != std::string::npos,
          "cmake text missing");
    PASS();
}

void test_as_tool_response_includes_dependencies() {
    TEST(as_tool_response_includes_dependencies);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {"sqlite3"});
    auto response = ProjectSkeletonGenerator::asToolResponse(out);
    CHECK(response["dependencies"].size() == 1, "dependency count mismatch");
    PASS();
}

void test_generate_handles_empty_dependencies_edge_case() {
    TEST(generate_handles_empty_dependencies_edge_case);
    auto out = ProjectSkeletonGenerator::generate("drone", "desc", {});
    CHECK(out.dependencies.empty(), "dependencies should be empty");
    PASS();
}

int main() {
    std::cout << "Step 639: whetstone_generate_project MCP tool\n";

    test_tool_name_matches_spec();
    test_generate_rejects_empty_name();
    test_generate_sets_project_name();
    test_generate_emits_cmake_project_line();
    test_generate_emits_executable_target();
    test_generate_emits_find_package_for_dependencies();
    test_generate_emits_main_cpp_with_description();
    test_generate_includes_module_headers();
    test_as_tool_response_includes_success_flag();
    test_as_tool_response_includes_cmake_text();
    test_as_tool_response_includes_dependencies();
    test_generate_handles_empty_dependencies_edge_case();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
