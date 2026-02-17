#pragma once
// Step 580: Taskitem Confidence + Ambiguity Annotation

#include "RequirementNormalizationConflictDetector.h"
#include "TaskitemGeneratorV2.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct AnnotatedTaskitem {
    GeneratedTaskitem base;
    int confidence = 0;  // 0..100
    int ambiguityCount = 0;
    bool escalate = false;
    std::vector<std::string> reasons;
};

class TaskitemConfidenceAmbiguity {
public:
    static bool annotate(const std::vector<GeneratedTaskitem>& tasks,
                         const RequirementNormalizationResult& normalized,
                         std::vector<AnnotatedTaskitem>* out,
                         std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->clear();
        if (tasks.empty()) {
            *error = "tasks_empty";
            return false;
        }

        const int ambiguousRequirements = countAmbiguous(normalized.requirements);
        const int conflictCount = static_cast<int>(normalized.conflicts.size());

        for (const auto& task : tasks) {
            AnnotatedTaskitem item;
            item.base = task;
            item.ambiguityCount = ambiguousRequirements;
            item.confidence = computeConfidence(task, ambiguousRequirements, conflictCount);
            item.reasons = deriveReasons(task, ambiguousRequirements, conflictCount, item.confidence);
            item.escalate = shouldEscalate(item);
            out->push_back(item);
        }
        return true;
    }

private:
    static int countAmbiguous(const std::vector<NormalizedRequirement>& requirements) {
        int count = 0;
        for (const auto& req : requirements) if (req.ambiguous) ++count;
        return count;
    }

    static int computeConfidence(const GeneratedTaskitem& task,
                                 int ambiguityCount,
                                 int conflictCount) {
        int confidence = 85;
        if (!task.queueReady) confidence -= 25;
        confidence -= ambiguityCount * 10;
        confidence -= conflictCount * 8;
        if (task.prerequisiteOps.size() > 2) confidence -= 5;
        if (!task.dependencyTaskIds.empty()) confidence -= 4;
        return std::max(0, std::min(100, confidence));
    }

    static std::vector<std::string> deriveReasons(const GeneratedTaskitem& task,
                                                   int ambiguityCount,
                                                   int conflictCount,
                                                   int confidence) {
        std::vector<std::string> reasons;
        if (!task.queueReady) reasons.push_back("queue_not_ready");
        if (ambiguityCount > 0) reasons.push_back("ambiguous_requirements:" + std::to_string(ambiguityCount));
        if (conflictCount > 0) reasons.push_back("conflicts_detected:" + std::to_string(conflictCount));
        if (!task.dependencyTaskIds.empty()) reasons.push_back("depends_on_prior_tasks");
        if (confidence < 60) reasons.push_back("low_confidence");
        if (reasons.empty()) reasons.push_back("high_confidence_clear_path");
        return reasons;
    }

    static bool shouldEscalate(const AnnotatedTaskitem& item) {
        return item.confidence < 60 || item.ambiguityCount > 0 || !item.base.queueReady;
    }
};
