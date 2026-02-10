// Step 232: Session-to-trace converter.
#include "SessionToTrace.h"
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

    try {
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

        SessionTraceConverter converter;
        auto traces = converter.convert(session);
        expect(traces.size() == 1, "one trace generated", passed, failed);
        if (!traces.empty()) {
            expect(traces[0].toolCallCount == 1, "tool call count", passed, failed);
            bool hasUser = false;
            bool hasToolCall = false;
            bool hasToolResult = false;
            for (const auto& step : traces[0].steps) {
                if (step.role == "user") hasUser = true;
                if (step.role == "tool_call") hasToolCall = true;
                if (step.role == "tool_result") hasToolResult = true;
            }
            expect(hasUser, "user step generated", passed, failed);
            expect(hasToolCall, "tool_call step generated", passed, failed);
            expect(hasToolResult, "tool_result step generated", passed, failed);
        }
    } catch (const std::exception& ex) {
        std::cout << "Exception: " << ex.what() << "\n";
        failed++;
    }

    std::cout << "\n=== Step 232 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
