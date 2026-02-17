// Step 459: Phase 21b Integration + Sprint Summary Tests (8 tests)

#include "TranspilationRPC.h"

#include <iostream>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_end_to_end_semantic_transpile_python_to_rust() {
    TEST(end_to_end_semantic_transpile_python_to_rust);
    json transpileParams = {
        {"functionName", "sort_items"},
        {"intent", "sort list ascending"},
        {"source", "for i in range(n): ... swap(...)"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"}
    };
    auto transpile = TranspilationRPCHandler::handleTranspile(transpileParams);
    CHECK(transpile["idiomatic"].get<bool>(), "expected idiomatic transpile");
    CHECK(transpile["confidence"].get<float>() >= 0.90f, "expected high confidence");

    json confParams = {
        {"functionName", "sort_items"},
        {"sourceCode", transpile["sourceCode"]},
        {"targetCode", transpile["targetCode"]},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"hasIntent", true}
    };
    auto conf = TranspilationRPCHandler::handleGetConfidence(confParams);
    CHECK(conf["score"].get<float>() >= 0.90f, "expected high confidence score");
    PASS();
}

void test_translation_report_shows_idiomatic_choice_with_reasoning() {
    TEST(translation_report_shows_idiomatic_choice_with_reasoning);
    json params = {
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"functions", json::array({
            {{"name", "sort_items"}, {"intent", "sort list ascending"}, {"source", "sort loop"}}
        })}
    };
    auto report = TranspilationRPCHandler::handleGetTranslationReport(params);
    CHECK(report["functions"].size() == 1, "expected one function report");
    auto fn = report["functions"][0];
    CHECK(fn["idiomatic"].get<bool>(), "expected idiomatic choice");
    CHECK(fn["rationale"].get<std::string>().find("idiomatic") != std::string::npos,
          "expected rationale mentioning idiomatic mapping");
    PASS();
}

void test_low_confidence_translation_auto_flagged_for_review() {
    TEST(low_confidence_translation_auto_flagged_for_review);
    json confParams = {
        {"functionName", "mystery"},
        {"sourceCode", "custom operation"},
        {"targetCode", "TODO: translate manually"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"hasIntent", false}
    };
    auto conf = TranspilationRPCHandler::handleGetConfidence(confParams);
    CHECK(conf["reviewRequired"].get<bool>(), "expected review-required");
    CHECK(conf["score"].get<float>() <= 0.30f, "expected low confidence");
    PASS();
}

void test_equivalence_assertions_generated_and_non_empty() {
    TEST(equivalence_assertions_generated_and_non_empty);
    json eqParams = {
        {"functionName", "find_user"},
        {"sourceCode", "find element by key"},
        {"targetCode", "items.iter().find(|x| pred(x))"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"}
    };
    auto eq = TranspilationRPCHandler::handleVerifyEquivalence(eqParams);
    CHECK(eq["assertions"].size() >= 1, "expected generated assertions");
    CHECK(eq["annotation"].get<std::string>().find("@Contract(") != std::string::npos,
          "expected contract annotation");
    PASS();
}

void test_rpc_dispatch_full_flow_methods() {
    TEST(rpc_dispatch_full_flow_methods);
    auto a = TranspilationRPCHandler::dispatch(
        "transpile",
        {{"functionName", "f"}, {"intent", "sort list ascending"}, {"source", "sort loop"}},
        1);
    auto b = TranspilationRPCHandler::dispatch(
        "getConfidence",
        {{"functionName", "f"}, {"sourceCode", "sort loop"}, {"targetCode", "items.sort()"}, {"hasIntent", true}},
        2);
    auto c = TranspilationRPCHandler::dispatch(
        "verifyEquivalence",
        {{"functionName", "f"}, {"sourceCode", "sort values"}, {"targetCode", "items.sort()"}},
        3);
    CHECK(a.contains("result"), "transpile dispatch failed");
    CHECK(b.contains("result"), "confidence dispatch failed");
    CHECK(c.contains("result"), "equivalence dispatch failed");
    PASS();
}

void test_mcp_tool_surface_contains_all_phase21b_tools() {
    TEST(mcp_tool_surface_contains_all_phase21b_tools);
    auto defs = TranspilationRPCHandler::toolDefinitions();
    CHECK(defs.size() == 4, "expected 4 phase21b tools");
    bool hasTranspile = false, hasReport = false, hasEq = false, hasConf = false;
    for (const auto& d : defs) {
        if (d.name == "whetstone_transpile") hasTranspile = true;
        if (d.name == "whetstone_get_translation_report") hasReport = true;
        if (d.name == "whetstone_verify_equivalence") hasEq = true;
        if (d.name == "whetstone_get_confidence") hasConf = true;
    }
    CHECK(hasTranspile && hasReport && hasEq && hasConf, "missing one or more MCP tools");
    PASS();
}

void test_report_summary_percentages_are_consistent() {
    TEST(report_summary_percentages_are_consistent);
    json params = {
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"functions", json::array({
            {{"name", "sort_items"}, {"intent", "sort list ascending"}, {"source", "sort loop"}},
            {{"name", "mystery"}, {"intent", ""}, {"source", "x = a + b"}}
        })}
    };
    auto report = TranspilationRPCHandler::handleGetTranslationReport(params);
    float idiomaticPercent = report["summary"]["idiomaticPercent"].get<float>();
    float literalPercent = report["summary"]["literalPercent"].get<float>();
    CHECK(idiomaticPercent > 49.0f && idiomaticPercent < 51.0f, "expected ~50% idiomatic");
    CHECK(literalPercent > 49.0f && literalPercent < 51.0f, "expected ~50% literal");
    PASS();
}

void test_unknown_dispatch_reports_method_error() {
    TEST(unknown_dispatch_reports_method_error);
    auto r = TranspilationRPCHandler::dispatch("unknown", json::object(), 9);
    CHECK(r.contains("error"), "expected error on unknown method");
    CHECK(r["error"]["code"] == -32601, "expected method-not-found code");
    PASS();
}

int main() {
    std::cout << "Step 459: Phase 21b Integration Tests\n";

    test_end_to_end_semantic_transpile_python_to_rust();           // 1
    test_translation_report_shows_idiomatic_choice_with_reasoning(); // 2
    test_low_confidence_translation_auto_flagged_for_review();     // 3
    test_equivalence_assertions_generated_and_non_empty();         // 4
    test_rpc_dispatch_full_flow_methods();                         // 5
    test_mcp_tool_surface_contains_all_phase21b_tools();           // 6
    test_report_summary_percentages_are_consistent();              // 7
    test_unknown_dispatch_reports_method_error();                  // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
