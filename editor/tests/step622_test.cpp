// Step 622: tools.json refresh (8 tests)

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <set>
#include <string>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool loadToolsJson(json* out, std::string* error) {
    std::ifstream in("tools/claude/tools.json");
    if (!in.is_open()) {
        *error = "open_failed";
        return false;
    }
    try {
        in >> *out;
        return true;
    } catch (...) {
        *error = "parse_failed";
        return false;
    }
}

static bool hasTool(const json& tools, const std::string& name) {
    for (const auto& tool : tools) {
        if (tool.value("name", "") == name) return true;
    }
    return false;
}

void test_tools_json_parses() {
    TEST(tools_json_parses);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    PASS();
}

void test_version_bumped_to_2_0() {
    TEST(version_bumped_to_2_0);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    CHECK(doc.value("version", "") == "2.0", "version should be 2.0");
    PASS();
}

void test_provider_still_anthropic() {
    TEST(provider_still_anthropic);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    CHECK(doc.value("provider", "") == "anthropic", "provider mismatch");
    PASS();
}

void test_tools_array_has_broad_coverage() {
    TEST(tools_array_has_broad_coverage);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    CHECK(doc.contains("tools") && doc["tools"].is_array(), "tools array missing");
    CHECK((int)doc["tools"].size() >= 70, "expected at least 70 tools");
    PASS();
}

void test_contains_new_sprint36_tools() {
    TEST(contains_new_sprint36_tools);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    const auto& tools = doc["tools"];
    CHECK(hasTool(tools, "whetstone_architect_intake"), "missing whetstone_architect_intake");
    CHECK(hasTool(tools, "whetstone_generate_taskitems"), "missing whetstone_generate_taskitems");
    CHECK(hasTool(tools, "whetstone_queue_ready"), "missing whetstone_queue_ready");
    CHECK(hasTool(tools, "whetstone_set_workspace"), "missing whetstone_set_workspace");
    PASS();
}

void test_contains_sprint32_35_operational_tools() {
    TEST(contains_sprint32_35_operational_tools);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    const auto& tools = doc["tools"];
    CHECK(hasTool(tools, "whetstone_create_workflow"), "missing whetstone_create_workflow");
    CHECK(hasTool(tools, "whetstone_orchestrate_run_deterministic"), "missing orchestrate tool");
    CHECK(hasTool(tools, "whetstone_get_event_stream"), "missing event stream tool");
    CHECK(hasTool(tools, "whetstone_get_review_queue"), "missing review queue tool");
    PASS();
}

void test_tool_names_are_unique() {
    TEST(tool_names_are_unique);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    std::set<std::string> seen;
    for (const auto& tool : doc["tools"]) {
        std::string name = tool.value("name", "");
        CHECK(!name.empty(), "tool name should not be empty");
        CHECK(seen.insert(name).second, "duplicate tool name found");
    }
    PASS();
}

void test_each_tool_has_input_schema_object() {
    TEST(each_tool_has_input_schema_object);
    json doc;
    std::string error;
    CHECK(loadToolsJson(&doc, &error), "tools.json should parse");
    for (const auto& tool : doc["tools"]) {
        CHECK(tool.contains("input_schema"), "input_schema missing");
        CHECK(tool["input_schema"].is_object(), "input_schema should be object");
    }
    PASS();
}

int main() {
    std::cout << "Step 622: tools.json refresh\n";

    test_tools_json_parses();                           // 1
    test_version_bumped_to_2_0();                       // 2
    test_provider_still_anthropic();                    // 3
    test_tools_array_has_broad_coverage();              // 4
    test_contains_new_sprint36_tools();                 // 5
    test_contains_sprint32_35_operational_tools();      // 6
    test_tool_names_are_unique();                       // 7
    test_each_tool_has_input_schema_object();           // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
