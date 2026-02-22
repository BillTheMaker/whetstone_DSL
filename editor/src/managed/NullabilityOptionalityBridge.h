#pragma once
// Step 763: Nullability + optionality canonical model bridge.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

struct NullabilityBridgePacket {
    std::string canonicalModel;
    bool nullableDetected = false;
    bool optionalDetected = false;
    std::string bridgeNote;
};

class NullabilityOptionalityBridge {
public:
    static NullabilityBridgePacket fromLowering(const ManagedLoweringPacket& p) {
        NullabilityBridgePacket out;
        out.nullableDetected = p.hasNullableSyntax;
        out.optionalDetected = p.hasOptionalType;
        if (p.hasOptionalType) {
            out.canonicalModel = "option";
            out.bridgeNote = "map option/optional to canonical Option<T>";
        } else if (p.hasNullableSyntax) {
            out.canonicalModel = "nullable";
            out.bridgeNote = "preserve nullable reference/value intent";
        } else {
            out.canonicalModel = "nonnullable";
            out.bridgeNote = "default nonnullable path";
        }
        return out;
    }

    static nlohmann::json toJson(const NullabilityBridgePacket& p) {
        return {
            {"canonical_model", p.canonicalModel},
            {"nullable_detected", p.nullableDetected},
            {"optional_detected", p.optionalDetected},
            {"bridge_note", p.bridgeNote}
        };
    }
};
