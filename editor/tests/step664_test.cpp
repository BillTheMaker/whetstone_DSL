// Step 664: whetstone_schema_to_cpp MCP tool (12 tests)
// Tests SchemaToCppGenerator directly — same logic the MCP handler calls.

#include "SchemaToCppGenerator.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static json minimalSchema() {
    return {{"title", "EnergyContext"}, {"properties", {{"state", {{"type", "string"}}}, {"surplus_watts", {{"type", "integer"}}}}}};
}

void t1() {
    T(valid_schema_succeeds);
    auto out = SchemaToCppGenerator::generate(minimalSchema(), "EnergyContext.h", "energy_context_types");
    C(out.success, "expected success");
    P();
}

void t2() {
    T(schema_without_title_errors);
    json s = {{"properties", {{"x", {{"type", "string"}}}}}};
    auto out = SchemaToCppGenerator::generate(s, "X.h", "x_types");
    C(!out.success, "expected failure");
    C(!out.errors.empty(), "expected error message");
    P();
}

void t3() {
    T(empty_properties_produces_raw_field);
    json s = {{"title", "Empty"}, {"properties", json::object()}};
    auto out = SchemaToCppGenerator::generate(s, "Empty.h", "empty_types");
    C(out.success, "expected success");
    C(out.headerCode.find("raw") != std::string::npos, "expected 'raw' fallback field");
    P();
}

void t4() {
    T(integer_type_maps_to_int);
    json s = {{"title", "Foo"}, {"properties", {{"count", {{"type", "integer"}}}}}};
    auto out = SchemaToCppGenerator::generate(s, "Foo.h", "foo_types");
    C(out.success, "expected success");
    C(out.headerCode.find("int count") != std::string::npos, "expected 'int count'");
    P();
}

void t5() {
    T(boolean_type_maps_to_bool);
    json s = {{"title", "Bar"}, {"properties", {{"flag", {{"type", "boolean"}}}}}};
    auto out = SchemaToCppGenerator::generate(s, "Bar.h", "bar_types");
    C(out.success, "expected success");
    C(out.headerCode.find("bool flag") != std::string::npos, "expected 'bool flag'");
    P();
}

void t6() {
    T(number_type_maps_to_double);
    json s = {{"title", "Baz"}, {"properties", {{"ratio", {{"type", "number"}}}}}};
    auto out = SchemaToCppGenerator::generate(s, "Baz.h", "baz_types");
    C(out.success, "expected success");
    C(out.headerCode.find("double ratio") != std::string::npos, "expected 'double ratio'");
    P();
}

void t7() {
    T(cmake_interface_target_non_empty_on_success);
    auto out = SchemaToCppGenerator::generate(minimalSchema(), "EnergyContext.h", "energy_context_types");
    C(out.success, "expected success");
    C(!out.cmakeInterfaceTarget.empty(), "expected non-empty cmake target");
    C(out.cmakeInterfaceTarget.find("energy_context_types") != std::string::npos, "target name missing");
    P();
}

void t8() {
    T(invalid_non_object_schema_errors);
    json s = json::array({"not", "an", "object"});
    auto out = SchemaToCppGenerator::generate(s, "X.h", "x");
    C(!out.success, "expected failure on array input");
    P();
}

void t9() {
    T(header_code_contains_pragma_once);
    auto out = SchemaToCppGenerator::generate(minimalSchema(), "EnergyContext.h", "energy_context_types");
    C(out.success, "expected success");
    C(out.headerCode.find("#pragma once") != std::string::npos, "missing #pragma once");
    P();
}

void t10() {
    T(header_code_contains_struct_name);
    auto out = SchemaToCppGenerator::generate(minimalSchema(), "EnergyContext.h", "energy_context_types");
    C(out.success, "expected success");
    C(out.headerCode.find("struct EnergyContext") != std::string::npos, "missing struct EnergyContext");
    P();
}

void t11() {
    T(header_code_contains_from_json_and_to_json);
    auto out = SchemaToCppGenerator::generate(minimalSchema(), "EnergyContext.h", "energy_context_types");
    C(out.success, "expected success");
    C(out.headerCode.find("from_json") != std::string::npos, "missing from_json");
    C(out.headerCode.find("to_json") != std::string::npos, "missing to_json");
    P();
}

void t12() {
    T(tool_name_is_whetstone_schema_to_cpp);
    C(SchemaToCppGenerator::toolName() == "whetstone_schema_to_cpp", "wrong tool name");
    P();
}

int main() {
    std::cout << "Step 664: whetstone_schema_to_cpp\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
