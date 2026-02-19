// Step 671: wiring verification for RegisterModelingTools + tools.json (8 tests)

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static json loadToolsJson() {
    std::vector<std::string> candidates = {
        "../../tools/claude/tools.json",
        "../../../tools/claude/tools.json",
        "/home/bill/Documents/CLionProjects/whetstone_DSL/tools/claude/tools.json"
    };
    for (const auto& path : candidates) {
        std::ifstream in(path);
        if (in.good()) {
            json d;
            in >> d;
            return d;
        }
    }
    return {};
}

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.good()) return "";
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

void t1() {
    T(tools_json_valid);
    auto d = loadToolsJson();
    C(!d.is_null(), "tools.json missing/invalid");
    C(d.contains("tools"), "missing tools array");
    P();
}

void t2() {
    T(tools_json_contains_generate_project);
    auto d = loadToolsJson();
    bool found = false;
    for (const auto& t : d["tools"]) {
        if (t.value("name", "") == "whetstone_generate_project") { found = true; break; }
    }
    C(found, "whetstone_generate_project missing");
    P();
}

void t3() {
    T(tools_json_contains_generate_inference_job);
    auto d = loadToolsJson();
    bool found = false;
    for (const auto& t : d["tools"]) {
        if (t.value("name", "") == "whetstone_generate_inference_job") { found = true; break; }
    }
    C(found, "whetstone_generate_inference_job missing");
    P();
}

void t4() {
    T(tool_count_is_86);
    auto d = loadToolsJson();
    C(d["tools"].size() == 86, "expected 86 tools, got " + std::to_string(d["tools"].size()));
    P();
}

void t5() {
    T(no_duplicate_names);
    auto d = loadToolsJson();
    std::vector<std::string> names;
    for (const auto& t : d["tools"]) names.push_back(t["name"].get<std::string>());
    std::sort(names.begin(), names.end());
    bool dup = std::adjacent_find(names.begin(), names.end()) != names.end();
    C(!dup, "duplicate tool names detected");
    P();
}

void t6() {
    T(new_tools_have_required_schema_fields);
    auto d = loadToolsJson();
    for (const auto& t : d["tools"]) {
        std::string n = t.value("name", "");
        if (n == "whetstone_generate_project" || n == "whetstone_generate_inference_job") {
            C(t.contains("input_schema"), n + " missing input_schema");
            C(t["input_schema"].contains("required"), n + " missing required");
        }
    }
    P();
}

void t7() {
    T(mcpserver_includes_register_modeling_tools);
    std::string text = readFile("src/MCPServer.h");
    if (text.empty()) text = readFile("/home/bill/Documents/CLionProjects/whetstone_DSL/editor/src/MCPServer.h");
    C(!text.empty(), "unable to read MCPServer.h");
    C(text.find("#include \"mcp/RegisterModelingTools.h\"") != std::string::npos, "include missing");
    P();
}

void t8() {
    T(register_whetstone_tools_calls_register_modeling_tools);
    std::string text = readFile("src/mcp/RegisterOnboardingAndAllTools.h");
    if (text.empty()) text = readFile("/home/bill/Documents/CLionProjects/whetstone_DSL/editor/src/mcp/RegisterOnboardingAndAllTools.h");
    C(!text.empty(), "unable to read RegisterOnboardingAndAllTools.h");
    C(text.find("registerModelingTools();") != std::string::npos, "registerModelingTools call missing");
    P();
}

int main() {
    std::cout << "Step 671: RegisterModelingTools wiring verification\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
