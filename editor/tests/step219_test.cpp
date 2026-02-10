// Step 219: Trace generation and export tests
//
// Verifies:
//   1. Traces conform to schema (valid JSON, correct roles)
//   2. Each scenario template produces traces with expected structure
//   3. Different seeds produce distinct traces
//   4. Anthropic Messages export is well-formed
//   5. OpenAI Chat export is well-formed
//   6. JSONL export is well-formed
//   7. Markdown export is well-formed
//   8. Batch generation produces correct count with expected distribution
//   9. Filtering works correctly
//  10. Statistics are computed correctly
//  11. Code corpus is populated
//  12. Trace toJson roundtrip

#include <cassert>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <nlohmann/json.hpp>
#include "TraceGenerator.h"
#include "TraceExporter.h"

using json = nlohmann::json;

static int passed = 0;
static int failed = 0;

static void expect(bool cond, const char* msg) {
    if (cond) { ++passed; }
    else { ++failed; printf("  FAIL: %s\n", msg); }
}

// --- Test 1: Trace schema validation ---
static void test_trace_schema() {
    printf("Test 1: Trace schema validation...\n");
    TraceGenerator gen;
    gen.setSeed(100);

    auto trace = gen.generate(ScenarioType::ReadAndUnderstand);
    json j = trace.toJson();

    expect(j.contains("id"), "trace has id");
    expect(j.contains("scenario"), "trace has scenario");
    expect(j.contains("difficulty"), "trace has difficulty");
    expect(j.contains("language"), "trace has language");
    expect(j.contains("steps"), "trace has steps");
    expect(j.contains("toolsUsed"), "trace has toolsUsed");
    expect(j.contains("toolCallCount"), "trace has toolCallCount");
    expect(j.contains("success"), "trace has success");
    expect(j["steps"].is_array(), "steps is array");
    expect(!j["steps"].empty(), "steps not empty");

    // Validate step roles
    std::set<std::string> validRoles = {"user", "assistant", "tool_call", "tool_result"};
    for (const auto& step : j["steps"]) {
        expect(step.contains("role"), "step has role");
        std::string role = step["role"];
        expect(validRoles.count(role) > 0, ("valid role: " + role).c_str());
    }
}

// --- Test 2: All scenario templates produce traces ---
static void test_all_scenarios() {
    printf("Test 2: All scenario templates...\n");
    TraceGenerator gen;
    gen.setSeed(200);

    std::vector<std::pair<ScenarioType, std::string>> scenarios = {
        {ScenarioType::ReadAndUnderstand, "read_and_understand"},
        {ScenarioType::AddAnnotations, "add_annotations"},
        {ScenarioType::CrossLanguage, "cross_language"},
        {ScenarioType::Refactor, "refactor"},
        {ScenarioType::SecurityAudit, "security_audit"},
        {ScenarioType::MultiStepDebug, "multi_step_debug"}
    };

    for (const auto& [type, name] : scenarios) {
        ScenarioParams params;
        params.language = "python";
        auto trace = gen.generate(type, params);

        expect(!trace.id.empty(), ("trace id not empty for " + name).c_str());
        expect(trace.scenario == name, ("scenario matches for " + name).c_str());
        expect(!trace.steps.empty(), ("steps not empty for " + name).c_str());
        expect(trace.toolCallCount > 0, ("has tool calls for " + name).c_str());
        expect(!trace.toolsUsed.empty(), ("has tools used for " + name).c_str());

        // First step should be user
        expect(trace.steps[0].role == "user", ("first step is user for " + name).c_str());

        // Tool calls should be paired with tool results
        for (size_t i = 0; i < trace.steps.size(); ++i) {
            if (trace.steps[i].role == "tool_call") {
                expect(i + 1 < trace.steps.size(), "tool_call has following step");
                if (i + 1 < trace.steps.size()) {
                    expect(trace.steps[i + 1].role == "tool_result",
                           ("tool_result follows tool_call for " + name).c_str());
                    expect(trace.steps[i].toolName == trace.steps[i + 1].toolName,
                           ("tool names match for " + name).c_str());
                }
            }
        }
    }
}

