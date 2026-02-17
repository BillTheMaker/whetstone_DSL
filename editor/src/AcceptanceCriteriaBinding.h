#pragma once
// Step 581: Acceptance-Criteria Binding

#include "IntakeTextUtil.h"
#include "TaskitemConfidenceAmbiguity.h"

#include <string>
#include <vector>

struct BoundAcceptanceCheck {
    std::string checkId;
    std::string text;
    std::string testSkeleton;
};

struct BoundTaskitem {
    AnnotatedTaskitem task;
    std::vector<BoundAcceptanceCheck> checks;
    bool hasCoverage = false;
};

class AcceptanceCriteriaBinding {
public:
    static bool bind(const std::vector<AnnotatedTaskitem>& tasks,
                     const std::vector<NormalizedRequirement>& requirements,
                     std::vector<BoundTaskitem>* out,
                     std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->clear();
        if (tasks.empty()) {
            *error = "tasks_empty";
            return false;
        }

        std::vector<NormalizedRequirement> acceptance;
        for (const auto& requirement : requirements) {
            if (requirement.kind == NormalizedRequirementKind::Acceptance) acceptance.push_back(requirement);
        }
        if (acceptance.empty()) {
            *error = "acceptance_requirements_empty";
            return false;
        }

        for (const auto& task : tasks) {
            BoundTaskitem bound;
            bound.task = task;
            for (const auto& req : acceptance) {
                BoundAcceptanceCheck check;
                check.checkId = task.base.taskId + "::" + req.requirementId;
                check.text = req.normalizedText;
                check.testSkeleton = makeTestSkeleton(task.base.taskId, req.normalizedText);
                bound.checks.push_back(check);
            }
            bound.hasCoverage = !bound.checks.empty();
            out->push_back(bound);
        }
        return true;
    }

private:
    static std::string makeTestSkeleton(const std::string& taskId, const std::string& checkText) {
        const std::string a = intakeSanitizeToken(taskId, '_');
        const std::string b = intakeSanitizeToken(checkText, '_');
        return "test_" + (a.empty() ? std::string("task") : a) + "_" +
               (b.empty() ? std::string("check") : b);
    }
};
