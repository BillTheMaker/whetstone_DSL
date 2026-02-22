#pragma once
// Step 812: Ambiguity packet model.

#include <string>
#include <nlohmann/json.hpp>

enum class AmbiguityKind { assumed, unknown, conflict };

struct AmbiguityPacket {
    std::string id;
    AmbiguityKind kind;
    std::string note;
};

class AmbiguityPacketModel {
public:
    static AmbiguityPacket build(const std::string& source) {
        if (source.find("TODO") != std::string::npos) {
            return {"amb1", AmbiguityKind::conflict, "TODO marker"};
        }
        if (source.find("guess") != std::string::npos) {
            return {"amb2", AmbiguityKind::assumed, "heuristic guess"};
        }
        return {"amb3", AmbiguityKind::unknown, "lack of data"};
    }

    static nlohmann::json toJson(const AmbiguityPacket& p) {
        return {{"id", p.id}, {"kind", static_cast<int>(p.kind)}, {"note", p.note}};
    }
};
