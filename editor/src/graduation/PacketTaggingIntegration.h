#pragma once
// Step 850: Packet tagging integration (failure codes -> packets).
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TaggingResult {
    std::string packetId;
    std::vector<std::string> tags;
    bool tagged = false;
};

class PacketTaggingIntegration {
public:
    static TaggingResult tag(const std::string& packetId, const std::vector<std::string>& codes) {
        TaggingResult r;
        r.packetId = packetId;
        r.tags = codes;
        r.tagged = !codes.empty();
        return r;
    }

    static nlohmann::json toJson(const TaggingResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& t : r.tags) arr.push_back(t);
        return {{"packet_id", r.packetId}, {"tags", arr}, {"tagged", r.tagged}};
    }
};
