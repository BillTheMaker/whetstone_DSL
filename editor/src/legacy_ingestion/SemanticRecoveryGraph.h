#pragma once
// Step 809: Legacy semantic recovery graph engine.

#include <string>
#include <vector>
#include <unordered_map>

#include <nlohmann/json.hpp>

struct RecoveryNode {
    std::string id;
    std::string intent;
    double confidence = 0.0;
    std::vector<std::string> neighbors;
};

class SemanticRecoveryGraph {
public:
    static std::vector<RecoveryNode> build(const std::string& source) {
        std::vector<RecoveryNode> nodes;
        nodes.push_back({"root", source.empty() ? "unknown" : "core", source.empty() ? 0.3 : 0.8, {"api"}});
        nodes.push_back({"api", "api_inference", 0.6, {"build"}});
        if (source.find("TODO") != std::string::npos) {
            nodes.push_back({"todo", "assumed", 0.4, {}});
        }
        return nodes;
    }

    static nlohmann::json toJson(const RecoveryNode& n) {
        return {{"id", n.id}, {"intent", n.intent}, {"confidence", n.confidence}, {"neighbors", n.neighbors}};
    }
};
