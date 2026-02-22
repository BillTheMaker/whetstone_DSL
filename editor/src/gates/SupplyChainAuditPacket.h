#pragma once
// Step 731: dependency and supply-chain audit packet.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DependencyAudit {
    std::string name;
    int highVulns = 0;
};

struct SupplyChainAuditPacket {
    std::vector<DependencyAudit> dependencies;
    int totalHigh = 0;
};

class SupplyChainAuditPacketModel {
public:
    static SupplyChainAuditPacket build(std::vector<DependencyAudit> deps) {
        std::sort(deps.begin(), deps.end(), [](const DependencyAudit& a, const DependencyAudit& b) {
            return a.name < b.name;
        });
        SupplyChainAuditPacket p;
        p.dependencies = std::move(deps);
        for (const auto& d : p.dependencies) p.totalHigh += d.highVulns;
        return p;
    }

    static nlohmann::json toJson(const SupplyChainAuditPacket& p) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& d : p.dependencies) arr.push_back({{"name", d.name}, {"high_vulns", d.highVulns}});
        return {{"dependencies", arr}, {"total_high", p.totalHigh}};
    }
};
