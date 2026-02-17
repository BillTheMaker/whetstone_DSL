#pragma once

// Step 500: Scenario - Legacy Modernization
// End-to-end modernization scenario for legacy C -> Rust migration.

#include "APIBoundaryPreserver.h"
#include "LegacyIdiomDetector.h"
#include "MigrationExecutor.h"
#include "MigrationPlanGenerator.h"
#include "MigrationTestGenerator.h"
#include "ModernizationSuggester.h"
#include "ModernizationWorkflow.h"
#include "SafetyAuditor.h"

#include <string>
#include <vector>

struct LegacyModernizationScenarioResult {
    std::string legacySource;
    LegacyAnalysisReport legacyReport;
    SafetyReport safetyReport;
    ModernizationReport modernizationReport;
    ModernizationWorkflow modernizationWorkflow;
    MigrationPlan migrationPlan;
    MigrationExecution migrationExecution;
    APIBoundaryReport apiBoundaryOriginal;
    APIBoundaryReport apiBoundaryMigrated;
    bool apiBoundaryPreserved = false;
    bool securityOwnershipStrengthened = false;
    MigrationTestSuite migrationTests;
    std::vector<std::string> notes;
};

class LegacyModernizationScenarioRunner {
public:
    static LegacyModernizationScenarioResult run() {
        LegacyModernizationScenarioResult out;
        out.legacySource = sampleLegacySource();

        out.legacyReport = LegacyIdiomDetector::analyzeFile(out.legacySource, "c", "legacy.c");
        out.safetyReport = SafetyAuditor::audit(out.legacySource, "c", "legacy.c");
        out.modernizationReport =
            ModernizationSuggester::suggest(out.legacyReport, out.safetyReport, "rust");
        out.modernizationWorkflow =
            ModernizationWorkflowGenerator::generate(out.legacySource, out.modernizationReport);

        auto files = sampleProjectFiles();
        out.migrationPlan = MigrationPlanGenerator::generate("legacy_modernization", files, "c", "rust");
        out.migrationExecution = MigrationExecutor::createExecution(out.migrationPlan, files);

        if (!out.migrationPlan.units.empty()) {
            const auto& unit = out.migrationPlan.units.front();
            out.apiBoundaryOriginal = APIBoundaryPreserver::analyze(files.front().source, unit);
            out.apiBoundaryMigrated = APIBoundaryPreserver::analyze(sampleMigratedRustSource(), unit);
            out.apiBoundaryPreserved =
                APIBoundaryPreserver::verifyPreservation(out.apiBoundaryOriginal, out.apiBoundaryMigrated);
            out.migrationTests = MigrationTestGenerator::generate(out.apiBoundaryOriginal);
        }

        out.securityOwnershipStrengthened =
            out.modernizationReport.hasSuggestionTo("Rust ownership + borrow checker");

        out.notes.push_back("Legacy C modernization scenario executed");
        out.notes.push_back("Pipeline: ingest -> audit -> suggest -> workflow -> migration plan");
        return out;
    }

private:
    static std::string sampleLegacySource() {
        return
            "#include <stdlib.h>\n"
            "#include <stdio.h>\n"
            "#include <string.h>\n"
            "int shorten(const char* input, char* out) {\n"
            "    char* p = (char*)malloc(32);\n"
            "    if (!p) return -1;\n"
            "    strcpy(p, input);\n"
            "    sprintf(out, \"%s\", p);\n"
            "    char c = *(p + 1);\n"
            "    free(p);\n"
            "    return (int)c;\n"
            "}\n";
    }

    static std::string sampleMigratedRustSource() {
        return
            "#[no_mangle]\n"
            "pub extern \"C\" fn clamp_small(x: i32, lo: i32, hi: i32) -> i32 { x }\n";
    }

    static std::vector<FileInfo> sampleProjectFiles() {
        std::vector<FileInfo> files;
        files.push_back({
            "src/simple_utils.c",
            "int clamp_small(int x, int lo, int hi) {\n"
            "    if (x < lo) return lo;\n"
            "    if (x > hi) return hi;\n"
            "    return x;\n"
            "}\n",
            {"clamp_small"},
            {}
        });

        std::string complex =
            "int normalize_token(const char* in, char* out) {\n"
            "    int score = 0;\n"
            "    if (!in || !out) return -1;\n"
            "    out[0] = '\\0';\n";
        for (int i = 0; i < 58; ++i) {
            complex += "    score += " + std::to_string((i % 5) + 1) + ";\n";
        }
        complex +=
            "    if (score > 1000) score = 1000;\n"
            "    return score;\n"
            "}\n";

        files.push_back({
            "src/complex_transform.c",
            complex,
            {"normalize_token"},
            {}
        });
        return files;
    }
};
