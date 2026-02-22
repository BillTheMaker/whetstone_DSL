#pragma once
// Step 784: Supervision tree preservation packet model.

#include <nlohmann/json.hpp>

#include "LogicActorPacketTypes.h"

struct SupervisionTreePreservationPacket {
    bool supervisionDetected = false;
    bool canPreserveAutomatically = false;
    int confidence = 0;
};

class SupervisionTreePacketModel {
public:
    static SupervisionTreePreservationPacket build(const LogicActorLoweringPacket& p, const std::string& targetLanguage) {
        SupervisionTreePreservationPacket out;
        out.supervisionDetected = p.supervision;
        out.canPreserveAutomatically = p.supervision && (targetLanguage == "erlang" || targetLanguage == "elixir");
        out.confidence = out.canPreserveAutomatically ? 90 : (p.supervision ? 40 : 100);
        return out;
    }

    static nlohmann::json toJson(const SupervisionTreePreservationPacket& p) {
        return {{"supervision_detected", p.supervisionDetected}, {"can_preserve_automatically", p.canPreserveAutomatically}, {"confidence", p.confidence}};
    }
};
