#pragma once

// Step 456: Transpilation Confidence Scorer — scores translations 0.0-1.0
// and auto-adds @Review(required,human) for low-confidence translations.

#include <string>
#include <vector>

enum class TranslationCategory {
    DirectStdLib,        // 0.95 — direct standard library mapping
    AlgorithmPattern,    // 0.85 — recognized algorithm pattern
    StructuralWithIntent,// 0.70 — structural translation with @Intent
    StructuralNoIntent,  // 0.50 — structural translation, no intent
    Unknown              // 0.30 — unknown/unrecognized pattern
};

struct ConfidenceScore {
    std::string functionName;
    std::string sourceLanguage;
    std::string targetLanguage;
    TranslationCategory category = TranslationCategory::Unknown;
    float score = 0.0f;
    bool reviewRequired = false;
    std::string annotation;      // e.g. @Confidence(0.85)
    std::string reviewReason;
};

struct ConfidenceReport {
    std::vector<ConfidenceScore> scores;
    std::string sourceLanguage;
    std::string targetLanguage;

    float averageScore() const {
        if (scores.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& s : scores) sum += s.score;
        return sum / (float)scores.size();
    }

    int countReviewRequired() const {
        int n = 0;
        for (const auto& s : scores)
            if (s.reviewRequired) ++n;
        return n;
    }

    int countByCategory(TranslationCategory cat) const {
        int n = 0;
        for (const auto& s : scores)
            if (s.category == cat) ++n;
        return n;
    }

    ConfidenceScore getScore(const std::string& funcName) const {
        for (const auto& s : scores)
            if (s.functionName == funcName) return s;
        return {};
    }
};

class TranspilationScorer {
public:
    static ConfidenceScore score(const std::string& functionName,
                                  const std::string& sourceCode,
                                  const std::string& targetCode,
                                  const std::string& srcLang,
                                  const std::string& tgtLang,
                                  bool hasIntent = false) {
        ConfidenceScore cs;
        cs.functionName = functionName;
        cs.sourceLanguage = srcLang;
        cs.targetLanguage = tgtLang;
        cs.category = classify(sourceCode, targetCode, hasIntent);
        cs.score = categoryScore(cs.category);

        if (cs.score < 0.60f) {
            cs.reviewRequired = true;
            cs.annotation = "@Confidence(" + std::to_string(cs.score).substr(0, 4) +
                            "), @Review(required,human)";
            cs.reviewReason = "Low confidence translation — manual review needed";
        } else {
            cs.annotation = "@Confidence(" + std::to_string(cs.score).substr(0, 4) + ")";
        }

        return cs;
    }

    static ConfidenceReport scoreAll(
        const std::vector<std::pair<std::string, std::pair<std::string, std::string>>>& functions,
        const std::string& srcLang,
        const std::string& tgtLang,
        const std::vector<bool>& hasIntents = {}) {

        ConfidenceReport report;
        report.sourceLanguage = srcLang;
        report.targetLanguage = tgtLang;

        for (size_t i = 0; i < functions.size(); ++i) {
            bool intent = (i < hasIntents.size()) ? hasIntents[i] : false;
            auto cs = score(functions[i].first,
                           functions[i].second.first,
                           functions[i].second.second,
                           srcLang, tgtLang, intent);
            report.scores.push_back(std::move(cs));
        }
        return report;
    }

    static float categoryScore(TranslationCategory cat) {
        switch (cat) {
            case TranslationCategory::DirectStdLib:         return 0.95f;
            case TranslationCategory::AlgorithmPattern:     return 0.85f;
            case TranslationCategory::StructuralWithIntent: return 0.70f;
            case TranslationCategory::StructuralNoIntent:   return 0.50f;
            case TranslationCategory::Unknown:              return 0.30f;
        }
        return 0.30f;
    }

private:
    static TranslationCategory classify(const std::string& source,
                                         const std::string& target,
                                         bool hasIntent) {
        // Direct stdlib mapping
        if (target.find(".sort()") != std::string::npos ||
            target.find("sorted(") != std::string::npos ||
            target.find("Collections.sort") != std::string::npos ||
            target.find("std::sort") != std::string::npos ||
            target.find("binary_search") != std::string::npos ||
            target.find("read_to_string") != std::string::npos ||
            target.find("ReadFile") != std::string::npos)
            return TranslationCategory::DirectStdLib;

        // Algorithm pattern: iterator chains, fold, filter, map
        if (target.find(".iter()") != std::string::npos ||
            target.find(".stream()") != std::string::npos ||
            target.find(".fold(") != std::string::npos ||
            target.find(".filter(") != std::string::npos ||
            target.find(".map(") != std::string::npos ||
            target.find("accumulate") != std::string::npos)
            return TranslationCategory::AlgorithmPattern;

        // Structural with or without intent
        if (hasIntent)
            return TranslationCategory::StructuralWithIntent;

        // Check if it looks like real code or just a placeholder stub
        if (target.find("TODO") != std::string::npos ||
            target.find("STUB") != std::string::npos ||
            target.empty())
            return TranslationCategory::Unknown;

        return TranslationCategory::StructuralNoIntent;
    }
};
