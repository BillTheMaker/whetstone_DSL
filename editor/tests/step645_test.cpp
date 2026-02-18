// Step 645: CMake generator from project AST (12 tests)

#include "ProjectAstCMakeGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static ProjectAstDescription sample() {
    return {"whetstone_editor", {"src/main.cpp", "src/FileDialog.cpp"}, {"step645_test"}, {"nlohmann_json"}, true};
}

void test_generate_rejects_missing_project_name(){TEST(generate_rejects_missing_project_name);auto ast=sample();ast.projectName="";CHECK(ProjectAstCMakeGenerator::generate(ast).empty(),"missing project should fail");PASS();}
void test_generate_emits_project_line(){TEST(generate_emits_project_line);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("project(whetstone_editor")!=std::string::npos,"project line missing");PASS();}
void test_generate_emits_cxx_standard(){TEST(generate_emits_cxx_standard);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("CMAKE_CXX_STANDARD 20")!=std::string::npos,"cxx standard missing");PASS();}
void test_generate_emits_vcpkg_line(){TEST(generate_emits_vcpkg_line);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("find_package(vcpkg")!=std::string::npos,"vcpkg line missing");PASS();}
void test_generate_emits_dependency_find_package(){TEST(generate_emits_dependency_find_package);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("find_package(nlohmann_json CONFIG REQUIRED)")!=std::string::npos,"dep line missing");PASS();}
void test_generate_emits_main_target(){TEST(generate_emits_main_target);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("add_executable(whetstone_editor")!=std::string::npos,"main target missing");PASS();}
void test_generate_includes_all_sources(){TEST(generate_includes_all_sources);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("src/main.cpp")!=std::string::npos&&txt.find("src/FileDialog.cpp")!=std::string::npos,"sources missing");PASS();}
void test_generate_emits_test_targets(){TEST(generate_emits_test_targets);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("add_executable(step645_test")!=std::string::npos,"test target missing");PASS();}
void test_generate_emits_install_rules_when_enabled(){TEST(generate_emits_install_rules_when_enabled);auto txt=ProjectAstCMakeGenerator::generate(sample());CHECK(txt.find("install(TARGETS whetstone_editor")!=std::string::npos,"install missing");PASS();}
void test_generate_skips_install_rules_when_disabled(){TEST(generate_skips_install_rules_when_disabled);auto ast=sample();ast.includeInstallRules=false;auto txt=ProjectAstCMakeGenerator::generate(ast);CHECK(txt.find("install(TARGETS")==std::string::npos,"install should be absent");PASS();}
void test_generate_handles_empty_tests_edge_case(){TEST(generate_handles_empty_tests_edge_case);auto ast=sample();ast.tests.clear();auto txt=ProjectAstCMakeGenerator::generate(ast);CHECK(txt.find("tests/")==std::string::npos,"tests should be absent");PASS();}
void test_generate_handles_empty_dependencies_edge_case(){TEST(generate_handles_empty_dependencies_edge_case);auto ast=sample();ast.dependencies.clear();auto txt=ProjectAstCMakeGenerator::generate(ast);CHECK(txt.find("find_package(nlohmann_json")==std::string::npos,"dep should be absent");PASS();}

int main(){
std::cout<<"Step 645: CMake generator from project AST\n";
 test_generate_rejects_missing_project_name();test_generate_emits_project_line();test_generate_emits_cxx_standard();test_generate_emits_vcpkg_line();test_generate_emits_dependency_find_package();test_generate_emits_main_target();test_generate_includes_all_sources();test_generate_emits_test_targets();test_generate_emits_install_rules_when_enabled();test_generate_skips_install_rules_when_disabled();test_generate_handles_empty_tests_edge_case();test_generate_handles_empty_dependencies_edge_case();
std::cout<<"\nResults: "<<passed<<"/"<<(passed+failed)<<" passed\n";return failed?1:0;}
