#pragma once

// Step 492: Phase 24b Integration Pipeline
// Composes security-preserving translation, secure defaults, threat modeling,
// and security test skeleton generation into one structured flow.

#include "SecureByDefaultGenerator.h"
#include "SecurityPreservingTranslation.h"
#include "SecurityTestSkeletonGenerator.h"
#include "ThreatModelIntegration.h"

#include <string>
#include <vector>

struct SecurityPipelineInput {
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string moduleName;
    std::string entityName;
    std::vector<SecuritySourceAnnotation> sourceAnnotations;
    ThreatModelInput threatModel;
};

struct SecurityPipelineResult {
    SecurityTranslationOutput translation;
    SecureGenerationOutput secureSqlSnippet;
    SecureGenerationOutput secureInputSnippet;
    ThreatModelOutput threat;
    SecuritySkeletonOutput tests;
    std::vector<std::string> notes;
};

class SecurityPipelineIntegration {
public:
    static SecurityPipelineResult run(const SecurityPipelineInput& in) {
        SecurityPipelineResult out;
        out.translation = SecurityPreservingTranslation::translate({
            in.sourceLanguage, in.targetLanguage, in.sourceAnnotations
        });

        out.secureSqlSnippet = SecureByDefaultGenerator::generate({
            in.targetLanguage, SecureSnippetKind::SqlQuery,
            "query" + in.entityName, "security-default"
        });
        out.secureInputSnippet = SecureByDefaultGenerator::generate({
            in.targetLanguage, SecureSnippetKind::InputHandling,
            "handle" + in.entityName + "Input", "security-default"
        });

        out.threat = ThreatModelIntegration::analyze(in.threatModel);
        out.tests = SecurityTestSkeletonGenerator::generate({
            in.targetLanguage, in.moduleName, in.entityName, true
        });

        out.notes.push_back("Pipeline executed: translation -> secure generation -> threat model -> test skeletons");
        if (out.translation.hasBlockingReview) {
            out.notes.push_back("Translation produced blocking review requirements");
        }
        return out;
    }
};
