// Sprint 58 legacy ingestion MCP tool (Step 816)
// Included inside MCPServer class body.

#include <vector>

#include <nlohmann/json.hpp>

#include "legacy_ingestion/SemanticRecoveryGraph.h"
#include "legacy_ingestion/CrossFileApiInference.h"
#include "legacy_ingestion/AssumptionInference.h"
#include "legacy_ingestion/AmbiguityPacket.h"
#include "legacy_ingestion/ReviewQueueGenerator.h"
#include "legacy_ingestion/ConfidenceCalibration.h"
#include "legacy_ingestion/MigrationReadiness.h"

    void registerLegacyIngestionTools() {
        tools_.push_back({"whetstone_ingest_legacy_to_ir",
            "Process legacy sources to recover semantic intent, surface ambiguities, and score migration readiness.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"}}},
                {"files", {{"type", "array", "items", {"type", "string"}}}},
                {"api", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"source"})}}
        });
        toolHandlers_["whetstone_ingest_legacy_to_ir"] =
            [this](const nlohmann::json& args) { return runIngestLegacyToIr(args); };
    }

    nlohmann::json runIngestLegacyToIr(const nlohmann::json& args) {
        if (!args.contains("source") || !args["source"].is_string()) return {{"success", false}, {"error", "source_missing"}};

        const std::string source = args.value("source", "");
        const std::string api = args.value("api", "core");
        std::vector<std::string> files;
        if (args.contains("files") && args["files"].is_array()) {
            for (const auto& v : args["files"]) if (v.is_string()) files.push_back(v.get<std::string>());
        }

        auto graph = SemanticRecoveryGraph::build(source);
        nlohmann::json graphJson = nlohmann::json::array();
        for (const auto& node : graph) graphJson.push_back(SemanticRecoveryGraph::toJson(node));
        auto apiIntent = CrossFileApiInference::infer(api, files);
        auto assumption = AssumptionInference::analyze(source);
        auto ambiguity = AmbiguityPacketModel::build(source);
        auto reviewQueue = ReviewQueueGenerator::build({ambiguity.id});
        auto calibrated = ConfidenceCalibration::calibrate(apiIntent.confidence);
        auto readiness = MigrationReadiness::evaluate(calibrated, graph.size());

        return {
            {"success", true},
            {"graph", graphJson},
            {"api_intent", CrossFileApiInference::toJson(apiIntent)},
            {"assumption", AssumptionInference::toJson(assumption)},
            {"ambiguity", AmbiguityPacketModel::toJson(ambiguity)},
            {"review_queue", ReviewQueueGenerator::toJson(reviewQueue.front())},
            {"confidence", ConfidenceCalibration::toJson(calibrated)},
            {"readiness", MigrationReadiness::toJson(readiness)}
        };
    }
