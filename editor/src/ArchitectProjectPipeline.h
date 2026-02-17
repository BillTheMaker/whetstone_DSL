#pragma once

// Step 481: Phase 23b Integration Pipeline
// Composes templates, scaffold planning, dependency awareness, and orchestration.

#include "ArchitectMultiLanguageOrchestrator.h"
#include "ArchitectScaffoldGenerator.h"
#include "ArchitectTemplates.h"

#include <string>

struct ArchitectPipelineResult {
    TemplateOutput templated;
    ScaffoldPlan scaffold;
    BuildAwarenessReport awareness;
    MultiLanguageWorkflowPlan workflow;
};

class ArchitectProjectPipeline {
public:
    static ArchitectPipelineResult fromTemplate(ArchitectureTemplateKind kind,
                                                const TemplateInput& input,
                                                const std::string& workspaceRoot) {
        ArchitectPipelineResult out;
        out.templated = ArchitectTemplates::instantiate(kind, input);
        out.scaffold = ArchitectScaffoldGenerator::buildPlan(
            {workspaceRoot, input.projectName, out.templated.skeleton, true});
        out.awareness = ArchitectBuildAwareness::annotate(out.templated.skeleton);
        out.workflow = ArchitectMultiLanguageOrchestrator::createWorkflow(
            out.templated.skeleton, out.awareness);
        return out;
    }

    static ArchitectPipelineResult fromSkeleton(const SkeletonProjectSpec& skeleton,
                                                const std::string& projectName,
                                                const std::string& workspaceRoot) {
        ArchitectPipelineResult out;
        out.templated.kind = ArchitectureTemplateKind::RestApi;
        out.templated.skeleton = skeleton;
        out.templated.notes.push_back("Generated from provided skeleton");
        out.scaffold = ArchitectScaffoldGenerator::buildPlan(
            {workspaceRoot, projectName, skeleton, true});
        out.awareness = ArchitectBuildAwareness::annotate(skeleton);
        out.workflow = ArchitectMultiLanguageOrchestrator::createWorkflow(
            skeleton, out.awareness);
        return out;
    }

    static SkeletonProjectSpec overrideModuleLanguage(const SkeletonProjectSpec& in,
                                                      const std::string& moduleName,
                                                      const std::string& language) {
        SkeletonProjectSpec out = in;
        for (auto& m : out.modules) {
            if (m.moduleName == moduleName) m.language = language;
        }
        return out;
    }
};
