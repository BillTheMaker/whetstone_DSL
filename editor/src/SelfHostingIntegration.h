#pragma once

// Step 487: Phase 23c Integration
// Validates full self-hosting data flow without invoking a real worker agent.

#include "PriorStepContextInjector.h"
#include "ProjectConventionAnalyzer.h"
#include "StepSpecExpander.h"
#include "TestPlanGenerator.h"

#include <string>
#include <vector>

struct SelfHostingInput {
    std::string repoRoot = ".";
    std::string stepMarkdown;
    std::string stepNumber;
    std::string componentName;
    std::vector<std::string> priorHeaders;
    std::vector<SourceFileDraft> knownGoodFiles;
    std::vector<SourceFileDraft> knownBrokenFiles;
};

struct SelfHostingResult {
    ProjectConventions conventions;
    StepContext context;
    StepSpec spec;
    GeneratedTestPlan testPlan;
    ConventionValidationReport goodReport;
    ConventionValidationReport brokenReport;
    bool workerReady = false;
    std::vector<std::string> notes;
};

class SelfHostingIntegration {
public:
    static SelfHostingResult run(const SelfHostingInput& in) {
        SelfHostingResult out;
        out.conventions = ProjectConventionAnalyzer::extract(in.repoRoot);
        out.context = PriorStepContextInjector::build(in.priorHeaders);
        out.spec = StepSpecExpander::expand({
            in.stepMarkdown,
            in.stepNumber,
            in.componentName,
            "cmake",
            {},
            in.priorHeaders
        });
        out.testPlan = TestPlanGenerator::generate({
            in.stepMarkdown,
            in.stepNumber,
            in.componentName,
            {},
            StepComplexity::Standard,
            false
        });

        out.goodReport = ProjectConventionAnalyzer::validate(out.conventions, in.knownGoodFiles);
        out.brokenReport = ProjectConventionAnalyzer::validate(out.conventions, in.knownBrokenFiles);

        out.workerReady = isWorkerReady(out);
        out.notes.push_back("Flow executed: conventions -> context -> spec -> test plan");
        out.notes.push_back(out.workerReady
                            ? "Worker brief sufficiency checks passed"
                            : "Worker brief sufficiency checks failed");
        return out;
    }

private:
    static bool isWorkerReady(const SelfHostingResult& r) {
        const bool hasSpec = !r.spec.testPlan.empty() &&
                             !r.spec.headerSkeleton.methods.empty() &&
                             !r.spec.buildEntry.targetName.empty();
        const bool hasContext = !r.context.priorApis.empty();
        const bool hasConventions = r.conventions.sizeLimits.count("header") &&
                                    r.conventions.sizeLimits.count("main.cpp");
        const bool hasPlan = !r.testPlan.tests.empty();
        return hasSpec && hasContext && hasConventions && hasPlan;
    }
};
