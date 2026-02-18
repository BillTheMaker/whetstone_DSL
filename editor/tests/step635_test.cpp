// Step 635: Capability declaration struct generator (12 tests)

#include "CapabilityDeclarationGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_generate_requires_non_empty_namespace() {
    TEST(generate_requires_non_empty_namespace);
    auto out = CapabilityDeclarationGenerator::generate("");
    CHECK(!out.success, "empty namespace should fail");
    PASS();
}

void test_generate_succeeds_with_default_namespace() {
    TEST(generate_succeeds_with_default_namespace);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.success, "generate should succeed");
    PASS();
}

void test_output_contains_node_capability_struct() {
    TEST(output_contains_node_capability_struct);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("struct NodeCapability") != std::string::npos, "NodeCapability missing");
    PASS();
}

void test_output_contains_capability_set_struct() {
    TEST(output_contains_capability_set_struct);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("struct CapabilitySet") != std::string::npos, "CapabilitySet missing");
    PASS();
}

void test_output_contains_energy_context_struct() {
    TEST(output_contains_energy_context_struct);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("struct EnergyContext") != std::string::npos, "EnergyContext missing");
    PASS();
}

void test_output_contains_job_requirements_struct() {
    TEST(output_contains_job_requirements_struct);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("struct JobRequirements") != std::string::npos, "JobRequirements missing");
    PASS();
}

void test_output_contains_supports_helper() {
    TEST(output_contains_supports_helper);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("inline bool supports") != std::string::npos, "supports helper missing");
    PASS();
}

void test_contains_type_detects_known_types() {
    TEST(contains_type_detects_known_types);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(CapabilityDeclarationGenerator::containsType(out, "NodeCapability"), "type should exist");
    CHECK(CapabilityDeclarationGenerator::containsType(out, "JobRequirements"), "type should exist");
    PASS();
}

void test_contains_type_rejects_unknown_type() {
    TEST(contains_type_rejects_unknown_type);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(!CapabilityDeclarationGenerator::containsType(out, "UnknownType"), "unknown type should not exist");
    PASS();
}

void test_header_uses_header_only_pattern() {
    TEST(header_uses_header_only_pattern);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.headerCode.find("#pragma once") != std::string::npos, "pragma once missing");
    PASS();
}

void test_custom_namespace_is_applied() {
    TEST(custom_namespace_is_applied);
    auto out = CapabilityDeclarationGenerator::generate("hivemind");
    CHECK(out.headerCode.find("namespace hivemind") != std::string::npos, "custom namespace missing");
    PASS();
}

void test_type_count_matches_expected_set() {
    TEST(type_count_matches_expected_set);
    auto out = CapabilityDeclarationGenerator::generate();
    CHECK(out.typeNames.size() == 4, "expected 4 generated types");
    PASS();
}

int main() {
    std::cout << "Step 635: Capability declaration struct generator\n";

    test_generate_requires_non_empty_namespace();
    test_generate_succeeds_with_default_namespace();
    test_output_contains_node_capability_struct();
    test_output_contains_capability_set_struct();
    test_output_contains_energy_context_struct();
    test_output_contains_job_requirements_struct();
    test_output_contains_supports_helper();
    test_contains_type_detects_known_types();
    test_contains_type_rejects_unknown_type();
    test_header_uses_header_only_pattern();
    test_custom_namespace_is_applied();
    test_type_count_matches_expected_set();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
