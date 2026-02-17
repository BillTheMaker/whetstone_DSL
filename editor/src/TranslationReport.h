#pragma once

// Step 457: Translation Report — per-function and project-level reporting for
// transpilation choices, confidence, annotation changes, and review flags.

#include "IntentTranslator.h"
#include "TranspilationConfidence.h"

#include <string>
#include <vector>

struct AnnotationDelta {
    std::vector<std::string> added;
    std::vector<std::string> removed;
    std::vector<std::string> changed;
};

struct FunctionTranslationReport {
    std::string functionName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string sourceConstruct;
    std::string targetConstruct;
    float confidence = 0.0f;
    bool idiomatic = false;
    bool reviewRequired = false;
    std::string reviewReason;
    std::string safetyDelta; // gained/lost/neutral
    AnnotationDelta annotations;
    std::string rationale;
};

struct ProjectTranslationSummary {
    int totalFunctions = 0;
    int idiomaticCount = 0;
    int literalCount = 0;
    int flaggedForReviewCount = 0;

    float idiomaticPercent() const {
        return totalFunctions == 0 ? 0.0f :
            (100.0f * static_cast<float>(idiomaticCount) / static_cast<float>(totalFunctions));
    }
    float literalPercent() const {
        return totalFunctions == 0 ? 0.0f :
            (100.0f * static_cast<float>(literalCount) / static_cast<float>(totalFunctions));
    }
    float reviewPercent() const {
        return totalFunctions == 0 ? 0.0f :
            (100.0f * static_cast<float>(flaggedForReviewCount) / static_cast<float>(totalFunctions));
    }
};

struct TranslationReport {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<FunctionTranslationReport> functions;
    ProjectTranslationSummary summary;

    bool hasFunction(const std::string& functionName) const {
        for (const auto& fn : functions)
            if (fn.functionName == functionName) return true;
        return false;
    }

    FunctionTranslationReport getFunction(const std::string& functionName) const {
        for (const auto& fn : functions)
            if (fn.functionName == functionName) return fn;
        return {};
    }
};

class TranslationReportGenerator {
public:
    static TranslationReport generate(
        const IntentTranslationResult& translations,
        const std::vector<bool>& hasIntents = {}) {

        TranslationReport report;
        report.sourceLanguage = translations.sourceLanguage;
        report.targetLanguage = translations.targetLanguage;

        for (size_t i = 0; i < translations.translations.size(); ++i) {
            const auto& choice = translations.translations[i];
            bool hasIntent = choice.intent.size() > 0;
            if (i < hasIntents.size()) hasIntent = hasIntents[i];

            auto score = TranspilationScorer::score(
                choice.functionName,
                choice.sourceCode,
                choice.targetCode,
                translations.sourceLanguage,
                translations.targetLanguage,
                hasIntent);

            FunctionTranslationReport fn;
            fn.functionName = choice.functionName;
            fn.sourceLanguage = choice.sourceLanguage;
            fn.targetLanguage = choice.targetLanguage;
            fn.sourceConstruct = classifyConstruct(choice.sourceCode);
            fn.targetConstruct = classifyConstruct(choice.targetCode);
            fn.confidence = score.score;
            fn.idiomatic = choice.isIdiomatic;
            fn.reviewRequired = choice.needsReview || score.reviewRequired;
            fn.reviewReason = !choice.reviewReason.empty() ? choice.reviewReason : score.reviewReason;
            fn.safetyDelta = inferSafetyDelta(choice.sourceLanguage, choice.targetLanguage, choice.targetCode);
            fn.annotations = inferAnnotationDelta(choice, score);
            fn.rationale = buildRationale(choice, score);

            report.functions.push_back(std::move(fn));
        }

        report.summary = computeSummary(report.functions);
        return report;
    }

private:
    static std::string classifyConstruct(const std::string& code) {
        if (code.find("sort") != std::string::npos) return "sort";
        if (code.find("find") != std::string::npos || code.find("lookup") != std::string::npos) return "lookup";
        if (code.find("filter") != std::string::npos) return "filter";
        if (code.find("map") != std::string::npos || code.find("transform") != std::string::npos) return "transform";
        if (code.find("fold") != std::string::npos || code.find("sum") != std::string::npos ||
            code.find("accumulate") != std::string::npos) return "reduce";
        if (code.find("read") != std::string::npos || code.find("open(") != std::string::npos) return "io";
        if (code.find("TODO") != std::string::npos) return "unknown";
        return "structural";
    }

    static std::string inferSafetyDelta(const std::string& srcLang,
                                        const std::string& tgtLang,
                                        const std::string& targetCode) {
        if (srcLang == "c" && (tgtLang == "rust" || tgtLang == "java" || tgtLang == "python"))
            return "gained";
        if (srcLang == "rust" && tgtLang == "c")
            return "lost";
        if (targetCode.find("TODO") != std::string::npos)
            return "neutral";
        return "neutral";
    }

    static AnnotationDelta inferAnnotationDelta(const TranslationChoice& choice,
                                                const ConfidenceScore& score) {
        AnnotationDelta d;
        d.added.push_back(score.annotation);
        if (choice.isIdiomatic) d.added.push_back("@Idiomatic(true)");
        else d.added.push_back("@Idiomatic(false)");
        if (choice.needsReview || score.reviewRequired)
            d.added.push_back("@Review(required,human)");
        return d;
    }

    static std::string buildRationale(const TranslationChoice& choice,
                                      const ConfidenceScore& score) {
        if (choice.isIdiomatic)
            return "Selected idiomatic target construct for intent: " + choice.intent;
        if (score.category == TranslationCategory::StructuralWithIntent)
            return "Intent present but no direct idiomatic mapping; structural translation used";
        if (score.category == TranslationCategory::StructuralNoIntent)
            return "No explicit intent; literal/structural translation preserved";
        return "Unrecognized translation shape; review recommended";
    }

    static ProjectTranslationSummary computeSummary(
        const std::vector<FunctionTranslationReport>& functions) {
        ProjectTranslationSummary s;
        s.totalFunctions = static_cast<int>(functions.size());
        for (const auto& fn : functions) {
            if (fn.idiomatic) ++s.idiomaticCount;
            else ++s.literalCount;
            if (fn.reviewRequired) ++s.flaggedForReviewCount;
        }
        return s;
    }
};
