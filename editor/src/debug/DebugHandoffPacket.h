#pragma once
// Step 1547: debug handoff packet model.

#include <nlohmann/json.hpp>

#include "DebugChecklistTemplate.h"
#include "DebugSessionHandoff.h"
#include "DebugEvidenceIndex.h"

struct DebugHandoffPacket {
    DebugChecklistTemplate checklist;
    DebugSessionHandoff handoff;
    std::vector<DebugEvidenceEntry> evidence;
};

class DebugHandoffPacketModel {
public:
    static DebugHandoffPacket build(const DebugChecklistTemplate& checklist,
                                    const DebugSessionHandoff& handoff,
                                    const std::vector<DebugEvidenceEntry>& evidence) {
        return {checklist, handoff, evidence};
    }

    static nlohmann::json toJson(const DebugHandoffPacket& p) {
        return {
            {"checklist", DebugChecklistTemplateModel::toJson(p.checklist)},
            {"handoff", DebugSessionHandoffModel::toJson(p.handoff)},
            {"evidence", DebugEvidenceIndexModel::toJson(DebugEvidenceIndexModel::sort(p.evidence))}
        };
    }
};
