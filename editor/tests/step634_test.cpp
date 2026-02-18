// Step 634: Job schema -> typed C++ structs generator (12 tests)

#include "SchemaToCppGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void test_tool_name_matches_spec() {
    TEST(tool_name_matches_spec);
    CHECK(SchemaToCppGenerator::toolName() == "whetstone_schema_to_cpp", "tool name mismatch");
    PASS();
}

void test_generate_rejects_non_object_schema() {
    TEST(generate_rejects_non_object_schema);
    auto out = SchemaToCppGenerator::generate(json::array(), "Job.h", "job_schema_types");
    CHECK(!out.success, "non-object should fail");
    CHECK(!out.errors.empty(), "error expected");
    PASS();
}

void test_generate_rejects_missing_title() {
    TEST(generate_rejects_missing_title);
    json schema = {{"title", ""}, {"properties", json::object()}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(!out.success, "missing title should fail");
    PASS();
}

void test_generate_emits_struct_name_from_title() {
    TEST(generate_emits_struct_name_from_title);
    json schema = {{"title", "JobPayload"}, {"properties", json::object()}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.success, "generate should pass");
    CHECK(out.headerCode.find("struct JobPayload") != std::string::npos, "struct missing");
    PASS();
}

void test_generate_maps_integer_to_int() {
    TEST(generate_maps_integer_to_int);
    json schema = {{"title", "JobPayload"}, {"properties", {{"priority", {{"type", "integer"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("int priority;") != std::string::npos, "int field missing");
    PASS();
}

void test_generate_maps_boolean_to_bool() {
    TEST(generate_maps_boolean_to_bool);
    json schema = {{"title", "JobPayload"}, {"properties", {{"urgent", {{"type", "boolean"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("bool urgent;") != std::string::npos, "bool field missing");
    PASS();
}

void test_generate_defaults_unknown_type_to_string() {
    TEST(generate_defaults_unknown_type_to_string);
    json schema = {{"title", "JobPayload"}, {"properties", {{"payload", {{"type", "object"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("std::string payload;") != std::string::npos, "string fallback missing");
    PASS();
}

void test_generate_emits_to_json_and_from_json() {
    TEST(generate_emits_to_json_and_from_json);
    json schema = {{"title", "JobPayload"}, {"properties", {{"id", {{"type", "string"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("to_json") != std::string::npos, "to_json missing");
    CHECK(out.headerCode.find("from_json") != std::string::npos, "from_json missing");
    PASS();
}

void test_generate_emits_validation_stub() {
    TEST(generate_emits_validation_stub);
    json schema = {{"title", "JobPayload"}, {"properties", {{"id", {{"type", "string"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("validateJobPayload") != std::string::npos, "validate function missing");
    PASS();
}

void test_generate_emits_interface_target_text() {
    TEST(generate_emits_interface_target_text);
    json schema = {{"title", "JobPayload"}, {"properties", json::object()}};
    auto out = SchemaToCppGenerator::generate(schema, "JobPayload.h", "job_schema_types");
    CHECK(out.cmakeInterfaceTarget.find("add_library(job_schema_types INTERFACE)") != std::string::npos,
          "interface target missing");
    PASS();
}

void test_generate_sanitizes_hyphenated_field_name() {
    TEST(generate_sanitizes_hyphenated_field_name);
    json schema = {{"title", "JobPayload"}, {"properties", {{"job-id", {{"type", "string"}}}}}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("job_id") != std::string::npos, "sanitized field missing");
    PASS();
}

void test_generate_inserts_default_field_when_properties_empty() {
    TEST(generate_inserts_default_field_when_properties_empty);
    json schema = {{"title", "JobPayload"}, {"properties", json::object()}};
    auto out = SchemaToCppGenerator::generate(schema, "Job.h", "job_schema_types");
    CHECK(out.headerCode.find("std::string raw;") != std::string::npos, "raw fallback field missing");
    PASS();
}

int main() {
    std::cout << "Step 634: Job schema -> typed C++ structs generator\n";

    test_tool_name_matches_spec();
    test_generate_rejects_non_object_schema();
    test_generate_rejects_missing_title();
    test_generate_emits_struct_name_from_title();
    test_generate_maps_integer_to_int();
    test_generate_maps_boolean_to_bool();
    test_generate_defaults_unknown_type_to_string();
    test_generate_emits_to_json_and_from_json();
    test_generate_emits_validation_stub();
    test_generate_emits_interface_target_text();
    test_generate_sanitizes_hyphenated_field_name();
    test_generate_inserts_default_field_when_properties_empty();

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
