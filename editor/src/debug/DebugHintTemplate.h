#pragma once
// Step 1509: debug hint template model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugHintPacket {
    std::string failureClass;
    std::vector<std::string> hints;
    std::string confidence = "medium";
};

class DebugHintTemplate {
public:
    static DebugHintPacket build(const std::string& failureClass,
                                 const std::vector<std::string>& context) {
        DebugHintPacket p;
        p.failureClass = failureClass;
        p.hints = baseHints(failureClass);
        if (!context.empty()) p.hints.push_back("Focus file: " + context.front());
        std::sort(p.hints.begin(), p.hints.end());
        p.hints.erase(std::unique(p.hints.begin(), p.hints.end()), p.hints.end());
        p.confidence = p.hints.size() > 2 ? "high" : "medium";
        return p;
    }

    static nlohmann::json toJson(const DebugHintPacket& p) {
        return {{"failure_class", p.failureClass}, {"hints", p.hints}, {"confidence", p.confidence}};
    }

private:
    static std::vector<std::string> baseHints(const std::string& failureClass) {
        if (failureClass == "compile") {
            return {"Check recent syntax edits", "Re-run single target first", "Review include/import order"};
        }
        if (failureClass == "test") {
            return {"Minimize fixture setup", "Verify expectation order", "Run failing test in isolation"};
        }
        return {"Capture failure packet", "Cluster by root symptom", "Guard against regressions"};
    }
};
