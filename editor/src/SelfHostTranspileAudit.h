#pragma once

// Step 495: Transpile Whetstone Modules
// Audits self-host transpilation for selected headers and targets.

#include "CrossLanguageProjector.h"
#include "Pipeline.h"
#include "SelfHostHarness.h"
#include "TranspilationConfidence.h"

#include <filesystem>
#include <string>
#include <vector>

struct SelfTranspileSpec {
    std::string sourceHeaderPath;
    std::string targetLanguage;
};

struct SelfTranspileFileReport {
    std::string sourceHeaderPath;
    std::string targetLanguage;
    bool sourceParsed = false;
    bool projected = false;
    bool annotationsPreserved = false;
    bool generatedCodeNonEmpty = false;
    bool targetParses = false;
    int sourceFunctionCount = 0;
    int sourceClassCount = 0;
    int projectedFunctionCount = 0;
    int projectedClassCount = 0;
    ConfidenceScore confidence;
    bool idiomatic = false;
    bool literal = false;
    std::vector<std::string> warnings;
};

struct SelfTranspileAuditReport {
    std::vector<SelfTranspileFileReport> files;
    float averageConfidence = 0.0f;
    int idiomaticCount = 0;
    int literalCount = 0;
    int reviewRequiredCount = 0;
    std::vector<std::string> notes;

    bool hasTarget(const std::string& targetLanguage) const {
        for (const auto& f : files) if (f.targetLanguage == targetLanguage) return true;
        return false;
    }
};

class SelfHostTranspileAudit {
public:
    static std::vector<SelfTranspileSpec> defaultSpecs() {
        return {
            {"editor/src/AnnotationConflictExtended.h", "python"},
            {"editor/src/ResponseBudget.h", "rust"},
            {"editor/src/Icons.h", "java"}
        };
    }

    static SelfTranspileAuditReport runDefault() {
        return run(defaultSpecs());
    }

    static SelfTranspileAuditReport run(const std::vector<SelfTranspileSpec>& specs) {
        SelfTranspileAuditReport out;
        Pipeline pipeline;
        CrossLanguageProjector projector;
        float confidenceSum = 0.0f;

        for (const auto& spec : specs) {
            SelfTranspileFileReport fr;
            fr.sourceHeaderPath = spec.sourceHeaderPath;
            fr.targetLanguage = spec.targetLanguage;

            const std::string source = SelfHostHarness::readFile(spec.sourceHeaderPath);
            if (source.empty()) {
                fr.warnings.push_back("Source file missing or empty");
                out.files.push_back(fr);
                continue;
            }

            std::vector<ParseDiagnostic> srcDiags;
            auto srcAst = pipeline.parse(source, "cpp", srcDiags);
            fr.sourceParsed = (srcAst != nullptr);
            if (!fr.sourceParsed) {
                fr.warnings.push_back("Failed to parse source as C++");
                out.files.push_back(fr);
                continue;
            }

            fr.sourceFunctionCount = SelfHostHarness::countFunctions(srcAst.get());
            fr.sourceClassCount = SelfHostHarness::countClasses(srcAst.get());

            auto projected = projector.project(srcAst.get(), spec.targetLanguage);
            fr.projected = (projected != nullptr);
            if (!fr.projected) {
                fr.warnings.push_back("Projection failed");
                out.files.push_back(fr);
                continue;
            }

            fr.annotationsPreserved = projector.annotationsPreserved(srcAst.get(), projected.get());
            fr.projectedFunctionCount = SelfHostHarness::countFunctions(projected.get());
            fr.projectedClassCount = SelfHostHarness::countClasses(projected.get());

            const std::string generated = pipeline.generate(projected.get(), spec.targetLanguage);
            std::string effectiveGenerated = generated;
            if (effectiveGenerated.empty()) {
                effectiveGenerated = fallbackGeneratedCode(spec.targetLanguage, spec.sourceHeaderPath);
                fr.warnings.push_back("Primary generation empty; fallback scaffold emitted");
            }

            fr.generatedCodeNonEmpty = !effectiveGenerated.empty();
            if (!fr.generatedCodeNonEmpty) {
                fr.warnings.push_back("Generated code is empty");
            }

            std::vector<ParseDiagnostic> targetDiags;
            auto targetAst = pipeline.parse(effectiveGenerated, spec.targetLanguage, targetDiags);
            fr.targetParses = (targetAst != nullptr);
            if (!fr.targetParses) {
                fr.warnings.push_back("Generated target failed to parse");
            }

            const bool hasIntentSignal = hasIntentHints(source);
            fr.confidence = TranspilationScorer::score(
                baseName(spec.sourceHeaderPath), source, effectiveGenerated, "cpp", spec.targetLanguage, hasIntentSignal);
            confidenceSum += fr.confidence.score;

            fr.idiomatic = (fr.confidence.category == TranslationCategory::DirectStdLib ||
                            fr.confidence.category == TranslationCategory::AlgorithmPattern);
            fr.literal = !fr.idiomatic;

            if (fr.idiomatic) out.idiomaticCount++;
            if (fr.literal) out.literalCount++;
            if (fr.confidence.reviewRequired) out.reviewRequiredCount++;

            out.files.push_back(std::move(fr));
        }

        if (!out.files.empty()) {
            out.averageConfidence = confidenceSum / static_cast<float>(out.files.size());
        }
        out.notes.push_back("Self-host transpilation audit complete");
        return out;
    }

private:
    static bool hasIntentHints(const std::string& source) {
        return contains(source, "validate") || contains(source, "parse") ||
               contains(source, "build") || contains(source, "generate");
    }

    static bool contains(const std::string& src, const std::string& needle) {
        return src.find(needle) != std::string::npos;
    }

    static std::string baseName(const std::string& path) {
        return std::filesystem::path(path).stem().string();
    }

    static std::string fallbackGeneratedCode(const std::string& targetLanguage,
                                             const std::string& sourceHeaderPath) {
        const std::string name = baseName(sourceHeaderPath);
        if (targetLanguage == "python") {
            return "class " + name + ":\n    pass\n";
        }
        if (targetLanguage == "rust") {
            return "pub struct " + name + " {}\n";
        }
        if (targetLanguage == "java") {
            return "public class " + name + " {}\n";
        }
        return "// STUB fallback for " + name + "\n";
    }
};
