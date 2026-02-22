#pragma once
// Step 836: Documentation pack for language support + migration playbooks.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct PlaybookEntry {
    std::string pairId;
    std::string paradigm;
    std::string steps;
    std::string caveats;
};

struct DocumentationPack {
    std::string version;
    std::vector<PlaybookEntry> playbooks;
    std::vector<std::string> supportedPairs;
    bool published = false;
};

class DocumentationPackGenerator {
public:
    static DocumentationPack build(const std::string& version,
                                   const std::vector<PlaybookEntry>& playbooks,
                                   const std::vector<std::string>& pairs) {
        DocumentationPack p;
        p.version = version;
        p.playbooks = playbooks;
        p.supportedPairs = pairs;
        p.published = !version.empty() && !pairs.empty();
        return p;
    }

    static nlohmann::json toJson(const DocumentationPack& p) {
        nlohmann::json pb = nlohmann::json::array();
        for (const auto& e : p.playbooks)
            pb.push_back({{"pair_id", e.pairId}, {"paradigm", e.paradigm}, {"steps", e.steps}});
        nlohmann::json pairs = nlohmann::json::array();
        for (const auto& s : p.supportedPairs) pairs.push_back(s);
        return {{"version", p.version}, {"playbooks", pb},
                {"supported_pairs", pairs}, {"published", p.published}};
    }
};
