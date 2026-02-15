#pragma once
#include "TrainingDataGenerator.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <map>

using json = nlohmann::json;

struct ExportStats {
    int totalPairs = 0;
    std::map<std::string, int> annotationCounts;  // type -> count
    std::map<std::string, int> languageCounts;     // language -> count
};

class TrainingDataExporter {
public:
    // HuggingFace instruction/input/output format (JSONL)
    static std::string exportHuggingFace(const std::vector<TrainingPair>& pairs) {
        std::ostringstream oss;
        for (const auto& p : pairs) {
            json line;
            line["instruction"] = "Add semantic annotations (Semanno format) to the following " +
                                  p.language + " code.";
            line["input"] = p.rawCode;
            line["output"] = p.annotatedCode;
            oss << line.dump() << "\n";
        }
        return oss.str();
    }

    // Raw/annotated pairs (JSONL)
    static std::string exportPairsJSONL(const std::vector<TrainingPair>& pairs) {
        std::ostringstream oss;
        for (const auto& p : pairs) {
            json line;
            line["raw_code"] = p.rawCode;
            line["annotated_code"] = p.annotatedCode;
            line["language"] = p.language;
            line["annotations"] = p.annotations;
            oss << line.dump() << "\n";
        }
        return oss.str();
    }

    // Compute statistics
    static ExportStats computeStats(const std::vector<TrainingPair>& pairs) {
        ExportStats stats;
        stats.totalPairs = (int)pairs.size();
        for (const auto& p : pairs) {
            stats.languageCounts[p.language]++;
            for (const auto& a : p.annotations) {
                stats.annotationCounts[a]++;
            }
        }
        return stats;
    }

    static json statsToJson(const ExportStats& stats) {
        json j;
        j["totalPairs"] = stats.totalPairs;
        j["annotationCounts"] = stats.annotationCounts;
        j["languageCounts"] = stats.languageCounts;
        return j;
    }
};
