// Step 233: Training data pipeline CLI helpers.
#include "SessionPipeline.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    json session = {
        {"sessionId", "s1"},
        {"metadata", {{"language", "python"}}},
        {"events", json::array({{
            {"timestamp", "2026-02-10T10:00:00.000"},
            {"type", "file_opened"},
            {"payload", {{"path", "main.py"}}}
        }})},
        {"calls", json::array({{
            {"timestamp", "2026-02-10T10:00:01.000"},
            {"method", "getAST"},
            {"request", {{"params", json::object()}}},
            {"response", {{"result", json::object()}}}
        }})}
    };

    PipelineOptions opts;
    opts.anonymizeEnabled = false;
    PipelineStats stats;
    SessionAnonymizer anonymizer;
    auto anon = anonymizeSessions({session}, opts, anonymizer);
    auto traces = sessionsToTraces(anon);
    stats.tracesGenerated = (int)traces.size();

    auto filtered = filterTraces(traces, stats);
    auto output = exportTraces(filtered, "jsonl");

    expect(stats.tracesGenerated == 1, "generated one trace", passed, failed);
    expect(!output.empty(), "export produced output", passed, failed);

    std::cout << "\n=== Step 233 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
