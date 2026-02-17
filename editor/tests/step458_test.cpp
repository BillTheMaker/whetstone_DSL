// Step 458: Transpilation RPC + MCP Tests (12 tests)

#include "TranspilationRPC.h"

#include <iostream>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_can_handle_all_methods() {
    TEST(can_handle_all_methods);
    CHECK(TranspilationRPCHandler::canHandle("transpile"), "missing transpile");
    CHECK(TranspilationRPCHandler::canHandle("getTranslationReport"), "missing getTranslationReport");
    CHECK(TranspilationRPCHandler::canHandle("verifyEquivalence"), "missing verifyEquivalence");
    CHECK(TranspilationRPCHandler::canHandle("getConfidence"), "missing getConfidence");
    PASS();
}

void test_handle_transpile_returns_target_code() {
    TEST(handle_transpile_returns_target_code);
    json params = {
        {"functionName", "sort_items"},
        {"intent", "sort list ascending"},
        {"source", "items.sort()"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"}
    };
    auto r = TranspilationRPCHandler::handleTranspile(params);
    CHECK(r["functionName"] == "sort_items", "wrong function name");
    CHECK(r["targetCode"].get<std::string>().find("sort") != std::string::npos,
          "expected sort mapping");
    CHECK(r["idiomatic"].get<bool>(), "expected idiomatic translation");
    PASS();
}

void test_handle_translation_report_returns_summary() {
    TEST(handle_translation_report_returns_summary);
    json params = {
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"functions", json::array({
            {{"name", "sort_items"}, {"intent", "sort list ascending"}, {"source", "items.sort()"}},
            {{"name", "mystery"}, {"intent", ""}, {"source", "x = a + b"}}
        })}
    };
    auto r = TranspilationRPCHandler::handleGetTranslationReport(params);
    CHECK(r.contains("summary"), "missing summary");
    CHECK(r["summary"]["totalFunctions"] == 2, "wrong function count");
    CHECK(r["functions"].size() == 2, "wrong function report count");
    PASS();
}

void test_handle_verify_equivalence_returns_assertions() {
    TEST(handle_verify_equivalence_returns_assertions);
    json params = {
        {"functionName", "sort_items"},
        {"sourceCode", "sort values"},
        {"targetCode", "items.sort()"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"}
    };
    auto r = TranspilationRPCHandler::handleVerifyEquivalence(params);
    CHECK(r["functionName"] == "sort_items", "wrong function");
    CHECK(r["assertions"].size() >= 1, "expected assertions");
    CHECK(r.contains("annotation"), "missing annotation");
    PASS();
}

void test_handle_get_confidence_returns_score() {
    TEST(handle_get_confidence_returns_score);
    json params = {
        {"functionName", "sort_items"},
        {"sourceCode", "sort values"},
        {"targetCode", "items.sort()"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"hasIntent", true}
    };
    auto r = TranspilationRPCHandler::handleGetConfidence(params);
    CHECK(r["score"].get<float>() >= 0.90f, "expected high confidence");
    CHECK(!r["reviewRequired"].get<bool>(), "high confidence should not need review");
    PASS();
}

void test_dispatch_wraps_result_in_jsonrpc_shape() {
    TEST(dispatch_wraps_result_in_jsonrpc_shape);
    json params = {
        {"functionName", "sort_items"},
        {"intent", "sort list ascending"},
        {"source", "items.sort()"}
    };
    auto r = TranspilationRPCHandler::dispatch("transpile", params, 7);
    CHECK(r["jsonrpc"] == "2.0", "wrong jsonrpc version");
    CHECK(r["id"] == 7, "wrong id");
    CHECK(r.contains("result"), "missing result");
    PASS();
}

void test_dispatch_unknown_method_returns_error() {
    TEST(dispatch_unknown_method_returns_error);
    auto r = TranspilationRPCHandler::dispatch("unknownMethod", json::object(), 8);
    CHECK(r.contains("error"), "missing error");
    CHECK(r["error"]["code"] == -32601, "wrong error code");
    PASS();
}

void test_tool_definitions_contains_four_tools() {
    TEST(tool_definitions_contains_four_tools);
    auto defs = TranspilationRPCHandler::toolDefinitions();
    CHECK(defs.size() == 4, "expected 4 tool defs");
    CHECK(defs[0].name == "whetstone_transpile", "first tool mismatch");
    CHECK(defs[1].name == "whetstone_get_translation_report", "second tool mismatch");
    PASS();
}

void test_tool_schemas_have_required_fields() {
    TEST(tool_schemas_have_required_fields);
    auto defs = TranspilationRPCHandler::toolDefinitions();
    CHECK(defs[0].inputSchema.contains("required"), "missing required on transpile schema");
    CHECK(defs[2].inputSchema.contains("required"), "missing required on equivalence schema");
    PASS();
}

void test_transpile_rpc_without_intent_marks_review() {
    TEST(transpile_rpc_without_intent_marks_review);
    json params = {
        {"functionName", "mystery"},
        {"intent", ""},
        {"source", "x = a + b"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"}
    };
    auto r = TranspilationRPCHandler::handleTranspile(params);
    CHECK(r["needsReview"].get<bool>(), "missing review flag for no-intent case");
    PASS();
}

void test_confidence_rpc_low_score_requests_review() {
    TEST(confidence_rpc_low_score_requests_review);
    json params = {
        {"functionName", "unknown"},
        {"sourceCode", "custom op"},
        {"targetCode", "TODO: translate"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "rust"},
        {"hasIntent", false}
    };
    auto r = TranspilationRPCHandler::handleGetConfidence(params);
    CHECK(r["score"].get<float>() <= 0.30f, "expected low confidence");
    CHECK(r["reviewRequired"].get<bool>(), "expected review requirement");
    PASS();
}

void test_equivalence_rpc_uses_custom_inputs() {
    TEST(equivalence_rpc_uses_custom_inputs);
    json params = {
        {"functionName", "custom"},
        {"sourceCode", "custom op"},
        {"targetCode", "custom_op(data)"},
        {"sourceLanguage", "python"},
        {"targetLanguage", "java"},
        {"sampleInputs", json::array({"A", "B"})}
    };
    auto r = TranspilationRPCHandler::handleVerifyEquivalence(params);
    CHECK(r["assertions"].size() == 2, "expected two custom assertions");
    PASS();
}

int main() {
    std::cout << "Step 458: Transpilation RPC + MCP Tests\n";

    test_can_handle_all_methods();                    // 1
    test_handle_transpile_returns_target_code();      // 2
    test_handle_translation_report_returns_summary(); // 3
    test_handle_verify_equivalence_returns_assertions(); // 4
    test_handle_get_confidence_returns_score();       // 5
    test_dispatch_wraps_result_in_jsonrpc_shape();    // 6
    test_dispatch_unknown_method_returns_error();     // 7
    test_tool_definitions_contains_four_tools();      // 8
    test_tool_schemas_have_required_fields();         // 9
    test_transpile_rpc_without_intent_marks_review(); // 10
    test_confidence_rpc_low_score_requests_review();  // 11
    test_equivalence_rpc_uses_custom_inputs();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
