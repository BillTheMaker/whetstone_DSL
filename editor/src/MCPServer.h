#pragma once
// Step 207-211: MCP Server for Whetstone
//
// Implements the Model Context Protocol (MCP) over stdio (JSON-RPC 2.0).
// Exposes Whetstone agent API as MCP tools, resources, and prompts so
// LLMs (Claude, Codex, etc.) can interact with the editor natively.
//
// Architecture: MCPServer holds tool/resource/prompt registrations and
// dispatches incoming JSON-RPC requests. A bridge function translates
// MCP tool calls to Whetstone JSON-RPC requests (via a callback).

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <nlohmann/json.hpp>

#include "MarkdownSpecParser.h"
#include "AcceptanceCriteriaBinding.h"
#include "RequirementNormalizationConflictDetector.h"
#include "ScopeMilestoneDecomposer.h"
#include "TaskitemConfidenceAmbiguity.h"
#include "TaskitemGeneratorV2.h"
#include "WorkspaceFileIndex.h"
#include "ContextSliceAssembler.h"
#include "TokenBudgetEnforcer.h"
#include "PrerequisiteOpResolver.h"
#include "SelfContainmentScorer.h"
#include "TaskitemQualityAuditor.h"
#include "AgentSessionRecorder.h"
#include "graduation/PairUpgradeQueue.h"
#include "graduation/UpgradePriorityPolicy.h"
#include "graduation/RuntimeAssumptionExtractor.h"
#include "graduation/RuntimeProfileStore.h"
#include "graduation/MigrationStepSequencer.h"
#include "graduation/MigrationFeasibilityEngine.h"
#include "graduation/MigrationPathCandidate.h"
#include "graduation/MigrationProgressTracker.h"
#include "graduation/CorpusManifestV1.h"
#include "graduation/CorpusIngestionNormalizer.h"
#include "graduation/GoldBenchmarkCurationPipeline.h"
#include "graduation/BenchmarkReplayRunner.h"
#include "graduation/PairBenchmarkScorecard.h"
#include "graduation/CorpusVersionDiff.h"
#include "graduation/BenchmarkPublicationBundle.h"
#include "graduation/TenantPolicyModel.h"
#include "graduation/PolicyInheritanceResolver.h"
#include "graduation/TenantIsolationGuardrails.h"
#include "graduation/TenantWaiverLedger.h"
#include "graduation/CrossTenantAdapterSharingContract.h"
#include "graduation/FederatedReproducibilityValidator.h"
#include "graduation/FederatedGovernanceReport.h"
#include "graduation/ImprovementProposalGenerator.h"
#include "graduation/SandboxTrialRunner.h"
#include "graduation/TrialEvaluationRubric.h"
#include "graduation/PromotionGateForImprovements.h"
#include "graduation/AutoRollbackTrigger.h"
#include "graduation/ImprovementROITracker.h"
#include "graduation/ContinuousImprovementProgramSummary.h"
#include "graduation/WebApiSemanticsPack.h"
#include "graduation/RouteMiddlewareAuthModel.h"
#include "graduation/BackendFrameworkAdapterHooks.h"
#include "graduation/FrontendApiClientContract.h"
#include "graduation/ApiSchemaCompatibilityChecker.h"
#include "graduation/ApiBehaviorRegressionRunner.h"
#include "graduation/WebEcosystemMigrationReport.h"
#include "graduation/DataLineageCanonicalModel.h"
#include "graduation/TransformationIntentPacket.h"
#include "graduation/SchemaEvolutionCompatibilityChecker.h"
#include "graduation/DataSampleReplayRunner.h"
#include "graduation/AggregationWindowSemanticsChecker.h"
#include "graduation/DataQualityGateIntegration.h"
#include "graduation/AnalyticsMigrationDossier.h"
#include "graduation/RealtimeConstraintModel.h"
#include "graduation/InterruptIsrSemanticsPacket.h"
#include "graduation/EmbeddedResourceBudgetModel.h"
#include "graduation/EmbeddedProfileRaisingPolicy.h"
#include "graduation/TimingRegressionRunner.h"
#include "graduation/BinaryFootprintGate.h"
#include "graduation/EmbeddedMigrationRiskReport.h"
#include "graduation/FinancialPrecisionModel.h"
#include "graduation/LedgerInvariantsPacket.h"
#include "graduation/CompliancePolicyBindingEngine.h"
#include "graduation/DeterministicTransactionReplayVerifier.h"
#include "graduation/AuditTrailCompletenessChecker.h"
#include "graduation/ComplianceGateIntegration.h"
#include "graduation/RegulatedDomainMigrationReport.h"
#include "graduation/SimulationLoopCanonicalModel.h"
#include "graduation/DeterminismModePacket.h"
#include "graduation/EventOrderStateSyncVerifier.h"
#include "graduation/FloatingPointDeterminismRiskClassifier.h"
#include "graduation/SimulationFrameTimeGate.h"
#include "graduation/ReplayTraceCompatibilityChecker.h"
#include "graduation/SimulationMigrationDossier.h"
#include "graduation/NumericalStabilityCanonicalModel.h"
#include "graduation/PrecisionRoundoffRiskPacket.h"
#include "graduation/ParallelPartitioningSemanticsModel.h"
#include "graduation/ScientificReproducibilityRunner.h"
#include "graduation/HpcOptimizationProfilePolicies.h"
#include "graduation/NumericalRegressionGate.h"
#include "graduation/HpcMigrationPlaybook.h"
#include "graduation/TranspilationSlaSloSchema.h"
#include "graduation/ReliabilitySignalCollector.h"
#include "graduation/AlertPolicyEngine.h"
#include "graduation/TranspilationIncidentClassifier.h"
#include "graduation/AutoContainmentStrategyHooks.h"
#include "graduation/IncidentRunbookDrillTracker.h"
#include "graduation/ReliabilityOperationsReport.h"
#include "graduation/RolloutStageModel.h"
#include "graduation/ApprovalChainPolicyEngine.h"
#include "graduation/RolloutBlastRadiusEstimator.h"
#include "graduation/FastRollbackOrchestrationContract.h"
#include "graduation/RolloutHealthScoringModel.h"
#include "graduation/AdoptionTelemetryIntegration.h"
#include "graduation/EnterpriseRolloutReport.h"
#include "graduation/BestPracticePackSchema.h"
#include "graduation/PackCurationWorkflowModel.h"
#include "graduation/CrossTenantSharingPolicyValidator.h"
#include "graduation/PracticeEffectivenessRankingEngine.h"
#include "graduation/UnsafePracticeQuarantine.h"
#include "graduation/PackRecommendationEngine.h"
#include "graduation/PracticeAdoptionOutcomeReport.h"
#include "graduation/LtsBaselineCriteriaSchema.h"
#include "graduation/LtsCandidateEvaluator.h"
#include "graduation/SnapshotFreezeIntegrityWorkflow.h"
#include "graduation/DeprecationPolicyModel.h"
#include "graduation/MaintenanceCadenceScheduler.h"
#include "graduation/LtsRiskWatchlistEscalation.h"
#include "graduation/LtsPublicationBundleHandbook.h"
#include "graduation/PluginSdkSchema.h"
#include "graduation/PluginManifestCapabilityModel.h"
#include "graduation/SignedPluginVerificationPipeline.h"
#include "graduation/PluginCompatibilityResolver.h"
#include "graduation/PluginSandboxExecutionPolicy.h"
#include "graduation/PluginCertificationWorkflow.h"
#include "graduation/PluginTrustRiskReport.h"
#include "graduation/ExternalVerifierAdapterInterface.h"
#include "graduation/EvidenceNormalizationSchema.h"
#include "graduation/VerifierTrustWeightingPolicyEngine.h"
#include "graduation/VerifierConflictResolutionModel.h"
#include "graduation/FederatedEvidenceBundleComposer.h"
#include "graduation/VerifierOutageFallbackStrategy.h"
#include "graduation/CrossVerifierConsistencyDashboardModel.h"
#include "graduation/SemanticIRCorePublicSpec.h"
#include "graduation/VerificationEvidenceBundlePublicSpec.h"
#include "graduation/SupportTierMatrixSpecFormat.h"
#include "graduation/SpecVersionNegotiationRules.h"
#include "graduation/ConformanceTestSuiteGenerator.h"
#include "graduation/InternalConformanceRunner.h"
#include "graduation/StandardsPublicationBundle.h"
#include "graduation/SemanticDiffCanonicalModel.h"
#include "graduation/AdapterDecisionTracePacketSchema.h"
#include "graduation/PolicyDecisionExplainabilityRenderer.h"
#include "graduation/UncertaintyAlternativePathExplainer.h"
#include "graduation/ReviewerSemanticDiffSummarizer.h"
#include "graduation/ExplainabilityRetentionPolicyIntegration.h"
#include "graduation/ExplainabilityReportTemplatePack.h"
#include "graduation/SafetyCaseSchema.h"
#include "graduation/EvidenceToClaimMappingEngine.h"
#include "graduation/AssuranceArgumentTreeBuilder.h"
#include "graduation/ResidualRiskOwnershipModel.h"
#include "graduation/SafetyCaseCompletenessChecker.h"
#include "graduation/AssuranceExportFormats.h"
#include "graduation/AssuranceReviewDossierTemplate.h"
#include "graduation/RoleBasedOnboardingModel.h"
#include "graduation/GuidedTranspilationExerciseFramework.h"
#include "graduation/SandboxCurriculumRunnerScoring.h"
#include "graduation/SkillProgressionTrackerIntegration.h"
#include "graduation/OnboardingQualityAnalyticsModel.h"
#include "graduation/CertificationBadgesRoleWorkflow.h"
#include "graduation/TeamOnboardingReadinessReportArtifact.h"
#include "graduation/LocalizationReadyArtifactSchema.h"
#include "graduation/RegionPolicyOverlayModel.h"
#include "graduation/LocalizedReportGenerationPipeline.h"
#include "graduation/PolicyTranslationIntegrityChecker.h"
#include "graduation/CrossRegionWorkflowCompatibilityRules.h"
#include "graduation/RegionalRolloutProfileIntegration.h"
#include "graduation/InternationalOperationsDashboardModel.h"
#include "graduation/EcosystemDriftSignalModel.h"
#include "graduation/PairStabilityForecastingEngine.h"
#include "graduation/MaintenanceDemandEstimator.h"
#include "graduation/PreventiveMitigationPlanner.h"
#include "graduation/ForecastConfidenceUncertaintyModel.h"
#include "graduation/DriftTriggeredSchedulingIntegration.h"
#include "graduation/DriftOutlookReportArtifact.h"
#include "graduation/ProgramKpiSchemaTargetModel.h"
#include "graduation/HistoricalKpiAggregator.h"
#include "graduation/CrossTeamBenchmarkComparator.h"
#include "graduation/SystemicRegressionDetector.h"
#include "graduation/KpiAnomalyTriageModel.h"
#include "graduation/MetaEvaluationRecommendationGenerator.h"
#include "graduation/ProgramBenchmarkingReportBundle.h"
#include "graduation/GovernanceModelRefreshFramework.h"
#include "graduation/LongHorizonRoadmapPlanningEngineV2.h"
#include "graduation/StewardshipOwnershipPolicyModel.h"
#include "graduation/DeprecationSuccessionLifecycleFramework.h"
#include "graduation/ConstitutionalInvariantsComplianceChecker.h"
#include "graduation/GovernanceChangeImpactSimulator.h"
#include "graduation/ProgramConstitutionHandbookPublicationBundle.h"
#include "graduation/MarketplaceCatalogSchema.h"
#include "graduation/ArtifactTrustTierModel.h"
#include "graduation/ArtifactQualityScoringRankingEngine.h"
#include "graduation/MarketplaceSupplyChainIntegrityChecks.h"
#include "graduation/ArtifactDelistingQuarantineWorkflow.h"
#include "graduation/MarketplacePolicyBindingsForTenants.h"
#include "graduation/MarketplaceGovernanceReportArtifact.h"
#include "graduation/InteropTestbedManifestModel.h"
#include "graduation/ReferenceImplementationHarnessForIREvidenceSpecs.h"
#include "graduation/ThirdPartyConformanceReplayRunner.h"
#include "graduation/CanonicalFixtureAndOracleDatasetPacks.h"
#include "graduation/DivergenceTriageModelForInteropFailures.h"
#include "graduation/InteropCertificationPolicyBindings.h"
#include "graduation/InteropPublicationBundle.h"
#include "TaskCompletionMetrics.h"
#include "ABTestComparison.h"
#include "LanguageCapabilityMatrix.h"
#include "LanguageSupportTier.h"
#include "MigrationAcceptanceContract.h"
#include "rust_ir/RustOwnershipBorrowExtractor.h"
#include "rust_ir/RustLifetimeRegionLowering.h"
#include "rust_ir/RustTraitImplLowering.h"
#include "rust_ir/RustGenericIntentModel.h"
#include "rust_ir/RustErrorModelLowering.h"
#include "rust_ir/RustAsyncIntentLowering.h"
#include "rust_ir/RustMacroBoundaryPolicy.h"
#include "rust_ir/RustUnsafeRiskPacket.h"
#include "cpp_ir/CppOwnershipMappingPolicy.h"
#include "cpp_ir/CppBorrowMappingStrategy.h"
#include "cpp_ir/CppTraitRaising.h"
#include "cpp_ir/CppTemplateRaisingPolicy.h"
#include "cpp_ir/CppErrorModelMapping.h"
#include "cpp_ir/CppAsyncMappingStrategy.h"
#include "cpp_ir/CppAlgorithmLifting.h"
#include "cpp_ir/CppBuildArtifactGenerator.h"
#include "debug/FailurePacket.h"
#include "debug/FailureClusterer.h"
#include "debug/FixContextAssembler.h"
#include "debug/PatchProposal.h"
#include "debug/FailurePatchProposer.h"
#include "debug/ReproPacketStore.h"
#include "debug/RegressionGuardPlanner.h"
#include "debug/DebugLoopOrchestrator.h"
#include "debug/FailureFixtureCatalog.h"
#include "debug/PatchDryRunApplier.h"
#include "debug/PatchInvariantChecker.h"
#include "debug/DebugMetricsPacket.h"
#include "debug/FailureTrendTracker.h"
#include "debug/PatchCandidateRanker.h"
#include "debug/ReproCommandRunner.h"
#include "debug/DebugBenchmarkHarness.h"
#include "debug/SLMDebugReadiness.h"
#include "debug/PatchExecutionRecord.h"
#include "debug/RollbackLedger.h"
#include "debug/BisectCandidateSelector.h"
#include "debug/PatchExecutionAuditBundle.h"
#include "debug/DebugCampaignSpec.h"
#include "debug/CampaignTargetPrioritizer.h"
#include "debug/SafetyEnvelopePolicy.h"
#include "debug/CampaignProgressTracker.h"
#include "debug/CampaignReportBundle.h"
#include "debug/CampaignCheckpoint.h"
#include "debug/CampaignCheckpointStore.h"
#include "debug/CampaignQueuePolicy.h"
#include "debug/CampaignResumePlanner.h"
#include "debug/CampaignExportBundle.h"
#include "debug/DebugHintTemplate.h"
#include "debug/FailureTriageScore.h"
#include "debug/MinimalReproReducer.h"
#include "debug/PatchRiskLabeler.h"
#include "debug/DebugAssistPacket.h"
#include "debug/DebugRecipeStep.h"
#include "debug/DebugRecipeLibrary.h"
#include "debug/DebugActionValidator.h"
#include "debug/DebugExecutionTrace.h"
#include "debug/DebugRecipePacket.h"
#include "debug/DebugPolicyConstraint.h"
#include "debug/DebugActionBudget.h"
#include "debug/DebugRecoveryAdvisor.h"
#include "debug/DebugStopReasonClassifier.h"
#include "debug/DebugGuidancePacket.h"
#include "debug/DebugChecklistItem.h"
#include "debug/DebugChecklistTemplate.h"
#include "debug/DebugSessionHandoff.h"
#include "debug/DebugEvidenceIndex.h"
#include "debug/DebugHandoffPacket.h"
#include "equiv/DifferentialExecutionHarness.h"
#include "equiv/TestVectorSpec.h"
#include "equiv/RustRunnerAdapter.h"
#include "equiv/CppRunnerAdapter.h"
#include "equiv/PropertyEquivalenceRunner.h"
#include "equiv/FuzzDifferentialRunner.h"
#include "equiv/BehavioralDivergencePacket.h"
#include "equiv/EquivalenceEvidenceBundle.h"
#include "gates/SecurityScanOrchestrator.h"
#include "gates/SanitizerGateIntegration.h"
#include "gates/SupplyChainAuditPacket.h"
#include "gates/PerfBenchmarkContract.h"
#include "gates/PerfComparator.h"
#include "gates/GateSeverityPolicy.h"
#include "gates/CppReviewReportTemplate.h"
#include "systems/CAdapterV1.h"
#include "systems/GoAdapterV1.h"
#include "systems/JavaAdapterV1.h"
#include "systems/CRaisingAdapterV1.h"
#include "systems/GoRaisingAdapterV1.h"
#include "systems/JavaRaisingAdapterV1.h"
#include "systems/SystemsCompatibilityMatrix.h"
#include "systems/SystemsFamilyAcceptanceReport.h"
#include "dynamic/PythonAdapterV2.h"
#include "dynamic/JavaScriptAdapterV2.h"
#include "dynamic/TypeScriptAdapterV2.h"
#include "dynamic/RubyAdapterV1.h"
#include "dynamic/LuaAdapterV1.h"
#include "dynamic/DynamicStrictnessPolicy.h"
#include "dynamic/DynamicRiskClassifier.h"
#include "dynamic/DynamicFamilyAcceptanceReport.h"
#include "managed/ManagedPacketTypes.h"
#include "managed/KotlinAdapterV1.h"
#include "managed/CSharpAdapterV1.h"
#include "managed/FSharpAdapterV1.h"
#include "managed/VbNetAdapterV1.h"
#include "managed/NullabilityOptionalityBridge.h"
#include "managed/AsyncModelBridge.h"
#include "managed/ADTPatternCanonicalLowering.h"
#include "managed/ManagedFamilyPromotion.h"
#include "ast_native/SExpressionCanonicalLowering.h"
#include "ast_native/LispAdapterV1.h"
#include "ast_native/SchemeAdapterV1.h"
#include "ast_native/ElispAdapterV1.h"
#include "ast_native/SmalltalkAdapterV1.h"
#include "ast_native/MacroHygieneBoundary.h"
#include "ast_native/EvalRuntimeRiskModel.h"
#include "ast_native/ASTNativeProjectionBenchmark.h"
#include "logic_actor/LogicActorPacketTypes.h"
#include "logic_actor/PrologAdapterV1.h"
#include "logic_actor/ErlangAdapterV1.h"
#include "logic_actor/ElixirAdapterV1.h"
#include "logic_actor/LogicImperativeProjectionPolicy.h"
#include "logic_actor/ActorAsyncProjectionPolicy.h"
#include "logic_actor/SupervisionTreePacket.h"
#include "logic_actor/SemanticBlocklistGate.h"
#include "logic_actor/LogicActorAcceptanceReport.h"
#include "data_query/SqlCanonicalQueryIR.h"
#include "data_query/PostgreSqlAdapterV1.h"
#include "data_query/TSqlAdapterV1.h"
#include "data_query/MySqlAdapterV1.h"
#include "data_query/TransactionIsolationPacket.h"
#include "data_query/QueryBehaviorEquivalenceRunner.h"
#include "data_query/QueryDivergenceClassifier.h"
#include "data_query/DataFamilyAcceptanceReport.h"

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  MCP Tool definition
// -----------------------------------------------------------------------
struct MCPTool {
    std::string name;
    std::string description;
    json inputSchema;  // JSON Schema for tool input
};

