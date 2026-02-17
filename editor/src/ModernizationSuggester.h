#pragma once

// Step 440: Modernization suggestions — maps legacy patterns to modern replacements
// with @Modernize annotations and effort classification.

#include "LegacyIdiomDetector.h"
#include "SafetyAuditor.h"

#include <string>
#include <vector>

enum class ModernizeEffort { QuickWin, Moderate, DeepRefactor };

struct ModernizeSuggestion {
    std::string fromPattern;
    std::string toReplacement;
    std::string annotation;  // @Modernize(from=..., to=..., risk=...)
    std::string risk;        // "low", "medium", "high"
    ModernizeEffort effort = ModernizeEffort::QuickWin;
    std::string rationale;
};

struct ModernizationReport {
    std::string filePath;
    std::string sourceLanguage;
    std::string targetLanguage;  // may differ for cross-language modernization
    std::vector<ModernizeSuggestion> suggestions;

    int countByEffort(ModernizeEffort e) const {
        int n = 0;
        for (const auto& s : suggestions)
            if (s.effort == e) ++n;
        return n;
    }

    bool hasSuggestionFrom(const std::string& from) const {
        for (const auto& s : suggestions)
            if (s.fromPattern == from) return true;
        return false;
    }

    bool hasSuggestionTo(const std::string& to) const {
        for (const auto& s : suggestions)
            if (s.toReplacement == to) return true;
        return false;
    }

    std::vector<ModernizeSuggestion> quickWins() const {
        std::vector<ModernizeSuggestion> out;
        for (const auto& s : suggestions)
            if (s.effort == ModernizeEffort::QuickWin) out.push_back(s);
        return out;
    }

    std::vector<ModernizeSuggestion> deepRefactors() const {
        std::vector<ModernizeSuggestion> out;
        for (const auto& s : suggestions)
            if (s.effort == ModernizeEffort::DeepRefactor) out.push_back(s);
        return out;
    }
};

class ModernizationSuggester {
public:
    static ModernizationReport suggest(const LegacyAnalysisReport& legacy,
                                       const SafetyReport& safety,
                                       const std::string& targetLanguage = "") {
        ModernizationReport report;
        report.filePath = legacy.filePath;
        report.sourceLanguage = legacy.language;
        report.targetLanguage = targetLanguage.empty() ? legacy.language : targetLanguage;

        suggestFromLegacyFindings(legacy, report);
        suggestFromSafetyFindings(safety, report);
        suggestCrossLanguage(legacy, report);
        return report;
    }

private:
    static void suggestFromLegacyFindings(const LegacyAnalysisReport& legacy,
                                          ModernizationReport& r) {
        for (const auto& f : legacy.findings) {
            if (f.pattern == "manual-memory-management") {
                r.suggestions.push_back({
                    "malloc/free", "std::unique_ptr / std::shared_ptr",
                    "@Modernize(from=\"malloc/free\", to=\"smart_pointers\", risk=\"medium\")",
                    "medium", ModernizeEffort::Moderate,
                    "Replace manual memory management with RAII smart pointers"
                });
            }
            if (f.pattern == "deprecated-api-sprintf") {
                r.suggestions.push_back({
                    "sprintf", "std::format / snprintf",
                    "@Modernize(from=\"sprintf\", to=\"std::format\", risk=\"low\")",
                    "low", ModernizeEffort::QuickWin,
                    "Replace sprintf with bounds-checked snprintf or C++20 std::format"
                });
            }
            if (f.pattern == "deprecated-api-strcpy") {
                r.suggestions.push_back({
                    "strcpy", "std::string / strncpy",
                    "@Modernize(from=\"strcpy\", to=\"std::string\", risk=\"low\")",
                    "low", ModernizeEffort::QuickWin,
                    "Replace strcpy with std::string or bounds-checked strncpy"
                });
            }
            if (f.pattern == "deprecated-api-gets") {
                r.suggestions.push_back({
                    "gets", "fgets / std::getline",
                    "@Modernize(from=\"gets\", to=\"fgets\", risk=\"low\")",
                    "low", ModernizeEffort::QuickWin,
                    "Replace gets with fgets (C) or std::getline (C++)"
                });
            }
            if (f.pattern == "goto-control-flow") {
                r.suggestions.push_back({
                    "goto", "structured control flow",
                    "@Modernize(from=\"goto\", to=\"structured_control\", risk=\"high\")",
                    "high", ModernizeEffort::DeepRefactor,
                    "Refactor goto-based control flow to loops/conditionals/early returns"
                });
            }
            if (f.pattern == "pointer-arithmetic") {
                r.suggestions.push_back({
                    "pointer arithmetic", "std::span / iterators",
                    "@Modernize(from=\"pointer_arithmetic\", to=\"std::span\", risk=\"medium\")",
                    "medium", ModernizeEffort::Moderate,
                    "Replace raw pointer arithmetic with std::span or container iterators"
                });
            }
            if (f.pattern == "knr-declaration") {
                r.suggestions.push_back({
                    "K&R declarations", "ANSI C prototypes",
                    "@Modernize(from=\"knr_decl\", to=\"ansi_prototype\", risk=\"low\")",
                    "low", ModernizeEffort::QuickWin,
                    "Convert K&R function declarations to ANSI C style"
                });
            }
        }
    }

    static void suggestFromSafetyFindings(const SafetyReport& safety,
                                          ModernizationReport& r) {
        for (const auto& f : safety.findings) {
            if (f.category == "race-condition" && f.annotation == "@Sync(none)") {
                r.suggestions.push_back({
                    "unprotected shared state", "std::mutex / std::atomic",
                    "@Modernize(from=\"unprotected_shared\", to=\"std::mutex\", risk=\"high\")",
                    "high", ModernizeEffort::DeepRefactor,
                    "Add synchronization primitives to protect shared mutable state"
                });
            }
            if (f.category == "integer-overflow") {
                r.suggestions.push_back({
                    "unchecked arithmetic", "checked arithmetic / SafeInt",
                    "@Modernize(from=\"unchecked_arithmetic\", to=\"checked_arithmetic\", risk=\"medium\")",
                    "medium", ModernizeEffort::Moderate,
                    "Add overflow checks or use safe integer types"
                });
            }
        }
    }

    static void suggestCrossLanguage(const LegacyAnalysisReport& legacy,
                                     ModernizationReport& r) {
        if (r.targetLanguage == r.sourceLanguage) return;

        std::string tgt = r.targetLanguage;
        if (tgt == "rust") {
            // C/C++ → Rust: ownership model replaces manual memory
            if (legacy.hasPattern("manual-memory-management") ||
                legacy.hasPattern("pointer-arithmetic")) {
                r.suggestions.push_back({
                    "manual memory (C/C++)", "Rust ownership + borrow checker",
                    "@Modernize(from=\"manual_memory\", to=\"rust_ownership\", risk=\"high\")",
                    "high", ModernizeEffort::DeepRefactor,
                    "Rust's ownership model eliminates manual memory management entirely"
                });
            }
        }
        if (tgt == "java" || tgt == "python") {
            if (legacy.hasPattern("manual-memory-management")) {
                r.suggestions.push_back({
                    "manual memory (C/C++)", "garbage collection (" + tgt + ")",
                    "@Modernize(from=\"manual_memory\", to=\"gc_" + tgt + "\", risk=\"medium\")",
                    "medium", ModernizeEffort::Moderate,
                    "Target language uses GC — manual memory management eliminated"
                });
            }
        }
    }
};
