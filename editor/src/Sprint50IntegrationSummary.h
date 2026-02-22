#pragma once
// Step 738: Sprint 50 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "gates/SecurityScanOrchestrator.h"
#include "gates/SanitizerGateIntegration.h"
#include "gates/SupplyChainAuditPacket.h"
#include "gates/PerfBenchmarkContract.h"
#include "gates/PerfComparator.h"
#include "gates/GateSeverityPolicy.h"
#include "MigrationAcceptanceContract.h"
#include "gates/CppReviewReportTemplate.h"

struct Sprint50IntegrationResult {
    bool securityReady = false;
    bool sanitizerReady = false;
    bool supplyReady = false;
    bool perfContractReady = false;
    bool perfComparatorReady = false;
    bool severityReady = false;
    bool contractWiringReady = false;
    bool reportReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 729;
    int stepEnd = 738;
    std::vector<std::string> filesAdded;
};

class Sprint50IntegrationSummary {
public:
    static Sprint50IntegrationResult run() {
        Sprint50IntegrationResult out;
        out.filesAdded = {
            "gates/SecurityScanOrchestrator.h",
            "gates/SanitizerGateIntegration.h",
            "gates/SupplyChainAuditPacket.h",
            "gates/PerfBenchmarkContract.h",
            "gates/PerfComparator.h",
            "gates/GateSeverityPolicy.h",
            "mcp/RegisterPortingGatesTools.h",
            "MigrationAcceptanceContract.h",
            "gates/CppReviewReportTemplate.h",
            "Sprint50IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto sec = SecurityScanOrchestrator::run({{"F1", "high", "a.cpp"}});
        out.securityReady = sec.highCount == 1;

        auto san = SanitizerGateIntegration::evaluate(SanitizerGateProfile{}, true, true);
        out.sanitizerReady = san.pass;

        auto sup = SupplyChainAuditPacketModel::build({{"dep", 1}});
        out.supplyReady = sup.totalHigh == 1;

        auto perfCases = PerfBenchmarkContract::normalize({{"b", 10.0, 11.0}});
        out.perfContractReady = perfCases.size() == 1;

        auto perf = PerfComparator::compare(perfCases, 20.0);
        out.perfComparatorReady = perf.pass;

        out.severityReady = GateSeverityPolicy::severityFor("security") == "block";

        PortingGateEvidence ev;
        ev.securityHighFindings = 0;
        ev.sanitizerPass = true;
        ev.supplyChainHighFindings = 0;
        ev.worstPerfRegressionPct = 5.0;
        auto gate = MigrationAcceptanceContract::evaluatePortingGates(ev, MigrationGateThresholds{});
        out.contractWiringReady = gate.pass;

        out.reportReady = !CppReviewReportTemplate::renderMarkdown("demo", 0, true, 1.0).empty();

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_run_porting_gates") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.securityReady && out.sanitizerReady && out.supplyReady && out.perfContractReady &&
                      out.perfComparatorReady && out.severityReady && out.contractWiringReady && out.reportReady &&
                      out.mcpToolReady && out.stepStart == 729 && out.stepEnd == 738;
        return out;
    }
};
