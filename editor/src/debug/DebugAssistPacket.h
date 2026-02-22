#pragma once
// Step 1517: debug assist packet bundle model.

#include <nlohmann/json.hpp>

#include "DebugHintTemplate.h"
#include "FailureTriageScore.h"
#include "MinimalReproReducer.h"
#include "PatchRiskLabeler.h"

struct DebugAssistPacket {
    DebugHintPacket hints;
    FailureTriageScore triage;
    MinimalReproResult repro;
    PatchRiskLabel risk;
};

class DebugAssistPacketModel {
public:
    static DebugAssistPacket build(const DebugHintPacket& hints,
                                   const FailureTriageScore& triage,
                                   const MinimalReproResult& repro,
                                   const PatchRiskLabel& risk) {
        return {hints, triage, repro, risk};
    }

    static nlohmann::json toJson(const DebugAssistPacket& p) {
        return {
            {"hints", DebugHintTemplate::toJson(p.hints)},
            {"triage", FailureTriageScorer::toJson(p.triage)},
            {"repro", MinimalReproReducer::toJson(p.repro)},
            {"risk", PatchRiskLabeler::toJson(p.risk)}
        };
    }
};
