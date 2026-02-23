    void registerOnboardingTools() {
        // whetstone_onboard_workspace
        tools_.push_back({"whetstone_onboard_workspace",
            "Run first-time workspace onboarding in one call: index files, detect key "
            "languages/files, run annotation inference on selected files, save sidecars, "
            "and bootstrap .whetstone metadata.",
            {{"type", "object"}, {"properties", {
                {"root", {{"type", "string"},
                    {"description", "Workspace root override (optional)"}}},
                {"maxFiles", {{"type", "integer"},
                    {"description", "Max key files to process (default 8, max 20)"}}}
            }}}
        });
        toolHandlers_["whetstone_onboard_workspace"] =
            [this](const json& args) {
                return runWorkspaceOnboarding(args);
            };
    }

    void registerWhetstoneTools() {
        registerASTTools();
        registerAnnotationTools();
        registerFileTools();
        registerDiagnosticTools();
        registerBatchTools();
        registerProjectTools();
        registerSaveUndoTools();
        registerSidecarTools();
        registerSemanticAnnotationTools();
        registerEnvironmentTools();
        registerTrainingDataTools();
        registerWorkflowTools();
        registerWorkflowExecutionTools();
        registerRoutingTools();
        registerOrchestratorTools();
        registerReviewTools();
        registerArchitectIntakeTools();
        registerCodegenTools();
        registerModelingTools();
        registerContextTools();
        registerValidationTools();
        registerMetricsTools();
        registerPortingFoundationTools();
        registerRustSemanticTools();
        registerCppRaisingTools();
        registerEquivalenceTools();
        registerPortingGatesTools();
        registerSystemsFamilyTools();
        registerDynamicFamilyTools();
        registerManagedFamilyTools();
        registerASTNativeFamilyTools();
        registerLogicActorFamilyTools();
        registerQueryFamilyTools();
        registerLowLevelFamilyTools();
        registerLegacyIngestionTools();
        registerDebugWorkflowTools();
        registerGovernanceTools();
        registerGraduationTools();
        registerCertificationTools();
        registerFailureTelemetryTools();
        registerAdapterHintsTools();
        registerCostPlanningTools();
        registerUpgradeQueueTools();
        registerRuntimePackTools();
        registerOnboardingTools();
    }
};
