#pragma once
// Step 581: Acceptance-Criteria Binding

#include "TaskitemConfidenceAmbiguity.h"

#include <algorithm>
#include <cctype>
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
        return "test_" + sanitize(taskId) + "_" + sanitize(checkText);
    }

    static std::string sanitize(const std::string& text) {
        std::string out;
        bool lastUnderscore = false;
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                lastUnderscore = false;
            } else if (!lastUnderscore) {
                out.push_back('_');
                lastUnderscore = true;
            }
        }
        while (!out.empty() && out.front() == '_') out.erase(out.begin());
        while (!out.empty() && out.back() == '_') out.pop_back();
        return out.empty() ? "check" : out;
    }
};
