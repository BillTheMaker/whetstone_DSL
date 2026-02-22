#pragma once
// Step 1450: root-cause clustering model.

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "FailurePacket.h"

struct FailureCluster {
    std::string clusterId;
    std::string rootCauseCandidate;
    std::string failureClass;
    std::vector<std::string> memberPacketIds;
    int blastRadius = 0;
    bool fixFirst = false;
};

class FailureClusterer {
public:
    static std::vector<FailureCluster> cluster(const std::vector<FailurePacket>& packets) {
        std::map<std::string, FailureCluster> byKey;
        for (const auto& p : packets) {
            std::string key = p.failureClass + "|" + p.primaryFile + "|" + p.primarySymbol;
            auto& c = byKey[key];
            if (c.clusterId.empty()) {
                c.failureClass = p.failureClass;
                c.rootCauseCandidate = p.primarySymbol.empty() ? p.primaryFile : p.primarySymbol;
                c.clusterId = stableId(key);
            }
            c.memberPacketIds.push_back(p.packetId);
        }

        std::vector<FailureCluster> out;
        for (auto& kv : byKey) {
            kv.second.blastRadius = static_cast<int>(kv.second.memberPacketIds.size());
            std::sort(kv.second.memberPacketIds.begin(), kv.second.memberPacketIds.end());
            out.push_back(kv.second);
        }

        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
            if (a.blastRadius != b.blastRadius) return a.blastRadius > b.blastRadius;
            return a.clusterId < b.clusterId;
        });
        if (!out.empty()) out[0].fixFirst = true;
        return out;
    }

    static nlohmann::json toJson(const std::vector<FailureCluster>& c) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& x : c) {
            j.push_back({
                {"cluster_id", x.clusterId},
                {"root_cause_candidate", x.rootCauseCandidate},
                {"failure_class", x.failureClass},
                {"member_packet_ids", x.memberPacketIds},
                {"blast_radius", x.blastRadius},
                {"fix_first", x.fixFirst}
            });
        }
        return j;
    }

private:
    static std::string stableId(const std::string& key) {
        uint64_t h = 2166136261u;
        for (unsigned char c : key) {
            h ^= c;
            h *= 16777619u;
        }
        return "fc_" + std::to_string(h);
    }
};