// -----------------------------------------------------------------------
//  MCP Resource definition
// -----------------------------------------------------------------------
struct MCPResource {
    std::string uri;
    std::string name;
    std::string description;
    std::string mimeType;
};

// -----------------------------------------------------------------------
//  MCP Prompt definition
// -----------------------------------------------------------------------
struct MCPPromptArgument {
    std::string name;
    std::string description;
    bool required = false;
};

struct MCPPrompt {
    std::string name;
    std::string description;
    std::vector<MCPPromptArgument> arguments;
};

struct MCPPromptMessage {
    std::string role;  // "user" or "assistant"
    json content;      // {type: "text", text: "..."}
};

// -----------------------------------------------------------------------
//  MCP Server
// -----------------------------------------------------------------------
class MCPServer {
public:
    // Callback type: given a Whetstone JSON-RPC request, returns a response.
    using RpcCallback = std::function<json(const json& request)>;

    MCPServer() {
        registerWhetstoneTools();
        registerWhetstoneResources();
        registerWhetstonePrompts();
    }

    void setRpcCallback(RpcCallback cb) { rpcCallback_ = std::move(cb); }

    // ---------------------------------------------------------------
    //  Handle a single MCP JSON-RPC request
    // ---------------------------------------------------------------
    json handleRequest(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        std::string method = request.value("method", "");

        if (method == "initialize") return handleInitialize(request);
        if (method == "notifications/initialized") return json(nullptr); // no response
        if (method == "ping") return handlePing(request);
        if (method == "tools/list") return handleToolsList(request);
        if (method == "tools/call") return handleToolsCall(request);
        if (method == "resources/list") return handleResourcesList(request);
        if (method == "resources/read") return handleResourcesRead(request);
        if (method == "prompts/list") return handlePromptsList(request);
        if (method == "prompts/get") return handlePromptsGet(request);

        response["error"] = {{"code", -32601}, {"message", "Method not found: " + method}};
        return response;
    }

