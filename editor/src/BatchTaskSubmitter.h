#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct AgentTaskRequest {
    std::string itemId;
    std::string workerType;   // slm|llm
    std::string language;
    std::string bufferId;
    std::string context;
    std::string prompt;
};

struct AgentTaskBatch {
    std::string workerType;
    std::string language;
    std::string bufferId;
    std::string sharedContext;
    std::vector<AgentTaskRequest> tasks;

    json toJson() const {
        json taskArr = json::array();
        for (const auto& t : tasks) {
            taskArr.push_back({
                {"itemId", t.itemId},
                {"prompt", t.prompt}
            });
        }
        return {
            {"workerType", workerType},
            {"language", language},
            {"bufferId", bufferId},
            {"sharedContext", sharedContext},
            {"tasks", taskArr}
        };
    }
};

struct BatchPlan {
    std::vector<AgentTaskBatch> batches;
    int estimatedTokensIndividual = 0;
    int estimatedTokensBatched = 0;

    double estimatedSavingsPercent() const {
        if (estimatedTokensIndividual <= 0) return 0.0;
        int saved = estimatedTokensIndividual - estimatedTokensBatched;
        if (saved <= 0) return 0.0;
        return (100.0 * (double)saved) / (double)estimatedTokensIndividual;
    }
};

class BatchTaskSubmitter {
public:
    static BatchPlan planBatches(const std::vector<AgentTaskRequest>& tasks,
                                 int maxBatchSize = 4) {
        if (maxBatchSize < 1) maxBatchSize = 1;
        BatchPlan plan;

        std::map<std::string, std::vector<AgentTaskRequest>> groups;
        for (const auto& t : tasks) {
            std::string key = makeGroupKey(t);
            groups[key].push_back(t);
            plan.estimatedTokensIndividual += estimateTokens(t.context + "\n" + t.prompt);
        }

        for (auto& [_, group] : groups) {
            // Stable order for deterministic payloads.
            std::sort(group.begin(), group.end(),
                      [](const AgentTaskRequest& a, const AgentTaskRequest& b) {
                          return a.itemId < b.itemId;
                      });

            for (size_t i = 0; i < group.size(); i += (size_t)maxBatchSize) {
                AgentTaskBatch batch;
                batch.workerType = group[i].workerType;
                batch.language = group[i].language;
                batch.bufferId = group[i].bufferId;
                batch.sharedContext = group[i].context;

                size_t end = std::min(group.size(), i + (size_t)maxBatchSize);
                for (size_t j = i; j < end; ++j) {
                    batch.tasks.push_back(group[j]);
                }
                plan.estimatedTokensBatched += estimateBatchTokens(batch);
                plan.batches.push_back(batch);
            }
        }

        std::sort(plan.batches.begin(), plan.batches.end(),
                  [](const AgentTaskBatch& a, const AgentTaskBatch& b) {
                      if (a.workerType != b.workerType) return a.workerType < b.workerType;
                      if (a.language != b.language) return a.language < b.language;
                      return a.bufferId < b.bufferId;
                  });
        return plan;
    }

private:
    static std::string makeGroupKey(const AgentTaskRequest& t) {
        return t.workerType + "|" + t.language + "|" + t.bufferId;
    }

    static int estimateTokens(const std::string& text) {
        if (text.empty()) return 0;
        int chars = (int)text.size();
        int est = chars / 4;
        return est > 0 ? est : 1;
    }

    static int estimateBatchTokens(const AgentTaskBatch& batch) {
        int total = estimateTokens(batch.sharedContext);
        for (const auto& t : batch.tasks) {
            total += estimateTokens(t.prompt);
        }
        return total;
    }
};
