#pragma once
// Sprint 27 refactor: shared schema-failure packet helper

#include <string>
#include <vector>

#include "ConstraintViolationDiagnostics.h"

inline ConstraintDiagnosticPacket makeUnderConstrainedContractPacket(
    const std::vector<std::string>& errors) {
    ConstraintDiagnosticPacket packet;
    packet.ok = false;
    packet.recommendedAction = "escalate";
    for (const auto& error : errors) {
        packet.violations.push_back({
            "under_constrained_contract",
            "Taskitem contract failed schema validation",
            error,
            false
        });
    }
    return packet;
}
