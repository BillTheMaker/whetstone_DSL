// Step 442: Modernization RPC + MCP Tests (12 tests)

#include "ModernizationRPC.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_rpc_analyze_legacy() {
    TEST(rpc_analyze_legacy);
    json params = {{"source", "void f(){ gets(buf); }"}, {"language", "c"}, {"filePath", "a.c"}};
    auto result = ModernizationRPCHandler::handleAnalyzeLegacy(params);
    CHECK(result.contains("legacyScore"), "expected legacyScore");
    CHECK(result["legacyScore"].get<int>() > 0, "expected non-zero legacy score");
    CHECK(result["findings"].is_array(), "expected findings array");
    CHECK(!result["findings"].empty(), "expected at least one finding");
    PASS();
}

void test_rpc_safety_report() {
    TEST(rpc_safety_report);
    json params = {{"source", "void f(){ char b[32]; gets(b); }"},
                   {"language", "c"}, {"filePath", "b.c"}};
    auto result = ModernizationRPCHandler::handleGetSafetyReport(params);
    CHECK(result.contains("overallRisk"), "expected overallRisk");
    CHECK(result["overallRisk"].get<int>() >= 3, "expected high risk for gets");
    CHECK(result["findings"].is_array(), "expected findings array");
    bool foundCwe = false;
    for (const auto& f : result["findings"])
        if (f["cwe"] == "CWE-120") foundCwe = true;
    CHECK(foundCwe, "expected CWE-120 in findings");
    PASS();
}

void test_rpc_suggest_modernization() {
    TEST(rpc_suggest_modernization);
    json params = {{"source", "void f(){ int* p=(int*)malloc(4); free(p); }"},
                   {"language", "cpp"}, {"filePath", "c.cpp"}};
    auto result = ModernizationRPCHandler::handleSuggestModernization(params);
    CHECK(result.contains("suggestions"), "expected suggestions");
    CHECK(!result["suggestions"].empty(), "expected at least one suggestion");
    CHECK(result.contains("quickWinCount"), "expected quickWinCount");
    PASS();
}

void test_rpc_suggest_with_target_language() {
    TEST(rpc_suggest_with_target_language);
    json params = {{"source", "void f(){ int* p=(int*)malloc(4); free(p); }"},
                   {"language", "c"}, {"filePath", "d.c"},
                   {"targetLanguage", "rust"}};
    auto result = ModernizationRPCHandler::handleSuggestModernization(params);
    CHECK(result["targetLanguage"] == "rust", "expected rust target");
    bool foundRust = false;
    for (const auto& s : result["suggestions"])
        if (s["to"].get<std::string>().find("Rust") != std::string::npos) foundRust = true;
    CHECK(foundRust, "expected Rust-specific suggestion");
    PASS();
}

void test_rpc_create_workflow() {
    TEST(rpc_create_workflow);
    json params = {{"source", "void f(){ gets(b); sprintf(b,\"x\"); goto end; end:; }"},
                   {"language", "c"}, {"filePath", "e.c"}};
    auto result = ModernizationRPCHandler::handleCreateWorkflow(params);
    CHECK(result.contains("items"), "expected items");
    CHECK(result["items"].is_array(), "expected items array");
    CHECK(result["itemCount"].get<int>() >= 3, "expected at least 3 items");
    CHECK(result.contains("deterministicCount"), "expected deterministicCount");
    CHECK(result.contains("skeleton"), "expected skeleton");
    PASS();
}

void test_rpc_workflow_routing_counts() {
    TEST(rpc_workflow_routing_counts);
    json params = {{"source", "void f(){ gets(b); goto end; end:; }"},
                   {"language", "c"}, {"filePath", "f.c"}};
    auto result = ModernizationRPCHandler::handleCreateWorkflow(params);
    int det = result["deterministicCount"].get<int>();
    int human = result["humanCount"].get<int>();
    CHECK(det >= 1, "expected at least 1 deterministic item");
    CHECK(human >= 1, "expected at least 1 human item");
    PASS();
}

