// Step 234: Session pipeline tests.
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

static json sampleSession(const std::string& pathValue, const std::string& method) {
    return {
        {"sessionId", "s1"},
        {"metadata", {{"language", "python"}}},
        {"events", json::array({{
            {"timestamp", "2026-02-10T10:00:00.000"},
            {"type", "file_opened"},
            {"payload", {{"path", pathValue}}}
        }})},
        {"calls", json::array({{
            {"timestamp", "2026-02-10T10:00:01.000"},
            {"method", method},
            {"request", {{"params", json::object()}}},
            {"response", {{"result", json::object()}}}
        }})}
    };
}

int main() {
    int passed = 0;
    int failed = 0;

    try {
        SessionAnonymizer anonymizer;
        anonymizer.setLevel(SessionAnonymizer::Level::Medium);

        json session = sampleSession("C:/Users/Bob/project/main.py", "getAST");
        json anon = anonymizer.anonymize(session);

        expect(session["calls"].size() == anon["calls"].size(), "structure preserved", passed, failed);
        expect(anon.contains("events") && anon.contains("calls"), "events and calls preserved", passed, failed);
        std::string anonPath = anon["events"][0]["payload"].value("path", "");
        expect(anonPath.find("/project/src/") == 0, "paths anonymized", passed, failed);
        expect(anon.dump().find("Bob") == std::string::npos, "identifiers removed", passed, failed);

        SessionTraceConverter converter;
        auto traces = converter.convert(anon);
        expect(!traces.empty() && validateTrace(traces[0]), "trace conversion valid", passed, failed);

        PipelineStats stats;
        auto filtered = filterTraces(traces, stats);
        std::string jsonl = exportTraces(filtered, "jsonl");
        int lineCount = 0;
        for (char c : jsonl) if (c == '\n') lineCount++;
        expect(lineCount == (int)filtered.size(), "pipeline output count", passed, failed);

        Trace degenerate;
        degenerate.id = "bad";
        degenerate.scenario = "session_replay";
        degenerate.language = "python";
        std::vector<Trace> withDegenerate = {traces[0], degenerate};
        PipelineStats stats2;
        auto filtered2 = filterTraces(withDegenerate, stats2);
        expect(filtered2.size() == 1, "quality filter removes degenerate", passed, failed);

        PipelineStats stats3;
        auto deduped = dedupeTraces({traces[0], traces[0]}, stats3);
        expect(deduped.size() == 1, "deduplication works", passed, failed);
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        failed++;
    }

    std::cout << "\n=== Step 234 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
