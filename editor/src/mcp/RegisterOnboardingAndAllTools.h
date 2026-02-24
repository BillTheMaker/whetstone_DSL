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
        registerMigrationPlanningTools();
        registerBenchmarkTools();
        registerFederationTools();
        registerImprovementLoopTools();
        registerApiSemanticsTools();
        registerDataPipelineTools();
        registerEmbeddedTools();
        registerFinancialComplianceTools();
        registerScientificTools();
        registerSimulationTools();
        registerReliabilityOpsTools();
        registerRolloutOpsTools();
        registerBestPracticeTools();
        registerLtsTools();
        registerPluginEcosystemTools();
        registerExternalVerifierTools();
        registerStandardsSpecTools();
        registerExplainabilityTools();
        registerSafetyAssuranceTools();
        registerEducationOnboardingTools();
        registerInternationalizationTools();
        registerDriftForecastTools();
        registerMetaEvaluationTools();
        registerGovernanceEpochTools();
        registerMarketplaceTools();
        registerInteropTools();
        registerSprint93Tools();
        registerSprint94Tools();
        registerSprint95Tools();
        registerSprint96Tools();
        registerSprint97Tools();
        registerSprint98Tools();
        registerSprint99Tools();
        registerSprint100Tools();
        registerSprint101Tools();
        registerSprint102Tools();
        registerSprint103Tools();
        registerSprint104Tools();
        registerSprint105Tools();
        registerSprint106Tools();
        registerSprint107Tools();
        registerSprint108Tools();
        registerSprint109Tools();
        registerSprint110Tools();
        registerSprint111Tools();
        registerSprint112Tools();
        registerSprint113Tools();
        registerSprint114Tools();
        registerSprint115Tools();
        registerSprint116Tools();
        registerSprint117Tools();
        registerSprint118Tools();
        registerSprint119Tools();
        registerOnboardingTools();
    }
};