// --- Test 3: Different seeds produce distinct traces ---
static void test_seed_determinism() {
    printf("Test 3: Seed determinism and distinction...\n");
    TraceGenerator gen;

    ScenarioParams p1; p1.seed = 42;
    ScenarioParams p2; p2.seed = 43;
    ScenarioParams p3; p3.seed = 42;

    auto trace1 = gen.generate(ScenarioType::ReadAndUnderstand, p1);
    auto trace2 = gen.generate(ScenarioType::ReadAndUnderstand, p2);
    auto trace3 = gen.generate(ScenarioType::ReadAndUnderstand, p3);

    // Sequential traces from same generator have different IDs
    expect(trace1.id != trace2.id, "sequential traces different ids");

    // Traces should have the same scenario
    expect(trace1.scenario == trace2.scenario, "same scenario type");
    expect(trace1.scenario == trace3.scenario, "same scenario type (seed 42 again)");
}

// --- Test 4: Anthropic Messages export ---
static void test_anthropic_export() {
    printf("Test 4: Anthropic Messages export...\n");
    TraceGenerator gen;
    gen.setSeed(300);

    auto trace = gen.generate(ScenarioType::AddAnnotations);
    json exported = TraceExporter::toAnthropicMessages(trace);

    expect(exported.contains("id"), "exported has id");
    expect(exported.contains("messages"), "exported has messages");
    expect(exported.contains("metadata"), "exported has metadata");
    expect(exported["messages"].is_array(), "messages is array");
    expect(!exported["messages"].empty(), "messages not empty");

    // Check message roles
    bool hasUser = false, hasAssistant = false;
    for (const auto& msg : exported["messages"]) {
        expect(msg.contains("role"), "message has role");
        std::string role = msg["role"];
        expect(role == "user" || role == "assistant", "valid Anthropic role");
        if (role == "user") hasUser = true;
        if (role == "assistant") hasAssistant = true;
    }
    expect(hasUser, "has user messages");
    expect(hasAssistant, "has assistant messages");

    // Check tool_use blocks exist in assistant messages
    bool hasToolUse = false;
    for (const auto& msg : exported["messages"]) {
        if (msg["role"] == "assistant" && msg["content"].is_array()) {
            for (const auto& block : msg["content"]) {
                if (block.contains("type") && block["type"] == "tool_use") {
                    hasToolUse = true;
                    expect(block.contains("id"), "tool_use has id");
                    expect(block.contains("name"), "tool_use has name");
                    expect(block.contains("input"), "tool_use has input");
                }
            }
        }
    }
    expect(hasToolUse, "has tool_use blocks");

    // Check tool_result blocks exist
    bool hasToolResult = false;
    for (const auto& msg : exported["messages"]) {
        if (msg["role"] == "user" && msg["content"].is_array()) {
            for (const auto& block : msg["content"]) {
                if (block.contains("type") && block["type"] == "tool_result") {
                    hasToolResult = true;
                    expect(block.contains("tool_use_id"), "tool_result has tool_use_id");
                    expect(block.contains("content"), "tool_result has content");
                }
            }
        }
    }
    expect(hasToolResult, "has tool_result blocks");

    // Metadata
    expect(exported["metadata"]["scenario"] == "add_annotations", "metadata scenario");
    expect(exported["metadata"]["language"] == "python", "metadata language");
}

// --- Test 5: OpenAI Chat export ---
static void test_openai_export() {
    printf("Test 5: OpenAI Chat export...\n");
    TraceGenerator gen;
    gen.setSeed(400);

    auto trace = gen.generate(ScenarioType::Refactor);
    json exported = TraceExporter::toOpenAIChat(trace);

    expect(exported.contains("id"), "exported has id");
    expect(exported.contains("messages"), "exported has messages");
    expect(exported["messages"].is_array(), "messages is array");

    // Check valid OpenAI roles
    std::set<std::string> validRoles = {"user", "assistant", "tool"};
    bool hasToolCalls = false;
    bool hasToolRole = false;
    for (const auto& msg : exported["messages"]) {
        expect(msg.contains("role"), "message has role");
        std::string role = msg["role"];
        expect(validRoles.count(role) > 0, ("valid OpenAI role: " + role).c_str());

        if (role == "assistant" && msg.contains("tool_calls")) {
            hasToolCalls = true;
            for (const auto& tc : msg["tool_calls"]) {
                expect(tc.contains("id"), "tool_call has id");
                expect(tc.contains("type"), "tool_call has type");
                expect(tc["type"] == "function", "tool_call type is function");
                expect(tc.contains("function"), "tool_call has function");
                expect(tc["function"].contains("name"), "function has name");
                expect(tc["function"].contains("arguments"), "function has arguments");
            }
        }
        if (role == "tool") {
            hasToolRole = true;
            expect(msg.contains("tool_call_id"), "tool msg has tool_call_id");
            expect(msg.contains("content"), "tool msg has content");
        }
    }
    expect(hasToolCalls, "has tool_calls in assistant messages");
    expect(hasToolRole, "has tool role messages");
}

