#pragma once

// Step 501: Scenario - Cross-Language Port
// End-to-end scenario for porting a Python ML pipeline to Rust.

#include "BehavioralEquivalence.h"
#include "ConcurrencyTranslator.h"
#include "IntentTranslator.h"
#include "MemoryModelTranslator.h"
#include "TranspilationConfidence.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

struct PortedFunctionResult {
    std::string name;
    std::string source;
    std::string intent;
    bool hotLoop = false;
    std::vector<std::string> annotations;
    TranslationChoice translation;
    ConfidenceScore confidence;
    BehavioralCheckResult equivalence;
};

struct CrossLanguagePortScenarioResult {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string pipelineSource;
    std::vector<PortedFunctionResult> functions;
    IntentTranslationResult intentTranslations;
    MemoryAnalysisResult memoryAnalysis;
    ConcurrencyAnalysisResult concurrencyAnalysis;
    std::vector<std::string> reviewRequiredFunctions;
    std::vector<std::string> notes;

    float averageConfidence() const {
        if (functions.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& f : functions) sum += f.confidence.score;
        return sum / static_cast<float>(functions.size());
    }
};

class CrossLanguagePortScenarioRunner {
public:
    static CrossLanguagePortScenarioResult run() {
        CrossLanguagePortScenarioResult out;
        out.sourceLanguage = "python";
        out.targetLanguage = "rust";

        const auto funcs = sampleFunctions();
        out.pipelineSource = composePipelineSource(funcs);
        out.intentTranslations = IntentTranslator::translate(funcs, out.sourceLanguage, out.targetLanguage);
        out.memoryAnalysis = MemoryModelTranslator::analyze(
            out.pipelineSource, out.sourceLanguage, out.targetLanguage);
        out.concurrencyAnalysis = ConcurrencyTranslator::analyze(
            out.pipelineSource, out.sourceLanguage, out.targetLanguage);

        for (size_t i = 0; i < funcs.size(); ++i) {
            PortedFunctionResult pf;
            pf.name = funcs[i].name;
            pf.source = funcs[i].source;
            pf.intent = funcs[i].intent;
            pf.hotLoop = isHotLoop(funcs[i].source);
            if (pf.hotLoop) pf.annotations.push_back("@HotLoop(true)");
            if (!funcs[i].intent.empty()) pf.annotations.push_back("@Intent(" + funcs[i].intent + ")");

            pf.translation = out.intentTranslations.getTranslation(funcs[i].name);
            pf.confidence = TranspilationScorer::score(
                funcs[i].name, funcs[i].source, pf.translation.targetCode,
                out.sourceLanguage, out.targetLanguage, !funcs[i].intent.empty());
            pf.equivalence = BehavioralChecker::check(
                funcs[i].name, funcs[i].source, pf.translation.targetCode,
                out.sourceLanguage, out.targetLanguage, {});

            if (pf.confidence.reviewRequired) out.reviewRequiredFunctions.push_back(pf.name);
            out.functions.push_back(std::move(pf));
        }

        out.notes.push_back("Cross-language Python->Rust scenario executed");
        out.notes.push_back("Includes hot-loop tagging, transpilation confidence, and review surfacing");
        return out;
    }

private:
    static std::vector<FunctionIntent> sampleFunctions() {
        return {
            {"train_epoch", "reduce gradient sum",
             "def train_epoch(samples):\n"
             "    total = 0\n"
             "    for i in range(len(samples)):\n"
             "        total += samples[i]\n"
             "    return total\n"},
            {"normalize_scores", "map transform normalize",
             "def normalize_scores(scores):\n"
             "    return [s / 255.0 for s in scores]\n"},
            {"async_fetch_batch", "",
             "import asyncio\n"
             "async def async_fetch_batch(client, urls):\n"
             "    out = []\n"
             "    for u in urls:\n"
             "        out.append(await client.get(u))\n"
             "    return out\n"}
        };
    }

    static std::string composePipelineSource(const std::vector<FunctionIntent>& funcs) {
        std::string out;
        for (const auto& f : funcs) {
            if (!out.empty()) out += "\n";
            out += f.source;
        }
        return out;
    }

    static bool isHotLoop(const std::string& source) {
        std::string s = source;
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s.find("for ") != std::string::npos ||
               s.find("while ") != std::string::npos ||
               s.find("range(") != std::string::npos;
    }
};
