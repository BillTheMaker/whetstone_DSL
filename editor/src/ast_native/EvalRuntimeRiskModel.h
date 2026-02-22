#pragma once
// Step 775: Eval/runtime boundary risk model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct EvalRiskPacket {
    std::string level;
    std::vector<std::string> reasons;
};

class EvalRuntimeRiskModel {
public:
    static EvalRiskPacket classify(const std::string& source) {
        bool hasEval = source.find("eval") != std::string::npos;
        bool hasLoad = source.find("load") != std::string::npos || source.find("read-from-string") != std::string::npos;

        EvalRiskPacket p;
        if (hasEval && hasLoad) p.level = "high";
        else if (hasEval || hasLoad) p.level = "medium";
        else p.level = "low";

        if (hasEval) p.reasons.push_back("eval_present");
        if (hasLoad) p.reasons.push_back("runtime_load_present");
        if (p.reasons.empty()) p.reasons.push_back("no_runtime_eval_boundary");
        return p;
    }

    static nlohmann::json toJson(const EvalRiskPacket& p) {
        return {
            {"level", p.level},
            {"reasons", p.reasons}
        };
    }
};