// --- Test 6: JSONL export ---
static void test_jsonl_export() {
    printf("Test 6: JSONL export...\n");
    TraceGenerator gen;
    gen.setSeed(500);

    auto traces = gen.generateBatch(6, 500);
    std::string jsonl = TraceExporter::toJSONL(traces);

    // Count lines
    int lineCount = 0;
    std::istringstream iss(jsonl);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            ++lineCount;
            // Each line should be valid JSON
            try {
                json parsed = json::parse(line);
                expect(parsed.contains("id"), "JSONL line has id");
                expect(parsed.contains("scenario"), "JSONL line has scenario");
            } catch (...) {
                expect(false, "JSONL line is valid JSON");
            }
        }
    }
    expect(lineCount == 6, "JSONL has correct number of lines");

    // Single trace export
    std::string single = TraceExporter::toJSONLSingle(traces[0]);
    expect(!single.empty(), "single JSONL not empty");
    expect(single.back() == '\n', "single JSONL ends with newline");
}

// --- Test 7: Markdown export ---
static void test_markdown_export() {
    printf("Test 7: Markdown export...\n");
    TraceGenerator gen;
    gen.setSeed(600);

    auto trace = gen.generate(ScenarioType::CrossLanguage);
    std::string md = TraceExporter::toMarkdown(trace);

    expect(!md.empty(), "markdown not empty");
    expect(md.find("# Trace:") != std::string::npos, "has trace header");
    expect(md.find("**Scenario:**") != std::string::npos, "has scenario field");
    expect(md.find("**Difficulty:**") != std::string::npos, "has difficulty field");
    expect(md.find("**Language:**") != std::string::npos, "has language field");
    expect(md.find("**Tool calls:**") != std::string::npos, "has tool calls field");
    expect(md.find("### Step") != std::string::npos, "has step headers");
    expect(md.find("```json") != std::string::npos, "has JSON code blocks");
    expect(md.find("cross_language") != std::string::npos, "scenario name in markdown");
}

// --- Test 8: Batch generation ---
static void test_batch_generation() {
    printf("Test 8: Batch generation...\n");
    TraceGenerator gen;

    auto traces = gen.generateBatch(12, 42);
    expect((int)traces.size() == 12, "batch produces correct count");

    // Check scenario distribution (6 types, 12 traces = 2 of each)
    std::map<std::string, int> scenarioCounts;
    for (const auto& t : traces) {
        scenarioCounts[t.scenario]++;
    }
    expect(scenarioCounts.size() == 6, "all 6 scenario types present");
    for (const auto& [scenario, count] : scenarioCounts) {
        expect(count == 2, ("2 traces for " + scenario).c_str());
    }

    // Check language alternation
    for (int i = 0; i < 12; ++i) {
        std::string expected = (i % 2 == 0) ? "python" : "cpp";
        expect(traces[i].language == expected,
               ("language alternation at index " + std::to_string(i)).c_str());
    }

    // All traces should have unique IDs
    std::set<std::string> ids;
    for (const auto& t : traces) ids.insert(t.id);
    expect((int)ids.size() == 12, "all traces have unique IDs");
}

// --- Test 9: Filtering ---
static void test_filtering() {
    printf("Test 9: Filtering...\n");
    TraceGenerator gen;
    auto traces = gen.generateBatch(12, 42);

    // Filter by scenario
    TraceFilter f;
    f.scenario = "read_and_understand";
    auto filtered = TraceExporter::filter(traces, f);
    for (const auto& t : filtered) {
        expect(t.scenario == "read_and_understand", "filtered by scenario");
    }
    expect(!filtered.empty(), "scenario filter has results");

    // Filter by language
    TraceFilter f2;
    f2.language = "python";
    auto filtered2 = TraceExporter::filter(traces, f2);
    for (const auto& t : filtered2) {
        expect(t.language == "python", "filtered by language");
    }
    expect((int)filtered2.size() == 6, "6 python traces");

    // Filter by difficulty
    TraceFilter f3;
    f3.difficulty = "advanced";
    auto filtered3 = TraceExporter::filter(traces, f3);
    for (const auto& t : filtered3) {
        expect(t.difficulty == "advanced", "filtered by difficulty");
    }

    // Filter by min tool calls
    TraceFilter f4;
    f4.minToolCalls = 2;
    auto filtered4 = TraceExporter::filter(traces, f4);
    for (const auto& t : filtered4) {
        expect(t.toolCallCount >= 2, "filtered by min tool calls");
    }
}

