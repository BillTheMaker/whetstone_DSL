#pragma once
// Step 385: Standard context bundle format for external model invocation.

#include "RoutingEngine.h"
#include "WorkerRegistry.h"
#include <sstream>
#include <string>
#include <vector>

struct AttemptSummary {
    std::string workerType;
    std::string generatedCode;
    std::string feedback;

    json toJson() const {
        return {
            {"workerType", workerType},
            {"generatedCode", generatedCode},
            {"feedback", feedback}
        };
    }

    static AttemptSummary fromJson(const json& j) {
        AttemptSummary a;
        a.workerType = j.value("workerType", "");
        a.generatedCode = j.value("generatedCode", "");
        a.feedback = j.value("feedback", "");
        return a;
    }
};

struct ContextBundle {
    std::string taskDescription;
    std::string skeletonCode;
    std::string intent;
    std::vector<std::string> constraints;
    std::string contextCode;
    std::string projectSummary;
    std::string existingTests;
    std::vector<AttemptSummary> previousAttempts;
    int tokenBudget = 0;
    std::string outputFormat; // "code-only" | "code-with-explanation"

    json toJson() const {
        json attempts = json::array();
        for (const auto& a : previousAttempts) attempts.push_back(a.toJson());
        return {
            {"taskDescription", taskDescription},
            {"skeletonCode", skeletonCode},
            {"intent", intent},
            {"constraints", constraints},
            {"contextCode", contextCode},
            {"projectSummary", projectSummary},
            {"existingTests", existingTests},
            {"previousAttempts", attempts},
            {"tokenBudget", tokenBudget},
            {"outputFormat", outputFormat}
        };
    }

    static ContextBundle fromJson(const json& j) {
        ContextBundle b;
        b.taskDescription = j.value("taskDescription", "");
        b.skeletonCode = j.value("skeletonCode", "");
        b.intent = j.value("intent", "");
        if (j.contains("constraints") && j["constraints"].is_array()) {
            b.constraints = j["constraints"].get<std::vector<std::string>>();
        }
        b.contextCode = j.value("contextCode", "");
        b.projectSummary = j.value("projectSummary", "");
        b.existingTests = j.value("existingTests", "");
        if (j.contains("previousAttempts") && j["previousAttempts"].is_array()) {
            for (const auto& a : j["previousAttempts"]) {
                b.previousAttempts.push_back(AttemptSummary::fromJson(a));
            }
        }
        b.tokenBudget = j.value("tokenBudget", 0);
        b.outputFormat = j.value("outputFormat", "code-with-explanation");
        return b;
    }
};

inline std::vector<std::string> collectBundleConstraints(const WorkerContext& context,
                                                         const WorkItem& item) {
    std::vector<std::string> out;
    for (const auto& anno : context.annotations) {
        std::string type = anno.value("type", "");
        std::string name = anno.value("name", "");
        std::string value = anno.value("value", "");
        if (!type.empty() || !name.empty() || !value.empty()) {
            std::string line = type;
            if (!name.empty()) line += ":" + name;
            if (!value.empty()) line += "=" + value;
            out.push_back(line);
        }
    }
    if (item.reviewRequired) out.push_back("requires-review");
    if (out.empty()) out.push_back("none");
    return out;
}

inline ContextBundle buildBundle(const WorkItem& item,
                                 const WorkerContext& context,
                                 const RoutingDecision& routingDecision) {
    ContextBundle bundle;
    bundle.taskDescription = "Implement " + item.nodeType + " '" + item.nodeName + "'";
    bundle.skeletonCode = context.nodeAst.is_null() ? "" : context.nodeAst.dump(2);
    bundle.intent = context.skeletonIntent;
    if (bundle.intent.empty() && !item.rejectionFeedback.empty()) {
        bundle.intent = item.rejectionFeedback;
    }
    bundle.constraints = collectBundleConstraints(context, item);
    bundle.contextCode = context.bufferContent;
    bundle.projectSummary = context.projectSummary.is_null() ? "" :
                            context.projectSummary.dump(2);
    bundle.existingTests = "";

    for (const auto& attempt : item.rejectionHistory) {
        bundle.previousAttempts.push_back({
            attempt.workerType,
            attempt.result.generatedCode,
            attempt.feedback
        });
    }

    bundle.tokenBudget = routingDecision.contextBudgetTokens > 0
        ? routingDecision.contextBudgetTokens
        : estimateContextBudget(routingDecision.contextWidth);
    bundle.outputFormat = (routingDecision.workerType == "deterministic" ||
                           routingDecision.workerType == "template")
        ? "code-only"
        : "code-with-explanation";
    return bundle;
}

inline std::string bundleToPrompt(const ContextBundle& bundle) {
    std::ostringstream oss;
    oss << "Task:\n" << bundle.taskDescription << "\n\n";
    oss << "Intent:\n" << (bundle.intent.empty() ? "(none)" : bundle.intent) << "\n\n";
    oss << "Constraints:\n";
    for (const auto& c : bundle.constraints) {
        oss << "- " << c << "\n";
    }
    oss << "\nSkeleton:\n" << (bundle.skeletonCode.empty() ? "(none)" : bundle.skeletonCode) << "\n\n";
    oss << "Context Code:\n" << (bundle.contextCode.empty() ? "(none)" : bundle.contextCode) << "\n\n";
    oss << "Project Summary:\n" << (bundle.projectSummary.empty() ? "(none)" : bundle.projectSummary) << "\n\n";
    oss << "Previous Attempts:\n";
    if (bundle.previousAttempts.empty()) {
        oss << "(none)\n";
    } else {
        for (const auto& a : bundle.previousAttempts) {
            oss << "- worker=" << a.workerType << "\n";
            if (!a.generatedCode.empty()) oss << "  code:\n" << a.generatedCode << "\n";
            if (!a.feedback.empty()) oss << "  feedback: " << a.feedback << "\n";
        }
    }
    oss << "\nToken Budget: " << bundle.tokenBudget << "\n";
    oss << "Output Format: " << bundle.outputFormat << "\n";
    return oss.str();
}

inline json bundleToJson(const ContextBundle& bundle) {
    return bundle.toJson();
}

inline int estimateBundleTokens(const ContextBundle& bundle) {
    return static_cast<int>(bundle.toJson().dump().size() / 4);
}
