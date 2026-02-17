#pragma once

// Step 498: Phase 25a Integration
// Full self-hosting pipeline: parse -> annotate -> transpile -> workflow -> metrics.

#include "SelfHostAnnotationAudit.h"
#include "SelfHostCodebaseAudit.h"
#include "SelfHostTranspileAudit.h"
#include "SelfHostingMetricsReport.h"
#include "SelfModernizationWorkflow.h"

#include <string>
#include <vector>

struct Phase25aIntegrationResult {
    CodebaseParseAudit parseAudit;
    SelfAnnotationAuditReport annotationAudit;
    SelfTranspileAuditReport transpileAudit;
    ModernizationWorkflowReport workflowAudit;
    SelfHostingMetrics metrics;
    bool parseCoverageGate = false;      // >=95%
    bool transpileGate = false;          // >=3 translated modules
    bool workflowGate = false;           // deterministic modernization viable
    bool phasePass = false;
    std::vector<std::string> notes;
};

class SelfHostingPhase25aIntegration {
public:
    static Phase25aIntegrationResult runCurrentState() {
        Phase25aIntegrationResult out;
        out.parseAudit = SelfHostCodebaseAudit::auditDirectories({"editor/src", "editor/src/ast"});
        out.annotationAudit = SelfHostAnnotationAudit::auditDirectories({"editor/src", "editor/src/ast"});
        out.transpileAudit = SelfHostTranspileAudit::runDefault();
        out.workflowAudit = SelfModernizationWorkflow::createAndRun("editor/src/AnnotationValidator.h");
        out.metrics = SelfHostingMetricsReport::generate(
            out.parseAudit, out.annotationAudit, out.transpileAudit, out.workflowAudit);

        out.parseCoverageGate = out.parseAudit.meetsTarget(0.95);
        out.transpileGate = static_cast<int>(out.transpileAudit.files.size()) >= 3;
        out.workflowGate = out.workflowAudit.deterministicCount == out.workflowAudit.deterministicValidCount;
        out.phasePass = out.parseCoverageGate && out.transpileGate && out.workflowGate;

        out.notes.push_back("Phase 25a pipeline executed");
        out.notes.push_back(out.phasePass ? "Self-hosting thesis gate passed"
                                          : "Self-hosting thesis gate has outstanding gaps");
        return out;
    }
};
