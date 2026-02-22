#pragma once
// Step 755: dynamic->static risk classifier.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "DynamicStrictnessPolicy.h"

struct DynamicRiskPacket {
    std::string level = "low";
    std::vector<std::string> reasons;
};

class DynamicRiskClassifier {
public:
    static DynamicRiskPacket classify(int dynamicDispatchCount,
                                      bool hasReflectionBoundary,
                                      const DynamicStrictnessPolicy& policy) {
        DynamicRiskPacket p;
        if (dynamicDispatchCount > 0) p.reasons.push_back("dynamic_dispatch");
        if (hasReflectionBoundary) p.reasons.push_back("reflection_boundary");
        if (policy.allowImplicitAny) p.reasons.push_back("implicit_any_allowed");

        if (dynamicDispatchCount > 0 && hasReflectionBoundary) p.level = "high";
        else if (dynamicDispatchCount > 0 || hasReflectionBoundary) p.level = "medium";
        else p.level = "low";
        return p;
    }

    static nlohmann::json toJson(const DynamicRiskPacket& p) {
        return {{"level", p.level}, {"reasons", p.reasons}};
    }
};
