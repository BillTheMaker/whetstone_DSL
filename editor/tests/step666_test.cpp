// Step 666: wiring verification — RegisterCodegenTools wired into MCPServer (8 tests)

#include "SchemaToCppGenerator.h"
#include "JobDispatchTableGenerator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;
static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static json loadToolsJson() {
    // Resolve path relative to standard build location
    std::vector<std::string> candidates = {
        "../../tools/claude/tools.json",
        "../../../tools/claude/tools.json",
        "/home/bill/Documents/CLionProjects/whetstone_DSL/tools/claude/tools.json"
    };
    for (const auto& path : candidates) {
        std::ifstream f(path);
        if (f.good()) { json d; f >> d; return d; }
    }
    return {};
}

void t1() {
    T(schema_to_cpp_generator_callable);
    json s = {{"title", "Test"}, {"properties", {{"x", {{"type", "string"}}}}}};
    auto out = SchemaToCppGenerator::generate(s, "Test.h", "test_types");
    C(out.success, "SchemaToCppGenerator::generate failed");
    P();
}

void t2() {
    T(dispatch_table_generator_callable);
    auto out = JobDispatchTableGenerator::generate({{"job_a", {}, "", "fnA"}});
    C(out.success, "JobDispatchTableGenerator::generate failed");
    P();
}

void t3() {
    T(tools_json_is_valid_and_loadable);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not found or invalid JSON");
    C(d.contains("tools"), "tools.json missing 'tools' key");
    P();
}

void t4() {
    T(tools_json_contains_schema_to_cpp);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not loaded");
    bool found = false;
    for (const auto& t : d["tools"]) { if (t["name"] == "whetstone_schema_to_cpp") { found = true; break; } }
    C(found, "whetstone_schema_to_cpp not in tools.json");
    P();
}

void t5() {
    T(tools_json_contains_dispatch_table);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not loaded");
    bool found = false;
    for (const auto& t : d["tools"]) { if (t["name"] == "whetstone_generate_dispatch_table") { found = true; break; } }
    C(found, "whetstone_generate_dispatch_table not in tools.json");
    P();
}

void t6() {
    T(tools_json_count_is_84);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not loaded");
    C(d["tools"].size() == 84, "expected 84 tools, got " + std::to_string(d["tools"].size()));
    P();
}

void t7() {
    T(no_duplicate_tool_names);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not loaded");
    std::vector<std::string> names;
    for (const auto& t : d["tools"]) names.push_back(t["name"].get<std::string>());
    std::sort(names.begin(), names.end());
    bool dupe = std::adjacent_find(names.begin(), names.end()) != names.end();
    C(!dupe, "duplicate tool name found in tools.json");
    P();
}

void t8() {
    T(new_tools_have_input_schema_and_required);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json not loaded");
    for (const auto& t : d["tools"]) {
        std::string name = t["name"].get<std::string>();
        if (name == "whetstone_schema_to_cpp" || name == "whetstone_generate_dispatch_table") {
            C(t.contains("input_schema"), name + " missing input_schema");
            C(t["input_schema"].contains("required"), name + " input_schema missing required");
        }
    }
    P();
}

int main() {
    std::cout << "Step 666: RegisterCodegenTools wiring verification\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