void test_rpc_dispatch_routes_correctly() {
    TEST(rpc_dispatch_routes_correctly);
    json params = {{"source", "void f(){ gets(b); }"}, {"language", "c"}};
    json id = 42;

    auto r1 = ModernizationRPCHandler::dispatch("analyzeLegacy", params, id);
    CHECK(r1["id"] == 42, "expected id preserved");
    CHECK(r1.contains("result"), "expected result for analyzeLegacy");

    auto r2 = ModernizationRPCHandler::dispatch("getSafetyReport", params, id);
    CHECK(r2.contains("result"), "expected result for getSafetyReport");

    auto r3 = ModernizationRPCHandler::dispatch("suggestModernization", params, id);
    CHECK(r3.contains("result"), "expected result for suggestModernization");

    auto r4 = ModernizationRPCHandler::dispatch("createModernizationWorkflow", params, id);
    CHECK(r4.contains("result"), "expected result for createModernizationWorkflow");
    PASS();
}

void test_rpc_dispatch_unknown_method() {
    TEST(rpc_dispatch_unknown_method);
    auto r = ModernizationRPCHandler::dispatch("nonexistent", {}, 1);
    CHECK(r.contains("error"), "expected error for unknown method");
    PASS();
}

void test_can_handle() {
    TEST(can_handle);
    CHECK(ModernizationRPCHandler::canHandle("analyzeLegacy"), "analyzeLegacy");
    CHECK(ModernizationRPCHandler::canHandle("getSafetyReport"), "getSafetyReport");
    CHECK(ModernizationRPCHandler::canHandle("suggestModernization"), "suggestModernization");
    CHECK(ModernizationRPCHandler::canHandle("createModernizationWorkflow"), "createModernizationWorkflow");
    CHECK(!ModernizationRPCHandler::canHandle("getAST"), "getAST should not match");
    PASS();
}

void test_tool_definitions_count() {
    TEST(tool_definitions_count);
    auto tools = ModernizationRPCHandler::toolDefinitions();
    CHECK(tools.size() == 4, "expected 4 tool definitions");
    // Verify names
    bool foundAnalyze = false, foundSafety = false, foundSuggest = false, foundWorkflow = false;
    for (const auto& t : tools) {
        if (t.name == "whetstone_analyze_legacy") foundAnalyze = true;
        if (t.name == "whetstone_get_safety_report") foundSafety = true;
        if (t.name == "whetstone_suggest_modernization") foundSuggest = true;
        if (t.name == "whetstone_create_modernization_workflow") foundWorkflow = true;
    }
    CHECK(foundAnalyze, "expected whetstone_analyze_legacy");
    CHECK(foundSafety, "expected whetstone_get_safety_report");
    CHECK(foundSuggest, "expected whetstone_suggest_modernization");
    CHECK(foundWorkflow, "expected whetstone_create_modernization_workflow");
    PASS();
}

void test_tool_schemas_have_source_property() {
    TEST(tool_schemas_have_source_property);
    auto tools = ModernizationRPCHandler::toolDefinitions();
    for (const auto& t : tools) {
        CHECK(t.inputSchema.contains("properties"), "schema missing properties");
        CHECK(t.inputSchema["properties"].contains("source"), "schema missing source property");
    }
    PASS();
}

void test_json_serialization_roundtrip() {
    TEST(json_serialization_roundtrip);
    json params = {
        {"source", "void f(){ int* p=(int*)malloc(4); free(p); *p = 1; }"},
        {"language", "c"}, {"filePath", "rt.c"}
    };
    auto result = ModernizationRPCHandler::handleCreateWorkflow(params);
    // Verify JSON is well-formed by serializing and re-parsing
    std::string serialized = result.dump();
    json reparsed = json::parse(serialized);
    CHECK(reparsed["filePath"] == "rt.c", "roundtrip filePath");
    CHECK(reparsed["items"].is_array(), "roundtrip items");
    CHECK(reparsed["skeleton"]["source"].is_string(), "roundtrip skeleton source");
    PASS();
}

int main() {
    std::cout << "Step 442: Modernization RPC + MCP Tests\n";

    test_rpc_analyze_legacy();                   // 1
    test_rpc_safety_report();                    // 2
    test_rpc_suggest_modernization();            // 3
    test_rpc_suggest_with_target_language();      // 4
    test_rpc_create_workflow();                   // 5
    test_rpc_workflow_routing_counts();           // 6
    test_rpc_dispatch_routes_correctly();         // 7
    test_rpc_dispatch_unknown_method();           // 8
    test_can_handle();                           // 9
    test_tool_definitions_count();               // 10
    test_tool_schemas_have_source_property();     // 11
    test_json_serialization_roundtrip();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