    // ---------------------------------------------------------------
    //  Accessors for testing
    // ---------------------------------------------------------------
    const std::vector<MCPTool>& getTools() const { return tools_; }
    const std::vector<MCPResource>& getResources() const { return resources_; }
    const std::vector<MCPPrompt>& getPrompts() const { return prompts_; }
    bool isInitialized() const { return initialized_; }

    // Resource read handler registration
    using ResourceReader = std::function<json(const std::string& uri)>;
    void setResourceReader(ResourceReader reader) { resourceReader_ = std::move(reader); }

private:
    std::vector<MCPTool> tools_;
    std::vector<MCPResource> resources_;
    std::vector<MCPPrompt> prompts_;
    std::map<std::string, std::function<json(const json&)>> toolHandlers_;
    RpcCallback rpcCallback_;
    ResourceReader resourceReader_;
    bool initialized_ = false;
    AgentSessionRecorder sessionRecorder_;
    bool recordingActive_ = false;
    std::string recordingSessionId_;
    PairUpgradeQueue upgradeQueue_;
    RuntimeProfileStore runtimeProfileStore_;
    MigrationProgressTracker migrationTracker_;
    std::map<std::string, PairBenchmarkScorecard> benchmarkScorecards_;
    std::map<std::string, TenantPolicyModel> tenantPolicies_;
    std::map<std::string, ImprovementProposal> improvementProposals_;
    std::map<std::string, SandboxTrialResult> improvementTrials_;
    std::map<std::string, WebApiSemanticsPack> apiSemanticsPacks_;
    std::map<std::string, DataLineageCanonicalModel> dataLineageModels_;
    std::map<std::string, RealtimeConstraintModel> realtimeConstraints_;

