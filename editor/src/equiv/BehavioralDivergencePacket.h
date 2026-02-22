#pragma once
// Step 725: behavioral divergence packet + minimization.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct BehavioralDivergencePacket {
    std::string caseId;
    std::string sourceOutput;
    std::string targetOutput;
    std::string classification = "equivalent";
    std::string minimizedInput;
};

class BehavioralDivergencePacketModel {
public:
    static BehavioralDivergencePacket classify(const std::string& caseId,
                                               const std::string& sourceOutput,
                                               const std::string& targetOutput,
                                               const std::string& input) {
        BehavioralDivergencePacket p;
        p.caseId = caseId;
        p.sourceOutput = sourceOutput;
        p.targetOutput = targetOutput;
        p.classification = sourceOutput == targetOutput ? "equivalent" : "unacceptable_divergence";
        p.minimizedInput = input.size() <= 16 ? input : input.substr(0, 16);
        return p;
    }

    static nlohmann::json toJson(const BehavioralDivergencePacket& p) {
        return {
            {"case_id", p.caseId},
            {"source_output", p.sourceOutput},
            {"target_output", p.targetOutput},
            {"classification", p.classification},
            {"minimized_input", p.minimizedInput}
        };
    }
};
