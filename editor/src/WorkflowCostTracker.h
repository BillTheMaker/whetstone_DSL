#pragma once

#include "ModelProfileRegistry.h"

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct CostRecord {
    std::string itemId;
    std::string workerType;   // slm|llm|deterministic|template|human
    std::string profileName;  // resolved model profile name
    int estimatedInputTokens = 0;
    int estimatedOutputTokens = 0;
    int actualInputTokens = 0;
    int actualOutputTokens = 0;
};

struct CostSummary {
    int estimatedInputTokens = 0;
    int estimatedOutputTokens = 0;
    int actualInputTokens = 0;
    int actualOutputTokens = 0;
    double estimatedCost = 0.0;
    double actualCost = 0.0;
    std::map<std::string, double> costByWorker;
    std::map<std::string, double> costByProfile;

    double varianceCost() const { return actualCost - estimatedCost; }
};

class WorkflowCostTracker {
public:
    explicit WorkflowCostTracker(const ModelProfileRegistry* registry = nullptr)
        : registry_(registry) {}

    void setRegistry(const ModelProfileRegistry* registry) {
        registry_ = registry;
    }

    void recordEstimate(const std::string& itemId,
                        const std::string& workerType,
                        int inputTokens,
                        int outputTokens) {
        CostRecord& rec = records_[itemId];
        rec.itemId = itemId;
        rec.workerType = workerType;
        rec.estimatedInputTokens = inputTokens;
        rec.estimatedOutputTokens = outputTokens;
        if (rec.profileName.empty()) rec.profileName = resolveProfile(workerType);
    }

    void recordActual(const std::string& itemId,
                      const std::string& workerType,
                      int inputTokens,
                      int outputTokens,
                      const std::string& explicitProfile = "") {
        CostRecord& rec = records_[itemId];
        rec.itemId = itemId;
        rec.workerType = workerType;
        rec.actualInputTokens = inputTokens;
        rec.actualOutputTokens = outputTokens;
        rec.profileName = !explicitProfile.empty() ? explicitProfile : resolveProfile(workerType);
    }

    bool hasRecord(const std::string& itemId) const {
        return records_.find(itemId) != records_.end();
    }

    CostRecord getRecord(const std::string& itemId) const {
        auto it = records_.find(itemId);
        if (it == records_.end()) return {};
        return it->second;
    }

    size_t count() const { return records_.size(); }

    CostSummary summarize() const {
        CostSummary out;
        for (const auto& [_, rec] : records_) {
            out.estimatedInputTokens += rec.estimatedInputTokens;
            out.estimatedOutputTokens += rec.estimatedOutputTokens;
            out.actualInputTokens += rec.actualInputTokens;
            out.actualOutputTokens += rec.actualOutputTokens;

            double est = estimateCost(rec.profileName, rec.estimatedInputTokens,
                                      rec.estimatedOutputTokens);
            double act = estimateCost(rec.profileName, rec.actualInputTokens,
                                      rec.actualOutputTokens);
            out.estimatedCost += est;
            out.actualCost += act;
            out.costByWorker[rec.workerType] += act;
            out.costByProfile[rec.profileName] += act;
        }
        return out;
    }

    json reportJson() const {
        auto s = summarize();
        json recs = json::array();
        for (const auto& [_, rec] : records_) {
            recs.push_back({
                {"itemId", rec.itemId},
                {"workerType", rec.workerType},
                {"profileName", rec.profileName},
                {"estimatedInputTokens", rec.estimatedInputTokens},
                {"estimatedOutputTokens", rec.estimatedOutputTokens},
                {"actualInputTokens", rec.actualInputTokens},
                {"actualOutputTokens", rec.actualOutputTokens}
            });
        }
        return {
            {"recordCount", (int)records_.size()},
            {"estimatedInputTokens", s.estimatedInputTokens},
            {"estimatedOutputTokens", s.estimatedOutputTokens},
            {"actualInputTokens", s.actualInputTokens},
            {"actualOutputTokens", s.actualOutputTokens},
            {"estimatedCost", s.estimatedCost},
            {"actualCost", s.actualCost},
            {"costVariance", s.varianceCost()},
            {"costByWorker", s.costByWorker},
            {"costByProfile", s.costByProfile},
            {"records", recs}
        };
    }

private:
    const ModelProfileRegistry* registry_ = nullptr;
    std::map<std::string, CostRecord> records_;

    std::string resolveProfile(const std::string& workerType) const {
        if (!registry_) return "unknown";
        std::string name = registry_->resolveProfileForWorker(workerType);
        return name.empty() ? "unknown" : name;
    }

    double estimateCost(const std::string& profileName,
                        int inputTokens,
                        int outputTokens) const {
        if (!registry_ || profileName.empty() || profileName == "unknown") return 0.0;
        auto p = registry_->getProfile(profileName);
        if (p.name.empty()) return 0.0;
        double inCost = (double)inputTokens / 1000.0 * p.costPer1kInput;
        double outCost = (double)outputTokens / 1000.0 * p.costPer1kOutput;
        return inCost + outCost;
    }
};