    // ---------------------------------------------------------------
    //  Protocol handlers
    // ---------------------------------------------------------------

    json handleInitialize(const json& request) {
        initialized_ = true;
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        response["result"] = {
            {"protocolVersion", "2024-11-05"},
            {"capabilities", {
                {"tools", json::object()},
                {"resources", json::object()},
                {"prompts", json::object()}
            }},
            {"serverInfo", {
                {"name", "whetstone-mcp"},
                {"version", "0.1.0"}
            }},
            {"instructions", loadInitializeInstructions()}
        };
        return response;
    }

    json handlePing(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        response["result"] = json::object();
        return response;
    }

    json handleToolsList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json toolArr = json::array();
        for (const auto& t : tools_) {
            toolArr.push_back({
                {"name", t.name},
                {"description", t.description},
                {"inputSchema", t.inputSchema}
            });
        }
        response["result"] = {{"tools", toolArr}};
        return response;
    }

    json handleToolsCall(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string toolName = params.value("name", "");
        json args = params.contains("arguments") ? params["arguments"] : json::object();

        auto it = toolHandlers_.find(toolName);
        if (it == toolHandlers_.end()) {
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", "Unknown tool: " + toolName}}})},
                {"isError", true}
            };
            return response;
        }

        try {
            const auto t0 = std::chrono::steady_clock::now();
            json result = it->second(args);
            const auto t1 = std::chrono::steady_clock::now();
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            if (recordingActive_ &&
                toolName != "whetstone_start_recording" &&
                toolName != "whetstone_get_metrics") {
                auto rec = AgentSessionRecorder::makeRecord(
                    toolName,
                    args.dump(),
                    result.dump(),
                    ms);
                sessionRecorder_.record(rec);
            }
            std::string text = result.dump(2);
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", text}}})},
                {"isError", false}
            };
        } catch (const std::exception& e) {
            response["result"] = {
                {"content", json::array({{{"type", "text"}, {"text", std::string("Error: ") + e.what()}}})},
                {"isError", true}
            };
        }
        return response;
    }

    json handleResourcesList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json arr = json::array();
        for (const auto& r : resources_) {
            arr.push_back({
                {"uri", r.uri},
                {"name", r.name},
                {"description", r.description},
                {"mimeType", r.mimeType}
            });
        }
        response["result"] = {{"resources", arr}};
        return response;
    }

    json handleResourcesRead(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string uri = params.value("uri", "");

        if (resourceReader_) {
            json content = resourceReader_(uri);
            response["result"] = {
                {"contents", json::array({{
                    {"uri", uri},
                    {"mimeType", "application/json"},
                    {"text", content.dump()}
                }})}
            };
        } else {
            response["error"] = {{"code", -32002}, {"message", "No resource reader configured"}};
        }
        return response;
    }

    json handlePromptsList(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);
        json arr = json::array();
        for (const auto& p : prompts_) {
            json argArr = json::array();
            for (const auto& a : p.arguments) {
                argArr.push_back({
                    {"name", a.name},
                    {"description", a.description},
                    {"required", a.required}
                });
            }
            arr.push_back({
                {"name", p.name},
                {"description", p.description},
                {"arguments", argArr}
            });
        }
        response["result"] = {{"prompts", arr}};
        return response;
    }

    json handlePromptsGet(const json& request) {
        json response;
        response["jsonrpc"] = "2.0";
        response["id"] = request.contains("id") ? request["id"] : json(nullptr);

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string name = params.value("name", "");
        json args = params.contains("arguments") ? params["arguments"] : json::object();

        auto messages = generatePromptMessages(name, args);
        if (messages.empty()) {
            response["error"] = {{"code", -32602}, {"message", "Unknown prompt: " + name}};
            return response;
        }

        json msgArr = json::array();
        for (const auto& m : messages) {
            msgArr.push_back({{"role", m.role}, {"content", m.content}});
        }
        response["result"] = {{"messages", msgArr}};
        return response;
    }

    // ---------------------------------------------------------------
    //  Bridge: call Whetstone JSON-RPC via callback
    // ---------------------------------------------------------------
    json callWhetstone(const std::string& method, const json& params = json::object()) {
        if (!rpcCallback_) return {{"error", "No RPC callback configured"}};
        json request = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", method},
            {"params", params}
        };
        json resp = rpcCallback_(request);
        if (resp.contains("result")) return resp["result"];
        if (resp.contains("error")) return {{"error", resp["error"]["message"]}};
        return resp;
    }

    static bool hasCallError(const json& value) {
        return value.is_object() && value.contains("error");
    }

    static void appendWarning(json& warnings, const std::string& message) {
        warnings.push_back(message);
    }

    static std::string toLowerCopy(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    static std::string languageForPath(const std::string& path) {
        auto dot = path.find_last_of('.');
        if (dot == std::string::npos) return "";
        std::string ext = toLowerCopy(path.substr(dot));
        if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" ||
            ext == ".hpp" || ext == ".h" || ext == ".hh" ||
            ext == ".hxx") return "cpp";
        if (ext == ".py") return "python";
        if (ext == ".js" || ext == ".mjs" || ext == ".cjs") return "javascript";
        if (ext == ".ts" || ext == ".tsx") return "typescript";
        if (ext == ".java") return "java";
        if (ext == ".rs") return "rust";
        if (ext == ".go") return "go";
        return "";
    }

    static std::string readFileText(const std::string& path) {
        std::ifstream input(path);
        if (!input.is_open()) return "";
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

    static std::string loadInitializeInstructions() {
        const char* envPath = std::getenv("WHETSTONE_SYSTEM_PROMPT_PATH");
        if (envPath != nullptr) {
            std::string envText = readFileText(envPath);
            if (!envText.empty()) return envText;
        }

        const std::vector<std::string> candidates = {
            "tools/claude/system_prompt.txt",
            "../tools/claude/system_prompt.txt",
            "../../tools/claude/system_prompt.txt"
        };
        for (const auto& path : candidates) {
            std::string text = readFileText(path);
            if (!text.empty()) return text;
        }
        return "System prompt unavailable: tools/claude/system_prompt.txt not found.";
    }

    static bool isIgnoredOnboardPath(const std::string& path) {
        std::string lower = toLowerCopy(path);
        auto hasSegment = [&](const std::string& segment) {
            return lower.find(segment) == 0 ||
                   lower.find("/" + segment) != std::string::npos;
        };
        return hasSegment(".git/") ||
               hasSegment("build/") ||
               hasSegment("build-native/") ||
               hasSegment("node_modules/") ||
               hasSegment(".whetstone/");
    }

    static int filePriority(const std::string& path) {
        std::string lower = toLowerCopy(path);
        if (lower.find("main.") != std::string::npos ||
            lower.find("app.") != std::string::npos ||
            lower.find("index.") != std::string::npos ||
            lower.find("lib.") != std::string::npos) return 0;
        if (lower.find("/src/") != std::string::npos) return 1;
        return 2;
    }

    static json selectOnboardingCandidates(const json& files, int maxFiles) {
        std::vector<std::pair<int, json>> ranked;
        std::map<std::string, int> languageCounts;
        json candidates = json::array();
        for (const auto& f : files) {
            if (!f.is_object()) continue;
            if (f.value("isDir", false)) continue;
            std::string path = f.value("path", "");
            if (path.empty() || isIgnoredOnboardPath(path)) continue;
            std::string language = languageForPath(path);
            if (language.empty()) continue;
            languageCounts[language] += 1;
            ranked.push_back({filePriority(path), {
                {"path", path},
                {"language", language}
            }});
        }
        std::sort(ranked.begin(), ranked.end(),
                  [](const auto& a, const auto& b) {
                      if (a.first != b.first) return a.first < b.first;
                      return a.second.value("path", "") < b.second.value("path", "");
                  });
        for (const auto& item : ranked) {
            if ((int)candidates.size() >= maxFiles) break;
            candidates.push_back(item.second);
        }
        json langJson = json::object();
        for (const auto& [lang, count] : languageCounts) langJson[lang] = count;
        return {
            {"candidates", candidates},
            {"languageCounts", langJson}
        };
    }

    json runWorkspaceOnboarding(const json& args) {
        json warnings = json::array();
        int maxFiles = args.value("maxFiles", 8);
        if (maxFiles < 1) maxFiles = 1;
        if (maxFiles > 20) maxFiles = 20;

        json indexParams = json::object();
        std::string root = args.value("root", "");
        if (!root.empty()) indexParams["root"] = root;
        json indexRes = callWhetstone("indexWorkspace", indexParams);
        if (hasCallError(indexRes)) {
            return {
                {"success", false},
                {"error", indexRes["error"]},
                {"warnings", warnings}
            };
        }

        json listRes = callWhetstone("workspaceList", json::object());
        json files = listRes.value("files", json::array());
        json selection = selectOnboardingCandidates(files, maxFiles);
        json candidates = selection["candidates"];
        json processed = json::array();
        int inferredCount = 0;
        int savedCount = 0;

        for (const auto& c : candidates) {
            std::string path = c.value("path", "");
            std::string language = c.value("language", "");

            json openRes = callWhetstone("openFile", {
                {"path", path},
                {"language", language}
            });
            if (hasCallError(openRes)) {
                appendWarning(warnings, "openFile failed for " + path);
                continue;
            }

            json inferRes = callWhetstone("inferAnnotations", json::object());
            if (hasCallError(inferRes)) {
                appendWarning(warnings, "inferAnnotations failed for " + path);
                continue;
            }
            inferredCount += inferRes.value("count", 0);

            std::string openedPath = openRes.value("path", path);
            json saveRes = callWhetstone("saveAnnotatedAST", {{"path", openedPath}});
            if (hasCallError(saveRes)) {
                appendWarning(warnings, "saveAnnotatedAST failed for " + openedPath);
            } else {
                savedCount += 1;
            }

            processed.push_back({
                {"path", openedPath},
                {"language", language},
                {"suggestionCount", inferRes.value("count", 0)},
                {"saved", !hasCallError(saveRes)}
            });
        }

        std::string readme =
            "# Whetstone Workspace\n"
            "This directory stores onboarding metadata and sidecar annotation artifacts.\n";
        json mkDirRes = callWhetstone("fileCreate", {
            {"path", ".whetstone/README.md"},
            {"content", readme}
        });
        if (hasCallError(mkDirRes))
            appendWarning(warnings, "Unable to create .whetstone/README.md");

        return {
            {"success", true},
            {"workspace", indexRes.value("root", root)},
            {"fileCount", indexRes.value("fileCount", 0)},
            {"selectedFileCount", (int)candidates.size()},
            {"processedFileCount", (int)processed.size()},
            {"inferredAnnotationCount", inferredCount},
            {"savedSidecarCount", savedCount},
            {"languageCounts", selection["languageCounts"]},
            {"processedFiles", processed},
            {"workflowSuggestion", {
                {"nextTools", json::array({
                    "whetstone_create_skeleton",
                    "whetstone_create_workflow",
                    "whetstone_orchestrate_run_deterministic",
                    "whetstone_get_blockers",
                    "whetstone_get_review_queue"
                })},
                {"note", "Onboarding complete. Use skeleton/workflow tools to start routed execution."}
            }},
            {"warnings", warnings}
        };
    }

    // ---------------------------------------------------------------
    //  Step 208: Register AST query and mutation tools
    // ---------------------------------------------------------------
#include "mcp/RegisterASTTools.h"
#include "mcp/RegisterAnnotationTools.h"
#include "mcp/RegisterResources.h"
#include "mcp/RegisterPrompts.h"
#include "mcp/RegisterFileTools.h"
#include "mcp/RegisterDiagnosticTools.h"
#include "mcp/RegisterBatchTools.h"
#include "mcp/RegisterProjectTools.h"
#include "mcp/RegisterSaveUndoTools.h"
#include "mcp/RegisterSidecarTools.h"
#include "mcp/RegisterSemanticAnnotationTools.h"
#include "mcp/RegisterEnvironmentTools.h"
#include "mcp/RegisterTrainingDataTools.h"
#include "mcp/RegisterWorkflowTools.h"
#include "mcp/RegisterWorkflowExecutionTools.h"
#include "mcp/RegisterRoutingTools.h"
#include "mcp/RegisterOrchestratorTools.h"
#include "mcp/RegisterReviewTools.h"
#include "mcp/RegisterArchitectIntakeTools.h"
#include "mcp/RegisterCodegenTools.h"
#include "mcp/RegisterModelingTools.h"
#include "mcp/RegisterContextTools.h"
#include "mcp/RegisterValidationTools.h"
#include "mcp/RegisterMetricsTools.h"
#include "mcp/RegisterPortingFoundationTools.h"
#include "mcp/RegisterRustSemanticTools.h"
#include "mcp/RegisterCppRaisingTools.h"
#include "mcp/RegisterEquivalenceTools.h"
#include "mcp/RegisterPortingGatesTools.h"
#include "mcp/RegisterSystemsFamilyTools.h"
#include "mcp/RegisterDynamicFamilyTools.h"
#include "mcp/RegisterManagedFamilyTools.h"
#include "mcp/RegisterASTNativeFamilyTools.h"
#include "mcp/RegisterLogicActorFamilyTools.h"
#include "mcp/RegisterQueryFamilyTools.h"
#include "mcp/RegisterLowLevelFamilyTools.h"
#include "mcp/RegisterLegacyIngestionTools.h"
#include "mcp/RegisterDebugWorkflowTools.h"
#include "governance/ReviewerDecisionLedger.h"
#include "mcp/RegisterGovernanceTools.h"
#include "mcp/RegisterGraduationTools.h"
#include "mcp/RegisterCertificationTools.h"
#include "mcp/RegisterFailureTelemetryTools.h"
#include "graduation/HintModelInterface.h"
#include "mcp/RegisterAdapterHintsTools.h"
#include "graduation/PortingCostModel.h"
#include "graduation/MultiPlanAlternativeGenerator.h"
#include "graduation/BudgetPolicyEnforcer.h"
#include "mcp/RegisterCostPlanningTools.h"
#include "mcp/RegisterUpgradeQueueTools.h"
#include "mcp/RegisterRuntimePackTools.h"
#include "mcp/RegisterMigrationPlanningTools.h"
#include "mcp/RegisterBenchmarkTools.h"
#include "mcp/RegisterFederationTools.h"
#include "mcp/RegisterImprovementLoopTools.h"
#include "mcp/RegisterApiSemanticsTools.h"
#include "mcp/RegisterDataPipelineTools.h"
#include "mcp/RegisterEmbeddedTools.h"
#include "mcp/RegisterFinancialComplianceTools.h"
#include "mcp/RegisterScientificTools.h"
#include "mcp/RegisterSimulationTools.h"
#include "mcp/RegisterReliabilityOpsTools.h"
#include "mcp/RegisterRolloutOpsTools.h"
#include "mcp/RegisterBestPracticeTools.h"
#include "mcp/RegisterLtsTools.h"
#include "mcp/RegisterPluginEcosystemTools.h"
#include "mcp/RegisterExternalVerifierTools.h"
#include "mcp/RegisterStandardsSpecTools.h"
#include "mcp/RegisterExplainabilityTools.h"
#include "mcp/RegisterSafetyAssuranceTools.h"
#include "mcp/RegisterEducationOnboardingTools.h"
#include "mcp/RegisterInternationalizationTools.h"
#include "mcp/RegisterDriftForecastTools.h"
#include "mcp/RegisterMetaEvaluationTools.h"
#include "mcp/RegisterGovernanceEpochTools.h"
#include "mcp/RegisterMarketplaceTools.h"
#include "mcp/RegisterInteropTools.h"
#include "mcp/RegisterSprint93Tools.h"
#include "mcp/RegisterSprint94Tools.h"
#include "mcp/RegisterSprint95Tools.h"
#include "mcp/RegisterSprint96Tools.h"
#include "mcp/RegisterSprint97Tools.h"
#include "mcp/RegisterSprint98Tools.h"
#include "mcp/RegisterSprint99Tools.h"
#include "mcp/RegisterSprint100Tools.h"
#include "mcp/RegisterSprint101Tools.h"
#include "mcp/RegisterSprint102Tools.h"
#include "mcp/RegisterSprint103Tools.h"
#include "mcp/RegisterSprint104Tools.h"
#include "mcp/RegisterSprint105Tools.h"
#include "mcp/RegisterSprint106Tools.h"
#include "mcp/RegisterSprint107Tools.h"
#include "mcp/RegisterSprint108Tools.h"
#include "mcp/RegisterSprint109Tools.h"
#include "mcp/RegisterSprint110Tools.h"
#include "mcp/RegisterSprint111Tools.h"
#include "mcp/RegisterSprint112Tools.h"
#include "mcp/RegisterSprint113Tools.h"
#include "mcp/RegisterSprint114Tools.h"
#include "mcp/RegisterSprint115Tools.h"
#include "mcp/RegisterSprint116Tools.h"
#include "mcp/RegisterSprint117Tools.h"
#include "mcp/RegisterSprint118Tools.h"
#include "mcp/RegisterSprint119Tools.h"
#include "mcp/RegisterSprint131Tools.h"
#include "mcp/RegisterSprint132Tools.h"
#include "mcp/RegisterSprint133Tools.h"
#include "mcp/RegisterSprint134Tools.h"
#include "mcp/RegisterSprint135Tools.h"
#include "mcp/RegisterSprint136Tools.h"
#include "mcp/RegisterSprint137Tools.h"
#include "mcp/RegisterSprint138Tools.h"
#include "mcp/RegisterSprint139Tools.h"
#include "mcp/RegisterSprint140Tools.h"
#include "mcp/RegisterSprint141Tools.h"
#include "mcp/RegisterSprint142Tools.h"
#include "mcp/RegisterSprint143Tools.h"
#include "mcp/RegisterSprint144Tools.h"
#include "mcp/RegisterSprint145Tools.h"
#include "mcp/RegisterOnboardingAndAllTools.h"
