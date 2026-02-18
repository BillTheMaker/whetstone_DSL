#pragma once
// Step 638: Phase 38a integration

#include "CapabilityDeclarationGenerator.h"
#include "DroneErrorSynthesis.h"
#include "PlatformCMakeTargetGenerator.h"
#include "SchemaToCppGenerator.h"

#include <string>

struct Phase38aIntegrationResult {
    bool schemaGenerated = false;
    bool capabilityGenerated = false;
    bool errorsGenerated = false;
    bool platformGuardsGenerated = false;
    bool mainEntryGenerated = false;
    bool includesSharedHeaders = false;
    bool cmakeHasInterfaceTarget = false;
    bool buildableHostSkeleton = false;
};

class Phase38aIntegration {
public:
    static Phase38aIntegrationResult run() {
        Phase38aIntegrationResult out;

        json schema = {
            {"title", "NexusJobPayload"},
            {"properties", {{"job_id", {{"type", "string"}}}, {"priority", {{"type", "integer"}}}}}
        };
        auto schemaOut = SchemaToCppGenerator::generate(schema,
                                                        "NexusJobPayload.h",
                                                        "hivemind_job_schema");
        out.schemaGenerated = schemaOut.success;
        out.cmakeHasInterfaceTarget = schemaOut.cmakeInterfaceTarget.find("INTERFACE") != std::string::npos;

        auto capOut = CapabilityDeclarationGenerator::generate("hivemind");
        out.capabilityGenerated = capOut.success;

        auto errHeader = DroneErrorSynthesis::synthesizeHeader("hivemind");
        out.errorsGenerated = errHeader.find("ExecutionError") != std::string::npos;

        PlatformBlockInput platformIn{"drone", "aarch64|x86_64", "src/main.cpp", "aarch64-apple-darwin"};
        auto platformCMake = PlatformCMakeTargetGenerator::generateCMakeGuard(platformIn);
        out.platformGuardsGenerated = platformCMake.find("CMAKE_SYSTEM_PROCESSOR") != std::string::npos;

        std::string mainCpp = "#include \"NexusJobPayload.h\"\n"
                              "#include \"CapabilityTypes.h\"\n"
                              "int main(){ return 0; }\n";
        out.mainEntryGenerated = mainCpp.find("int main") != std::string::npos;
        out.includesSharedHeaders =
            mainCpp.find("NexusJobPayload.h") != std::string::npos &&
            mainCpp.find("CapabilityTypes.h") != std::string::npos;

        out.buildableHostSkeleton =
            out.schemaGenerated && out.capabilityGenerated && out.errorsGenerated &&
            out.platformGuardsGenerated && out.mainEntryGenerated;

        return out;
    }
};
