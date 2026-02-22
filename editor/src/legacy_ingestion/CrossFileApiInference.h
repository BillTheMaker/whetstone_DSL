#pragma once
// Step 810: Cross-file API intent inference.

#include <string>
#include <vector>
#include <unordered_map>

#include <nlohmann/json.hpp>

struct ApiIntentPacket {
    std::string api;
    std::string inferredIntent;
    double confidence;
};

class CrossFileApiInference {
public:
    static ApiIntentPacket infer(const std::string& api, const std::vector<std::string>& files) {
        ApiIntentPacket pkt{api, "read", 0.5};
        if (files.size() > 1 || api.find("write") != std::string::npos) {
            pkt.inferredIntent = "write";
            pkt.confidence = 0.7;
        }
        if (api.find("stream") != std::string::npos) {
            pkt.confidence = 0.85;
        }
        return pkt;
    }

    static nlohmann::json toJson(const ApiIntentPacket& p) {
        return {{"api", p.api}, {"intent", p.inferredIntent}, {"confidence", p.confidence}};
    }
};
