#pragma once
// Step 386: External result acceptance protocol for workflow orchestration.

#include "Pipeline.h"
#include "ReviewGate.h"
#include <string>
#include <vector>

struct ResultSubmission {
    std::string itemId;
    std::string generatedCode;
    float confidence = 0.0f;
    std::string reasoning;
    std::vector<json> suggestedAnnotations;

    static ResultSubmission fromJson(const json& j) {
        ResultSubmission s;
        s.itemId = j.value("itemId", "");
        s.generatedCode = j.value("generatedCode", "");
        s.confidence = j.value("confidence", 0.0f);
        s.reasoning = j.value("reasoning", "");
        if (j.contains("suggestedAnnotations") && j["suggestedAnnotations"].is_array()) {
            s.suggestedAnnotations = j["suggestedAnnotations"].get<std::vector<json>>();
        }
        return s;
    }
};

struct ResultAcceptance {
    bool accepted = false;
    bool validationPassed = false;
    int diagnosticCount = 0;
    bool autoApproved = false;
    bool reviewRequired = true;
    std::vector<std::string> validationErrors;

    json toJson() const {
        return {
            {"accepted", accepted},
            {"validationPassed", validationPassed},
            {"diagnosticCount", diagnosticCount},
            {"autoApproved", autoApproved},
            {"reviewRequired", reviewRequired},
            {"validationErrors", validationErrors}
        };
    }
};

inline std::vector<std::string> validateSuggestedAnnotations(
    const std::vector<json>& suggestedAnnotations) {
    std::vector<std::string> errors;
    for (size_t i = 0; i < suggestedAnnotations.size(); ++i) {
        const auto& a = suggestedAnnotations[i];
        if (!a.is_object()) {
            errors.push_back("Suggested annotation #" + std::to_string(i) + " must be an object");
            continue;
        }
        if (!a.contains("nodeId") || !a.contains("annotationType")) {
            errors.push_back("Suggested annotation #" + std::to_string(i) +
                             " requires nodeId and annotationType");
        }
    }
    return errors;
}

inline ResultAcceptance evaluateResultSubmission(const ResultSubmission& submission,
                                                 const WorkItem& item,
                                                 const std::string& language,
                                                 const ReviewGate& reviewGate,
                                                 const ReviewPolicy& reviewPolicy,
                                                 WorkItemResult& outResult) {
    ResultAcceptance acceptance;
    outResult = WorkItemResult{};
    outResult.generatedCode = submission.generatedCode;
    outResult.confidence = submission.confidence;
    outResult.reasoning = submission.reasoning;
    outResult.tokensGenerated = static_cast<int>(submission.generatedCode.size() / 4);
    outResult.tokensBudget = estimateContextBudget(item.contextWidth);

    if (submission.generatedCode.empty()) {
        acceptance.validationErrors.push_back("Generated code is empty");
        acceptance.validationPassed = false;
        return acceptance;
    }

    Pipeline pipeline;
    std::vector<ParseDiagnostic> parseDiags;
    auto parsed = pipeline.parse(submission.generatedCode,
                                 language.empty() ? "python" : language,
                                 parseDiags);
    for (const auto& d : parseDiags) {
        acceptance.validationErrors.push_back(
            "Parse error line " + std::to_string(d.line) + ": " + d.message);
    }
    acceptance.diagnosticCount = static_cast<int>(parseDiags.size());

    auto annotationErrors = validateSuggestedAnnotations(submission.suggestedAnnotations);
    for (const auto& err : annotationErrors) {
        acceptance.validationErrors.push_back(err);
    }
    acceptance.diagnosticCount += static_cast<int>(annotationErrors.size());

    if (!submission.suggestedAnnotations.empty()) {
        outResult.astJson["suggestedAnnotations"] = submission.suggestedAnnotations;
    }
    if (parsed) {
        outResult.astJson["parsed"] = true;
        outResult.astJson["language"] = language.empty() ? "python" : language;
    }

    if (!acceptance.validationErrors.empty() || !parsed) {
        acceptance.validationPassed = false;
        acceptance.accepted = false;
        acceptance.reviewRequired = true;
        return acceptance;
    }

    acceptance.validationPassed = true;
    ReviewDecision reviewDecision =
        reviewGate.shouldAutoApprove(item, outResult, reviewPolicy);
    acceptance.autoApproved = reviewDecision.approved;
    acceptance.reviewRequired = !reviewDecision.approved;
    acceptance.accepted = true;
    return acceptance;
}
