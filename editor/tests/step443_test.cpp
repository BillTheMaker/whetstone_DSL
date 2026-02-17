// Step 443: Phase 20a Integration Tests (8 tests)
// Full pipeline: legacy C → analyze → safety audit → suggest → workflow → validate

#include "ModernizationRPC.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Representative legacy C source with multiple issues
static const std::string LEGACY_C_SOURCE = R"(
static int request_count = 0;

int process_request(buf, len)
int buf;
int len;
{
    char response[64];
    int* data = (int*)malloc(len * sizeof(int));

    gets(response);
    sprintf(response, "Received: %d", buf);
    strcpy(response, "hello");

    goto cleanup;

    request_count++;
    unsigned int total = request_count + len;

cleanup:
    free(data);
    *data = 0;
    return 0;
}
)";

void test_full_pipeline_legacy_to_workflow() {
    TEST(full_pipeline_legacy_to_workflow);
    auto legacy = LegacyIdiomDetector::analyzeFile(LEGACY_C_SOURCE, "c", "server.c");
    auto safety = SafetyAuditor::audit(LEGACY_C_SOURCE, "c", "server.c");
    auto suggestions = ModernizationSuggester::suggest(legacy, safety);
    auto wf = ModernizationWorkflowGenerator::generate(LEGACY_C_SOURCE, suggestions);

    // Legacy analysis should find multiple patterns
    CHECK(legacy.legacyScore >= 5, "expected high legacy score for this code");
    CHECK(legacy.hasPattern("knr-declaration"), "expected K&R pattern");
    CHECK(legacy.hasPattern("goto-control-flow"), "expected goto pattern");
    CHECK(legacy.hasPattern("deprecated-api-gets"), "expected gets pattern");
    CHECK(legacy.hasPattern("manual-memory-management"), "expected manual memory");

    // Workflow should have items covering all findings
    CHECK(wf.items.size() >= 5, "expected at least 5 work items");
    PASS();
}

void test_safety_report_flags_real_issues() {
    TEST(safety_report_flags_real_issues);
    auto safety = SafetyAuditor::audit(LEGACY_C_SOURCE, "c", "server.c");

    CHECK(safety.hasCategory("buffer-overflow"), "expected buffer-overflow (gets/sprintf/strcpy)");
    CHECK(safety.hasCategory("use-after-free"), "expected use-after-free (free then *data)");
    CHECK(safety.overallRisk >= 3, "expected high overall risk");

    // CWE mappings should be present
    CHECK(safety.hasCwe("CWE-120"), "expected CWE-120 for buffer overflow");
    CHECK(safety.hasCwe("CWE-416"), "expected CWE-416 for use-after-free");
    PASS();
}

void test_workflow_respects_risk_ordering() {
    TEST(workflow_respects_risk_ordering);
    auto legacy = LegacyIdiomDetector::analyzeFile(LEGACY_C_SOURCE, "c", "server.c");
    auto safety = SafetyAuditor::audit(LEGACY_C_SOURCE, "c", "server.c");
    auto suggestions = ModernizationSuggester::suggest(legacy, safety);
    auto wf = ModernizationWorkflowGenerator::generate(LEGACY_C_SOURCE, suggestions);

    auto ordered = wf.itemsInOrder();
    CHECK(ordered.size() >= 3, "expected items in order");

    // First items should be safe quick wins (priority 0)
    CHECK(ordered.front().priority == 0, "first item should be priority 0 (quick win)");
    // Last items should be risky deep refactors (priority 2)
    CHECK(ordered.back().priority >= 1, "last item should be higher priority (risky)");

    // Quick wins should be deterministic
    for (const auto& item : ordered) {
        if (item.effort == ModernizeEffort::QuickWin)
            CHECK(item.routing == WorkItemRouting::Deterministic,
                  "quick wins should route deterministic");
    }
    PASS();
}

