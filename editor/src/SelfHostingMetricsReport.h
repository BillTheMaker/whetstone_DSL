#pragma once

// Step 497: Self-Hosting Metrics Report
// Aggregates parse, annotation, transpilation, and workflow viability metrics.

#include "SelfHostAnnotationAudit.h"
#include "SelfHostCodebaseAudit.h"
#include "SelfHostTranspileAudit.h"
#include "SelfModernizationWorkflow.h"

#include <string>
#include <vector>

struct SelfHostingMetrics {
    int filesParsed = 0;
    int filesTotal = 0;
    double parseCoveragePercent = 0.0;

    int inferredAnnotations = 0;
    int annotationSignals = 0;
    double annotationSignalDensity = 0.0;

    float averageTranspileConfidence = 0.0f;
    int transpileReviewRequired = 0;
    int idiomaticTranslations = 0;

    int workflowTasks = 0;
    int workflowDeterministicValid = 0;
    bool workflowViable = false;

    std::vector<std::string> gaps;
    std::vector<std::string> notes;
};

class SelfHostingMetricsReport {
public:
    static SelfHostingMetrics generate(const CodebaseParseAudit& parseAudit,
                                       const SelfAnnotationAuditReport& annotationAudit,
                                       const SelfTranspileAuditReport& transpileAudit,
                                       const ModernizationWorkflowReport& workflowAudit) {
        SelfHostingMetrics out;

        out.filesParsed = parseAudit.parsedFiles;
        out.filesTotal = parseAudit.totalFiles;
        out.parseCoveragePercent = parseAudit.parseRate() * 100.0;

        out.inferredAnnotations = annotationAudit.inferredTotal;
        out.annotationSignals = annotationAudit.complexityCount + annotationAudit.riskCount +
                                annotationAudit.ownerCount + annotationAudit.intentCount +
                                annotationAudit.tailCallCount;
        if (annotationAudit.parsedFiles > 0) {
            out.annotationSignalDensity =
                static_cast<double>(out.annotationSignals) /
                static_cast<double>(annotationAudit.parsedFiles);
        }

        out.averageTranspileConfidence = transpileAudit.averageConfidence;
        out.transpileReviewRequired = transpileAudit.reviewRequiredCount;
        out.idiomaticTranslations = transpileAudit.idiomaticCount;

        out.workflowTasks = static_cast<int>(workflowAudit.tasks.size());
        out.workflowDeterministicValid = workflowAudit.deterministicValidCount;
        out.workflowViable = (workflowAudit.deterministicCount == workflowAudit.deterministicValidCount);

        fillGaps(out);
        out.notes.push_back("Self-hosting metrics report generated");
        return out;
    }

    static SelfHostingMetrics generateFromCurrentState() {
        auto parse = SelfHostCodebaseAudit::auditDirectories({"editor/src", "editor/src/ast"});
        auto ann = SelfHostAnnotationAudit::auditDirectories({"editor/src", "editor/src/ast"});
        auto transpile = SelfHostTranspileAudit::runDefault();
        auto workflow = SelfModernizationWorkflow::createAndRun("editor/src/AnnotationValidator.h");
        return generate(parse, ann, transpile, workflow);
    }

private:
    static void fillGaps(SelfHostingMetrics& out) {
        if (out.parseCoveragePercent < 95.0) {
            out.gaps.push_back("Parse coverage below 95% target");
        }
        if (out.annotationSignalDensity < 1.0) {
            out.gaps.push_back("Low annotation signal density");
        }
        if (out.averageTranspileConfidence < 0.60f) {
            out.gaps.push_back("Low average transpilation confidence");
        }
        if (!out.workflowViable) {
            out.gaps.push_back("Deterministic modernization outputs not fully valid");
        }
        if (out.gaps.empty()) {
            out.gaps.push_back("No blocking gaps detected");
        }
    }
};
