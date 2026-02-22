#pragma once
// Step 818: Sprint 58 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "legacy_ingestion/SemanticRecoveryGraph.h"
#include "legacy_ingestion/CrossFileApiInference.h"
#include "legacy_ingestion/AssumptionInference.h"
#include "legacy_ingestion/AmbiguityPacket.h"
#include "legacy_ingestion/ReviewQueueGenerator.h"
#include "legacy_ingestion/ConfidenceCalibration.h"
#include "legacy_ingestion/MigrationReadiness.h"
#include "MCPServer.h"

struct Sprint58IntegrationResult {
    bool graphReady = false;
    bool apiReady = false;
    bool assumptionReady = false;
    bool ambiguityReady = false;
    bool reviewReady = false;
    bool confidenceReady = false;
    bool readinessReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 809;
    int stepEnd = 818;
    std::vector<std::string> filesAdded;
};

class Sprint58IntegrationSummary {
public:
    static Sprint58IntegrationResult run() {
        Sprint58IntegrationResult out;
        out.filesAdded = {
            "legacy_ingestion/SemanticRecoveryGraph.h",
            "legacy_ingestion/CrossFileApiInference.h",
            "legacy_ingestion/AssumptionInference.h",
            "legacy_ingestion/AmbiguityPacket.h",
            "legacy_ingestion/ReviewQueueGenerator.h",
            "legacy_ingestion/ConfidenceCalibration.h",
            "legacy_ingestion/MigrationReadiness.h",
            "mcp/RegisterLegacyIngestionTools.h",
            "Sprint58IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto graph = SemanticRecoveryGraph::build("legacy code");
        auto api = CrossFileApiInference::infer("read", {"a.cpp", "b.cpp"});
        auto assumption = AssumptionInference::analyze("Makefile");
        auto ambiguity = AmbiguityPacketModel::build("TODO");
        auto queue = ReviewQueueGenerator::build({ambiguity.id});
        auto confidence = ConfidenceCalibration::calibrate(api.confidence);
        auto readiness = MigrationReadiness::evaluate(confidence, graph.size());

        out.graphReady = !graph.empty();
        out.apiReady = api.inferredIntent == "write" || api.inferredIntent == "read";
        out.assumptionReady = assumption.category == "build";
        out.ambiguityReady = ambiguity.kind != AmbiguityKind::unknown;
        out.reviewReady = !queue.empty();
        out.confidenceReady = confidence >= 0.5;
        out.readinessReady = readiness.ready || readiness.completeness > 0.5;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_ingest_legacy_to_ir") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.graphReady && out.apiReady && out.assumptionReady && out.ambiguityReady &&
                      out.reviewReady && out.confidenceReady && out.readinessReady && out.mcpToolReady &&
                      out.stepStart == 809 && out.stepEnd == 818;
        return out;
    }
};