void test_cross_language_c_to_rust_modernization() {
    TEST(cross_language_c_to_rust_modernization);
    auto legacy = LegacyIdiomDetector::analyzeFile(LEGACY_C_SOURCE, "c", "server.c");
    auto safety = SafetyAuditor::audit(LEGACY_C_SOURCE, "c", "server.c");
    auto suggestions = ModernizationSuggester::suggest(legacy, safety, "rust");

    CHECK(suggestions.targetLanguage == "rust", "expected rust target");
    // Should include Rust-specific ownership suggestion
    bool foundRust = false;
    for (const auto& s : suggestions.suggestions)
        if (s.toReplacement.find("Rust") != std::string::npos) foundRust = true;
    CHECK(foundRust, "expected Rust ownership suggestion for C→Rust migration");
    PASS();
}

void test_rpc_full_pipeline_via_json() {
    TEST(rpc_full_pipeline_via_json);
    json params = {{"source", LEGACY_C_SOURCE}, {"language", "c"}, {"filePath", "server.c"}};

    // Step 1: analyze legacy
    auto legacy = ModernizationRPCHandler::handleAnalyzeLegacy(params);
    CHECK(legacy["legacyScore"].get<int>() >= 5, "RPC legacy score too low");

    // Step 2: safety report
    auto safety = ModernizationRPCHandler::handleGetSafetyReport(params);
    CHECK(safety["overallRisk"].get<int>() >= 3, "RPC safety risk too low");

    // Step 3: suggest
    auto suggest = ModernizationRPCHandler::handleSuggestModernization(params);
    CHECK(!suggest["suggestions"].empty(), "RPC suggestions empty");

    // Step 4: create workflow
    auto wf = ModernizationRPCHandler::handleCreateWorkflow(params);
    CHECK(wf["itemCount"].get<int>() >= 5, "RPC workflow too few items");
    CHECK(wf["skeleton"]["source"].is_string(), "RPC skeleton missing");
    PASS();
}

void test_deterministic_modernizations_identified() {
    TEST(deterministic_modernizations_identified);
    json params = {{"source", LEGACY_C_SOURCE}, {"language", "c"}, {"filePath", "server.c"}};
    auto wf = ModernizationRPCHandler::handleCreateWorkflow(params);

    int det = wf["deterministicCount"].get<int>();
    CHECK(det >= 3, "expected at least 3 deterministic (quick-win) items");

    // Verify deterministic items are API replacements
    for (const auto& item : wf["items"]) {
        if (item["routing"] == "deterministic") {
            CHECK(item["effort"] == "quick_win", "deterministic items should be quick wins");
        }
    }
    PASS();
}

void test_complex_items_prepared_for_llm_or_human() {
    TEST(complex_items_prepared_for_llm_or_human);
    json params = {{"source", LEGACY_C_SOURCE}, {"language", "c"}, {"filePath", "server.c"}};
    auto wf = ModernizationRPCHandler::handleCreateWorkflow(params);

    int llm = wf["llmCount"].get<int>();
    int human = wf["humanCount"].get<int>();
    CHECK(llm + human >= 1, "expected at least 1 LLM or human routed item");

    // Deep refactors and high-risk items should go to human
    for (const auto& item : wf["items"]) {
        if (item["effort"] == "deep_refactor")
            CHECK(item["routing"] == "human", "deep refactors should route to human");
    }
    PASS();
}

void test_language_version_detection_in_pipeline() {
    TEST(language_version_detection_in_pipeline);
    json params = {{"source", LEGACY_C_SOURCE}, {"language", "c"}, {"filePath", "server.c"}};
    auto legacy = ModernizationRPCHandler::handleAnalyzeLegacy(params);

    // K&R declarations → should detect as c89
    CHECK(legacy["languageVersion"] == "c89", "expected c89 for K&R code");
    PASS();
}

int main() {
    std::cout << "Step 443: Phase 20a Integration Tests\n";

    test_full_pipeline_legacy_to_workflow();           // 1
    test_safety_report_flags_real_issues();            // 2
    test_workflow_respects_risk_ordering();             // 3
    test_cross_language_c_to_rust_modernization();     // 4
    test_rpc_full_pipeline_via_json();                 // 5
    test_deterministic_modernizations_identified();     // 6
    test_complex_items_prepared_for_llm_or_human();    // 7
    test_language_version_detection_in_pipeline();      // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
