// Step 492: Phase 24b Integration + Sprint Summary Tests (8 tests)

#include "SecurityPipelineIntegration.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SecurityPipelineInput sampleInput() {
    SecurityPipelineInput in;
    in.sourceLanguage = "python";
    in.targetLanguage = "rust";
    in.moduleName = "api";
    in.entityName = "User";
    in.sourceAnnotations = {
        {"@InputValidation", "sanitized"},
        {"@TrustBoundary", "external-api"},
        {"@Auth", "jwt"},
        {"@Encryption", "aes256"}
    };
    in.threatModel.moduleName = "api";
    in.threatModel.boundaries = {{"external-api", "high"}, {"database", "medium"}};
    in.threatModel.flows = {{"external-api", "database", true, "sanitized"}};
    return in;
}

void test_python_to_rust_translation_preserves_security_annotations() {
    TEST(python_to_rust_translation_preserves_security_annotations);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    CHECK(out.translation.mappings.size() == 4, "annotation count mismatch");
    CHECK(out.translation.preservedCount() == 4, "security annotations not fully preserved");
    PASS();
}

void test_generated_sql_and_input_code_use_secure_defaults() {
    TEST(generated_sql_and_input_code_use_secure_defaults);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    CHECK(out.secureSqlSnippet.code.find(".bind(") != std::string::npos ||
          out.secureSqlSnippet.code.find("?") != std::string::npos ||
          out.secureSqlSnippet.code.find("%s") != std::string::npos,
          "sql snippet not parameterized");
    bool hasRaw = false;
    for (const auto& a : out.secureInputSnippet.annotations) {
        if (a == "@InputValidation(raw)") hasRaw = true;
    }
    CHECK(hasRaw, "input snippet missing raw annotation");
    PASS();
}

void test_threat_model_outputs_boundary_diagnostics_when_validation_missing() {
    TEST(threat_model_outputs_boundary_diagnostics_when_validation_missing);
    auto in = sampleInput();
    in.threatModel.flows = {{"external-api", "database", false, ""}};
    auto out = SecurityPipelineIntegration::run(in);
    CHECK(!out.threat.diagnostics.empty(), "expected threat diagnostics");
    CHECK(out.threat.diagnostics[0].code == 1300, "expected E1300");
    PASS();
}

void test_security_test_skeletons_generated_with_meaningful_intent() {
    TEST(security_test_skeletons_generated_with_meaningful_intent);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    CHECK(out.tests.tests.size() >= 5, "expected baseline security tests");
    CHECK(out.tests.tests[0].intentAnnotation.find("@Intent(") == 0, "missing intent annotation");
    PASS();
}

void test_translation_blocking_review_propagates_to_pipeline_notes() {
    TEST(translation_blocking_review_propagates_to_pipeline_notes);
    auto in = sampleInput();
    in.sourceAnnotations.push_back({"@Secrets", "rotation:30d"});
    auto out = SecurityPipelineIntegration::run(in);
    CHECK(out.translation.hasBlockingReview, "expected blocking review");
    bool found = false;
    for (const auto& n : out.notes) {
        if (n.find("blocking review") != std::string::npos) found = true;
    }
    CHECK(found, "pipeline notes missing blocking-review mention");
    PASS();
}

void test_pipeline_notes_include_execution_trace_summary() {
    TEST(pipeline_notes_include_execution_trace_summary);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    CHECK(!out.notes.empty(), "missing notes");
    CHECK(out.notes[0].find("translation -> secure generation -> threat model -> test skeletons")
          != std::string::npos, "missing execution trace summary");
    PASS();
}

void test_access_control_and_pen_tests_route_human_in_generated_suite() {
    TEST(access_control_and_pen_tests_route_human_in_generated_suite);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    bool hasHuman = false;
    for (const auto& t : out.tests.tests) {
        if (t.kind == SecurityTestKind::SqlInjectionProbe ||
            t.kind == SecurityTestKind::XssPayload ||
            t.kind == SecurityTestKind::AuthBypass) {
            if (t.routeTo == "human") hasHuman = true;
        }
    }
    CHECK(hasHuman, "expected human-routed penetration tests");
    PASS();
}

void test_integration_regression_step491_behavior_still_available() {
    TEST(integration_regression_step491_behavior_still_available);
    auto out = SecurityPipelineIntegration::run(sampleInput());
    bool hasAccessMatrix = false;
    for (const auto& t : out.tests.tests) {
        if (t.kind == SecurityTestKind::AccessControlMatrix) hasAccessMatrix = true;
    }
    CHECK(hasAccessMatrix, "expected access control matrix test");
    PASS();
}

int main() {
    std::cout << "Step 492: Phase 24b Integration + Sprint Summary Tests\n";

    test_python_to_rust_translation_preserves_security_annotations();          // 1
    test_generated_sql_and_input_code_use_secure_defaults();                  // 2
    test_threat_model_outputs_boundary_diagnostics_when_validation_missing(); // 3
    test_security_test_skeletons_generated_with_meaningful_intent();          // 4
    test_translation_blocking_review_propagates_to_pipeline_notes();          // 5
    test_pipeline_notes_include_execution_trace_summary();                     // 6
    test_access_control_and_pen_tests_route_human_in_generated_suite();       // 7
    test_integration_regression_step491_behavior_still_available();           // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
