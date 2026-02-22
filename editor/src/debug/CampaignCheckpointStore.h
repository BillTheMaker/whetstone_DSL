#pragma once
// Step 1500: campaign checkpoint store.

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "CampaignCheckpoint.h"

class CampaignCheckpointStore {
public:
    static bool save(const std::string& path, const CampaignCheckpoint& c) {
        auto all = list(path);
        all.push_back(c);
        sortStable(all);

        nlohmann::json arr = nlohmann::json::array();
        for (const auto& x : all) arr.push_back(CampaignCheckpointModel::toJson(x));

        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out << arr.dump(2);
        return out.good();
    }

    static std::vector<CampaignCheckpoint> list(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) return {};
        nlohmann::json j;
        try {
            in >> j;
        } catch (...) {
            return {};
        }
        std::vector<CampaignCheckpoint> out;
        if (!j.is_array()) return out;
        for (const auto& it : j) out.push_back(CampaignCheckpointModel::fromJson(it));
        sortStable(out);
        return out;
    }

private:
    static void sortStable(std::vector<CampaignCheckpoint>& items) {
        std::sort(items.begin(), items.end(), [](const CampaignCheckpoint& a, const CampaignCheckpoint& b) {
            if (a.campaignId != b.campaignId) return a.campaignId < b.campaignId;
            if (a.sequence != b.sequence) return a.sequence < b.sequence;
            return a.checkpointId < b.checkpointId;
        });
    }
};
