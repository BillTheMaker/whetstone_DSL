#pragma once
// Step 714: async mapping strategy.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppAsyncDecision {
    std::string nodeId;
    std::string runtimeProfile; // coroutine_std, task_runtime
};

class CppAsyncMappingStrategy {
public:
    static std::vector<CppAsyncDecision> map(const SemanticCoreIR& ir,
                                             const std::string& profile = "safe-first") {
        std::vector<CppAsyncDecision> out;
        for (const auto& n : ir.nodes) {
            if (n.kind != IRNodeKind::ConcurrencyRegion && n.kind != IRNodeKind::Function) continue;
            bool hasAsync = false;
            for (const auto& t : n.intentTags) if (t == "async") hasAsync = true;
            if (!hasAsync) continue;
            out.push_back({n.id, profile == "interop-first" ? "task_runtime" : "coroutine_std"});
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.nodeId < b.nodeId; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<CppAsyncDecision>& d) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& x : d) j.push_back({{"nodeId", x.nodeId}, {"runtimeProfile", x.runtimeProfile}});
        return j;
    }
};
