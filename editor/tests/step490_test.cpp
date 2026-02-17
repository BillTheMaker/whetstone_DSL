// Step 490: Threat Model Integration Tests (12 tests)

#include "ThreatModelIntegration.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ThreatModelInput baseInput() {
    ThreatModelInput in;
    in.moduleName = "api";
    in.boundaries = {{"external-api", "high"}, {"database", "medium"}, {"internal", "low"}};
    return in;
}

void test_threat_model_annotation_emitted_at_module_level() {
    TEST(threat_model_annotation_emitted_at_module_level);
    auto out = ThreatModelIntegration::analyze(baseInput());
    bool found = false;
    for (const auto& a : out.requiredAnnotations) {
        if (a.find("@ThreatModel(module=api)") != std::string::npos) found = true;
    }
    CHECK(found, "missing module threat model annotation");
    PASS();
}

void test_trust_boundaries_are_included_in_overlay_data() {
    TEST(trust_boundaries_are_included_in_overlay_data);
    auto out = ThreatModelIntegration::analyze(baseInput());
    CHECK(out.overlayNodes.size() == 3, "overlay node count mismatch");
    CHECK(out.overlayNodes[0].find("external-api:high") != std::string::npos, "missing boundary node");
    PASS();
}

void test_cross_boundary_flow_without_validation_emits_e1300() {
    TEST(cross_boundary_flow_without_validation_emits_e1300);
    auto in = baseInput();
    in.flows = {{"external-api", "database", false, ""}};
    auto out = ThreatModelIntegration::analyze(in);
    CHECK(!out.diagnostics.empty(), "expected diagnostics");
    CHECK(out.diagnostics[0].code == 1300, "expected E1300 diagnostic");
    PASS();
}

void test_boundary_flow_with_raw_validation_emits_e1301() {
    TEST(boundary_flow_with_raw_validation_emits_e1301);
    auto in = baseInput();
    in.flows = {{"external-api", "database", true, "raw"}};
    auto out = ThreatModelIntegration::analyze(in);
    CHECK(!out.diagnostics.empty(), "expected diagnostics");
    CHECK(out.diagnostics[0].code == 1301, "expected E1301 diagnostic");
    PASS();
}

void test_sanitized_boundary_flow_avoids_validation_diagnostic() {
    TEST(sanitized_boundary_flow_avoids_validation_diagnostic);
    auto in = baseInput();
    in.flows = {{"external-api", "database", true, "sanitized"}};
    auto out = ThreatModelIntegration::analyze(in);
    CHECK(out.diagnostics.empty(), "did not expect diagnostics for sanitized flow");
    PASS();
}

void test_unknown_validation_kind_emits_e1303() {
    TEST(unknown_validation_kind_emits_e1303);
    auto in = baseInput();
    in.flows = {{"external-api", "database", true, "custom"}};
    auto out = ThreatModelIntegration::analyze(in);
    CHECK(!out.diagnostics.empty(), "expected diagnostics");
    CHECK(out.diagnostics[0].code == 1303, "expected E1303 diagnostic");
    PASS();
}

void test_no_boundaries_emits_e1302() {
    TEST(no_boundaries_emits_e1302);
    ThreatModelInput in;
    in.moduleName = "api";
    in.flows = {{"external-api", "database", false, ""}};
    auto out = ThreatModelIntegration::analyze(in);
    bool found = false;
    for (const auto& d : out.diagnostics) if (d.code == 1302) found = true;
    CHECK(found, "expected E1302 for missing boundaries");
    PASS();
}

void test_data_flow_annotation_is_emitted_for_crossing() {
    TEST(data_flow_annotation_is_emitted_for_crossing);
    auto in = baseInput();
    in.flows = {{"external-api", "database", true, "sanitized"}};
    auto out = ThreatModelIntegration::analyze(in);
    bool found = false;
    for (const auto& a : out.requiredAnnotations) {
        if (a.find("@DataFlow(external-api->database)") != std::string::npos) found = true;
    }
    CHECK(found, "expected @DataFlow annotation");
    PASS();
}

void test_missing_validation_adds_input_validation_required_annotation() {
    TEST(missing_validation_adds_input_validation_required_annotation);
    auto in = baseInput();
    in.flows = {{"user-input", "internal", false, ""}};
    auto out = ThreatModelIntegration::analyze(in);
    bool found = false;
    for (const auto& a : out.requiredAnnotations) {
        if (a == "@InputValidation(required)") found = true;
    }
    CHECK(found, "expected @InputValidation(required)");
    PASS();
}

void test_internal_same_boundary_flow_does_not_trigger_e1300() {
    TEST(internal_same_boundary_flow_does_not_trigger_e1300);
    auto in = baseInput();
    in.flows = {{"internal", "internal", false, ""}};
    auto out = ThreatModelIntegration::analyze(in);
    for (const auto& d : out.diagnostics) {
        CHECK(d.code != 1300, "E1300 should not be emitted for non-crossing flow");
    }
    PASS();
}

void test_overlay_note_is_present() {
    TEST(overlay_note_is_present);
    auto out = ThreatModelIntegration::analyze(baseInput());
    CHECK(!out.notes.empty(), "expected notes");
    CHECK(out.notes.back().find("overlay") != std::string::npos, "missing overlay note");
    PASS();
}

void test_empty_flows_add_note_but_not_false_positive_diagnostic() {
    TEST(empty_flows_add_note_but_not_false_positive_diagnostic);
    auto in = baseInput();
    auto out = ThreatModelIntegration::analyze(in);
    bool note = false;
    for (const auto& n : out.notes) {
        if (n.find("No data flows") != std::string::npos) note = true;
    }
    CHECK(note, "expected no-data-flow note");
    PASS();
}

int main() {
    std::cout << "Step 490: Threat Model Integration Tests\n";

    test_threat_model_annotation_emitted_at_module_level();             // 1
    test_trust_boundaries_are_included_in_overlay_data();               // 2
    test_cross_boundary_flow_without_validation_emits_e1300();          // 3
    test_boundary_flow_with_raw_validation_emits_e1301();               // 4
    test_sanitized_boundary_flow_avoids_validation_diagnostic();        // 5
    test_unknown_validation_kind_emits_e1303();                         // 6
    test_no_boundaries_emits_e1302();                                   // 7
    test_data_flow_annotation_is_emitted_for_crossing();                // 8
    test_missing_validation_adds_input_validation_required_annotation(); // 9
    test_internal_same_boundary_flow_does_not_trigger_e1300();          // 10
    test_overlay_note_is_present();                                     // 11
    test_empty_flows_add_note_but_not_false_positive_diagnostic();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
