#pragma once
// Step 894: Runtime-specific verification plugin interface.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct VerificationFinding {
    std::string findingId;
    std::string severity;   // "error","warning","info"
    std::string message;
};

struct RuntimeVerificationResult {
    std::string pluginId;
    std::string runtimeId;
    bool passed = false;
    std::vector<VerificationFinding> findings;
};

class RuntimeVerificationPlugin {
public:
    static RuntimeVerificationResult verify(const std::string& pluginId,
                                              const std::string& runtimeId,
                                              const std::vector<std::string>& checks) {
        RuntimeVerificationResult r;
        r.pluginId = pluginId;
        r.runtimeId = runtimeId;
        r.passed = !checks.empty();
        for (size_t i = 0; i < checks.size(); ++i) {
            if (checks[i] == "fail") {
                r.findings.push_back({"F-" + std::to_string(i+1), "error", "check_failed: " + checks[i]});
                r.passed = false;
            }
        }
        return r;
    }

    static nlohmann::json toJson(const RuntimeVerificationResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& f : r.findings)
            arr.push_back({{"finding_id", f.findingId}, {"severity", f.severity},
                           {"message", f.message}});
        return {{"plugin_id", r.pluginId}, {"runtime_id", r.runtimeId},
                {"passed", r.passed}, {"findings", arr}};
    }
};
