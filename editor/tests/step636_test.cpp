// Step 636: Error type synthesis for drone (12 tests)

#include "DroneErrorSynthesis.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_class_name_for_mqtt_error() {
    TEST(class_name_for_mqtt_error);
    CHECK(DroneErrorSynthesis::className(DroneErrorKind::MqttError) == "MqttError", "name mismatch");
    PASS();
}

void test_class_name_for_nexus_error() {
    TEST(class_name_for_nexus_error);
    CHECK(DroneErrorSynthesis::className(DroneErrorKind::NexusError) == "NexusError", "name mismatch");
    PASS();
}

void test_category_for_execution_error() {
    TEST(category_for_execution_error);
    CHECK(DroneErrorSynthesis::toErrorCodeCategory(DroneErrorKind::ExecutionError) == "execution", "category mismatch");
    PASS();
}

void test_category_for_capability_mismatch() {
    TEST(category_for_capability_mismatch);
    CHECK(DroneErrorSynthesis::toErrorCodeCategory(DroneErrorKind::CapabilityMismatch) == "capability", "category mismatch");
    PASS();
}

void test_expected_alias_default_value_type() {
    TEST(expected_alias_default_value_type);
    CHECK(DroneErrorSynthesis::expectedAlias(DroneErrorKind::MqttError) == "std::expected<void, MqttError>", "alias mismatch");
    PASS();
}

void test_expected_alias_custom_value_type() {
    TEST(expected_alias_custom_value_type);
    CHECK(DroneErrorSynthesis::expectedAlias(DroneErrorKind::NexusError, "int") == "std::expected<int, NexusError>", "alias mismatch");
    PASS();
}

void test_retryable_true_for_mqtt_positive_code() {
    TEST(retryable_true_for_mqtt_positive_code);
    DroneErrorSpec spec{DroneErrorKind::MqttError, "timeout", 1, true};
    CHECK(DroneErrorSynthesis::isRetryable(spec), "expected retryable");
    PASS();
}

void test_retryable_false_for_capability_mismatch() {
    TEST(retryable_false_for_capability_mismatch);
    DroneErrorSpec spec{DroneErrorKind::CapabilityMismatch, "missing cap", 1, true};
    CHECK(!DroneErrorSynthesis::isRetryable(spec), "capability mismatch should not retry");
    PASS();
}

void test_retryable_false_for_negative_code() {
    TEST(retryable_false_for_negative_code);
    DroneErrorSpec spec{DroneErrorKind::NexusError, "fatal", -1, true};
    CHECK(!DroneErrorSynthesis::isRetryable(spec), "negative code should not retry");
    PASS();
}

void test_synthesize_header_contains_mqtt_error() {
    TEST(synthesize_header_contains_mqtt_error);
    auto code = DroneErrorSynthesis::synthesizeHeader();
    CHECK(code.find("struct MqttError") != std::string::npos, "mqtt struct missing");
    PASS();
}

void test_synthesize_header_contains_expected_include() {
    TEST(synthesize_header_contains_expected_include);
    auto code = DroneErrorSynthesis::synthesizeHeader();
    CHECK(code.find("#include <expected>") != std::string::npos, "expected include missing");
    PASS();
}

void test_synthesize_header_rejects_empty_namespace() {
    TEST(synthesize_header_rejects_empty_namespace);
    auto code = DroneErrorSynthesis::synthesizeHeader("");
    CHECK(code.empty(), "empty namespace should produce empty output");
    PASS();
}

int main() {
    std::cout << "Step 636: Error type synthesis for drone\n";

    test_class_name_for_mqtt_error();
    test_class_name_for_nexus_error();
    test_category_for_execution_error();
    test_category_for_capability_mismatch();
    test_expected_alias_default_value_type();
    test_expected_alias_custom_value_type();
    test_retryable_true_for_mqtt_positive_code();
    test_retryable_false_for_capability_mismatch();
    test_retryable_false_for_negative_code();
    test_synthesize_header_contains_mqtt_error();
    test_synthesize_header_contains_expected_include();
    test_synthesize_header_rejects_empty_namespace();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
