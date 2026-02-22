#pragma once
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "legacy_ingestion/MigrationReadiness.h"

struct DossierEntry {
    std::string area;
    std::string status;
    double confidence;
};

class ModernizationDossier {
public:
    static std::vector<DossierEntry> produce(const MigrationScore& score) {
        std::vector<DossierEntry> entries;
        entries.push_back(DossierEntry{"core", score.ready ? "ready" : "pending", score.confidence});
        entries.push_back(DossierEntry{"api", score.completeness > 0.5 ? "ready" : "review", score.completeness});
        return entries;
    }

    static nlohmann::json toJson(const DossierEntry& entry) {
        return {{"area", entry.area}, {"status", entry.status}, {"confidence", entry.confidence}};
    }
};
