#pragma once

// Step 485: Worker Dispatch + Result Validation
// Models architect->worker handoff, result validation, retry budget, and escalation.

#include "PriorStepContextInjector.h"
#include "ProjectConventionAnalyzer.h"
#include "StepSpecExpander.h"

#include <sstream>
#include <string>
#include <vector>

struct WorkerSubmission {
    std::vector<SourceFileDraft> files;
    bool buildPassed = false;
    bool testsPassed = false;
    std::string errorOutput;
};

struct WorkerDispatchInput {
    std::string stepMarkdown;
    std::string stepNumber;
    std::string componentName;
    std::string buildSystem = "cmake";
    std::string repoRoot = ".";
    std::vector<std::string> priorHeaders;
};

struct DispatchAttemptReport {
    int attempt = 0;
    bool accepted = false;
    bool buildPassed = false;
    bool testsPassed = false;
    ConventionValidationReport conventionReport;
    std::string status;
};

struct WorkerDispatchResult {
    StepSpec spec;
    StepContext context;
    ProjectConventions conventions;
    std::vector<DispatchAttemptReport> attempts;
    bool completed = false;
    bool escalated = false;
    int retryBudget = 3;
    int attemptsUsed = 0;
    std::string finalStatus;
};

class WorkerDispatchValidator {
public:
    static WorkerDispatchResult run(const WorkerDispatchInput& in,
                                    const std::vector<WorkerSubmission>& submissions,
                                    int retryBudget = 3) {
        WorkerDispatchResult out;
        out.retryBudget = retryBudget <= 0 ? 1 : retryBudget;

        out.spec = StepSpecExpander::expand({
            in.stepMarkdown,
            in.stepNumber,
            in.componentName,
            in.buildSystem,
            {}, // conventions are extracted from repo state
            in.priorHeaders
        });
        out.context = PriorStepContextInjector::build(in.priorHeaders);
        out.conventions = ProjectConventionAnalyzer::extract(in.repoRoot);

        if (submissions.empty()) {
            out.escalated = true;
            out.finalStatus = "No worker submissions provided";
            return out;
        }

        const int maxAttempts = out.retryBudget < (int)submissions.size()
                                ? out.retryBudget : (int)submissions.size();
        for (int i = 0; i < maxAttempts; ++i) {
            DispatchAttemptReport report;
            report.attempt = i + 1;
            report.buildPassed = submissions[i].buildPassed;
            report.testsPassed = submissions[i].testsPassed;
            report.conventionReport =
                ProjectConventionAnalyzer::validate(out.conventions, submissions[i].files);

            const bool conventionsOk = report.conventionReport.violations.empty();
            report.accepted = report.buildPassed && report.testsPassed && conventionsOk;
            report.status = classifyAttemptStatus(report, conventionsOk, submissions[i].errorOutput);
            out.attempts.push_back(report);
            out.attemptsUsed = i + 1;

            if (report.accepted) {
                out.completed = true;
                out.finalStatus = "Step completed";
                return out;
            }
        }

        out.escalated = true;
        out.finalStatus = out.attemptsUsed >= out.retryBudget
                          ? "Retry budget exhausted; escalate to architect"
                          : "All submissions consumed without completion";
        return out;
    }

    static std::string buildWorkerBrief(const WorkerDispatchInput& in,
                                        const StepSpec& spec,
                                        const StepContext& context,
                                        const ProjectConventions& conventions) {
        std::ostringstream out;
        out << "Step " << in.stepNumber << ": " << in.componentName << "\n";
        out << "Complexity: " << complexityLabel(spec.complexity) << "\n";
        out << "Tests planned: " << spec.testPlan.size() << "\n";
        out << "Header target: " << spec.headerSkeleton.fileName << "\n";
        out << "Build target: " << spec.buildEntry.targetName << "\n";
        out << "Conventions: " << conventions.classNaming << ", "
            << conventions.methodNaming << ", " << conventions.fileNaming << "\n";
        out << "Context brief: " << context.condensedBrief() << "\n";
        return out.str();
    }

private:
    static std::string classifyAttemptStatus(const DispatchAttemptReport& report,
                                             bool conventionsOk,
                                             const std::string& errorOutput) {
        if (!report.buildPassed) {
            return "build_failed" + (errorOutput.empty() ? "" : ": " + errorOutput);
        }
        if (!report.testsPassed) {
            return "tests_failed" + (errorOutput.empty() ? "" : ": " + errorOutput);
        }
        if (!conventionsOk) return "convention_warnings";
        return "accepted";
    }

    static std::string complexityLabel(StepComplexity c) {
        switch (c) {
            case StepComplexity::Trivial: return "trivial";
            case StepComplexity::Standard: return "standard";
            case StepComplexity::Complex: return "complex";
            case StepComplexity::Pipeline: return "pipeline";
        }
        return "standard";
    }
};
