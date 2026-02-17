// Step 491: Security Testing Skeleton Generation Tests (12 tests)

#include "SecurityTestSkeletonGenerator.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SecuritySkeletonOutput run(const std::string& module = "api",
                                  const std::string& entity = "User",
                                  bool complex = true) {
    SecuritySkeletonRequest req;
    req.language = "cpp";
    req.moduleName = module;
    req.entityName = entity;
    req.includeComplexPenTest = complex;
    return SecurityTestSkeletonGenerator::generate(req);
}

void test_generates_input_validation_fuzz_test() {
    TEST(generates_input_validation_fuzz_test);
    auto out = run();
    CHECK(out.tests[0].kind == SecurityTestKind::InputValidationFuzz, "kind mismatch");
    CHECK(out.tests[0].testName.find("input_validation_fuzz") != std::string::npos, "name mismatch");
    PASS();
}

void test_generates_auth_bypass_test() {
    TEST(generates_auth_bypass_test);
    auto out = run();
    bool found = false;
    for (const auto& t : out.tests) if (t.kind == SecurityTestKind::AuthBypass) found = true;
    CHECK(found, "missing auth bypass test");
    PASS();
}

void test_generates_sql_injection_probe_test() {
    TEST(generates_sql_injection_probe_test);
    auto out = run();
    bool found = false;
    for (const auto& t : out.tests) if (t.kind == SecurityTestKind::SqlInjectionProbe) found = true;
    CHECK(found, "missing sql injection test");
    PASS();
}

void test_generates_xss_payload_test() {
    TEST(generates_xss_payload_test);
    auto out = run();
    bool found = false;
    for (const auto& t : out.tests) if (t.kind == SecurityTestKind::XssPayload) found = true;
    CHECK(found, "missing xss payload test");
    PASS();
}

void test_generates_access_control_matrix_test() {
    TEST(generates_access_control_matrix_test);
    auto out = run();
    bool found = false;
    for (const auto& t : out.tests) if (t.kind == SecurityTestKind::AccessControlMatrix) found = true;
    CHECK(found, "missing access control matrix test");
    PASS();
}

void test_all_tests_include_intent_annotation() {
    TEST(all_tests_include_intent_annotation);
    auto out = run();
    for (const auto& t : out.tests) {
        CHECK(t.intentAnnotation.find("@Intent(") == 0, "missing @Intent prefix");
    }
    PASS();
}

void test_simple_validation_tests_route_to_slm() {
    TEST(simple_validation_tests_route_to_slm);
    auto out = run();
    bool ok = false;
    for (const auto& t : out.tests) {
        if (t.kind == SecurityTestKind::InputValidationFuzz && t.routeTo == "slm") ok = true;
    }
    CHECK(ok, "input validation fuzz should route to slm");
    PASS();
}

void test_complex_penetration_tests_route_to_human() {
    TEST(complex_penetration_tests_route_to_human);
    auto out = run();
    for (const auto& t : out.tests) {
        if (t.kind == SecurityTestKind::SqlInjectionProbe ||
            t.kind == SecurityTestKind::XssPayload ||
            t.kind == SecurityTestKind::AuthBypass) {
            CHECK(t.routeTo == "human", "complex pen test should route to human");
            CHECK(t.humanReviewRequired, "complex pen test should require human review");
        }
    }
    PASS();
}

void test_complex_pen_tests_forced_human_when_disabled_flag() {
    TEST(complex_pen_tests_forced_human_when_disabled_flag);
    auto out = run("api", "User", false);
    bool hasNote = false;
    for (const auto& n : out.notes) {
        if (n.find("forced to human review") != std::string::npos) hasNote = true;
    }
    CHECK(hasNote, "missing forced-human note");
    PASS();
}

void test_test_names_include_module_name() {
    TEST(test_names_include_module_name);
    auto out = run("payments", "Invoice");
    for (const auto& t : out.tests) {
        CHECK(t.testName.find("payments") != std::string::npos, "module name missing in test name");
    }
    PASS();
}

void test_entity_name_propagates_into_intent_text() {
    TEST(entity_name_propagates_into_intent_text);
    auto out = run("api", "Invoice");
    bool mentioned = false;
    for (const auto& t : out.tests) {
        if (t.intentAnnotation.find("Invoice") != std::string::npos) mentioned = true;
    }
    CHECK(mentioned, "entity name not propagated");
    PASS();
}

void test_skeleton_output_contains_summary_note() {
    TEST(skeleton_output_contains_summary_note);
    auto out = run();
    bool found = false;
    for (const auto& n : out.notes) {
        if (n.find("Security test skeletons generated") != std::string::npos) found = true;
    }
    CHECK(found, "missing generation summary note");
    PASS();
}

int main() {
    std::cout << "Step 491: Security Testing Skeleton Generation Tests\n";

    test_generates_input_validation_fuzz_test();              // 1
    test_generates_auth_bypass_test();                        // 2
    test_generates_sql_injection_probe_test();                // 3
    test_generates_xss_payload_test();                        // 4
    test_generates_access_control_matrix_test();              // 5
    test_all_tests_include_intent_annotation();               // 6
    test_simple_validation_tests_route_to_slm();              // 7
    test_complex_penetration_tests_route_to_human();          // 8
    test_complex_pen_tests_forced_human_when_disabled_flag(); // 9
    test_test_names_include_module_name();                    // 10
    test_entity_name_propagates_into_intent_text();           // 11
    test_skeleton_output_contains_summary_note();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
