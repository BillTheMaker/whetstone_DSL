#pragma once
// Step 1172: Integrity attestation packet schema.
#include <string>
#include <nlohmann/json.hpp>

struct IntegrityAttestationPacketSchema {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class IntegrityAttestationPacketSchemaFactory {
public:
    static IntegrityAttestationPacketSchema make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const IntegrityAttestationPacketSchema& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
