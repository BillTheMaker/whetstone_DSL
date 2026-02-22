#pragma once
// Step 798: Sprint 56 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "data_query/SqlCanonicalQueryIR.h"
#include "data_query/PostgreSqlAdapterV1.h"
#include "data_query/TSqlAdapterV1.h"
#include "data_query/MySqlAdapterV1.h"
#include "data_query/TransactionIsolationPacket.h"
#include "data_query/QueryBehaviorEquivalenceRunner.h"
#include "data_query/QueryDivergenceClassifier.h"
#include "data_query/DataFamilyAcceptanceReport.h"

struct Sprint56IntegrationResult {
    bool canonicalReady = false;
    bool postgresReady = false;
    bool tsqlReady = false;
    bool mysqlReady = false;
    bool transactionReady = false;
    bool equivalenceReady = false;
    bool divergenceReady = false;
    bool reportReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 789;
    int stepEnd = 798;
    std::vector<std::string> filesAdded;
};

class Sprint56IntegrationSummary {
public:
    static Sprint56IntegrationResult run() {
        Sprint56IntegrationResult out;
        out.filesAdded = {
            "data_query/SqlCanonicalQueryIR.h",
            "data_query/PostgreSqlAdapterV1.h",
            "data_query/TSqlAdapterV1.h",
            "data_query/MySqlAdapterV1.h",
            "data_query/TransactionIsolationPacket.h",
            "data_query/QueryBehaviorEquivalenceRunner.h",
            "data_query/QueryDivergenceClassifier.h",
            "data_query/DataFamilyAcceptanceReport.h",
            "mcp/RegisterQueryFamilyTools.h",
            "Sprint56IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto c = SqlCanonicalQueryIR::lower("SELECT * FROM t", "postgresql");
        auto pg = PostgreSqlAdapterV1::lower("SELECT * FROM t");
        auto ts = TSqlAdapterV1::lower("SELECT * FROM t");
        auto my = MySqlAdapterV1::lower("SELECT * FROM t");

        out.canonicalReady = c.irSummary == "sql_query_ir_v1";
        out.postgresReady = pg.sourceDialect == "postgresql";
        out.tsqlReady = ts.sourceDialect == "tsql";
        out.mysqlReady = my.sourceDialect == "mysql";

        auto tx = TransactionIsolationModel::analyze("BEGIN TRANSACTION ISOLATION LEVEL SERIALIZABLE", "read_committed");
        out.transactionReady = tx.riskOnDowngrade;

        auto eq = QueryBehaviorEquivalenceRunner::compare(nlohmann::json::array({{{"id",1}}}), nlohmann::json::array({{{"id",1}}}));
        out.equivalenceReady = eq.equivalent;

        auto div = QueryDivergenceClassifier::classify(SqlCanonicalQueryIR::lower("SELECT COUNT(*) FROM a JOIN b ON a.id=b.id", "postgresql"));
        out.divergenceReady = div.level == "high" || div.level == "medium";

        auto report = DataFamilyAcceptanceReportModel::build({div});
        out.reportReady = report.total == 1;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_query_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.canonicalReady && out.postgresReady && out.tsqlReady && out.mysqlReady &&
                      out.transactionReady && out.equivalenceReady && out.divergenceReady && out.reportReady &&
                      out.mcpToolReady && out.stepStart == 789 && out.stepEnd == 798;
        return out;
    }
};