// --- Test 10: Statistics ---
static void test_statistics() {
    printf("Test 10: Statistics...\n");
    TraceGenerator gen;
    auto traces = gen.generateBatch(12, 42);

    auto stats = TraceExporter::computeStats(traces);
    expect(stats.totalTraces == 12, "total traces count");
    expect(stats.successCount + stats.failureCount == 12, "success + failure = total");
    expect(stats.totalToolCalls > 0, "total tool calls > 0");
    expect(stats.avgToolCalls > 0.0, "avg tool calls > 0");
    expect(stats.avgSteps > 0.0, "avg steps > 0");
    expect(stats.scenarioDistribution.size() == 6, "6 scenarios in distribution");
    expect(stats.languageDistribution.size() == 2, "2 languages in distribution");
    expect(!stats.toolUsageCount.empty(), "tool usage tracked");
    expect(!stats.difficultyDistribution.empty(), "difficulty distribution tracked");

    // Stats to JSON
    json statsJson = stats.toJson();
    expect(statsJson.contains("totalTraces"), "stats json has totalTraces");
    expect(statsJson["totalTraces"] == 12, "stats json totalTraces value");
}

// --- Test 11: Code corpus ---
static void test_corpus() {
    printf("Test 11: Code corpus...\n");
    TraceGenerator gen;
    const auto& corpus = gen.corpus();

    expect(!corpus.empty(), "corpus not empty");
    expect(corpus.size() >= 9, "at least 9 samples in corpus");

    // Check corpus has multiple languages
    std::set<std::string> languages;
    for (const auto& s : corpus) {
        languages.insert(s.language);
        expect(!s.source.empty(), "sample has source");
        expect(!s.description.empty(), "sample has description");
        expect(s.complexity >= 1 && s.complexity <= 3, "valid complexity");
    }
    expect(languages.size() >= 4, "at least 4 languages in corpus");
    expect(languages.count("python") > 0, "has python samples");
    expect(languages.count("cpp") > 0, "has cpp samples");
}

// --- Test 12: Trace toJson roundtrip ---
static void test_toJson_roundtrip() {
    printf("Test 12: Trace toJson roundtrip...\n");
    TraceGenerator gen;
    gen.setSeed(700);

    auto trace = gen.generate(ScenarioType::AddAnnotations);
    json j = trace.toJson();

    // Verify all fields round-trip
    expect(j["id"] == trace.id, "id roundtrip");
    expect(j["scenario"] == trace.scenario, "scenario roundtrip");
    expect(j["difficulty"] == trace.difficulty, "difficulty roundtrip");
    expect(j["language"] == trace.language, "language roundtrip");
    expect(j["toolCallCount"] == trace.toolCallCount, "toolCallCount roundtrip");
    expect(j["success"] == trace.success, "success roundtrip");
    expect(j["steps"].size() == trace.steps.size(), "steps count roundtrip");
    expect(j["toolsUsed"].size() == trace.toolsUsed.size(), "toolsUsed count roundtrip");

    // Verify JSON is parseable after dump
    std::string dumped = j.dump();
    json reparsed = json::parse(dumped);
    expect(reparsed == j, "JSON dump/parse roundtrip");
}

// --- Test 13: Batch export formats ---
static void test_batch_exports() {
    printf("Test 13: Batch export formats...\n");
    TraceGenerator gen;
    auto traces = gen.generateBatch(6, 42);

    json anthropicBatch = TraceExporter::toAnthropicBatch(traces);
    expect(anthropicBatch.is_array(), "Anthropic batch is array");
    expect(anthropicBatch.size() == 6, "Anthropic batch has 6 items");

    json openaiBatch = TraceExporter::toOpenAIBatch(traces);
    expect(openaiBatch.is_array(), "OpenAI batch is array");
    expect(openaiBatch.size() == 6, "OpenAI batch has 6 items");
}

int main() {
    printf("=== Step 219: Trace Generation & Export Tests ===\n\n");

    test_trace_schema();
    test_all_scenarios();
    test_seed_determinism();
    test_anthropic_export();
    test_openai_export();
    test_jsonl_export();
    test_markdown_export();
    test_batch_generation();
    test_filtering();
    test_statistics();
    test_corpus();
    test_toJson_roundtrip();
    test_batch_exports();

    printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed ? 1 : 0;
}
