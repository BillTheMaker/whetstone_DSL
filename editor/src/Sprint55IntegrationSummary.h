#pragma once
// Step 788: Sprint 55 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "logic_actor/LogicActorPacketTypes.h"
#include "logic_actor/PrologAdapterV1.h"
#include "logic_actor/ErlangAdapterV1.h"
#include "logic_actor/ElixirAdapterV1.h"
#include "logic_actor/LogicImperativeProjectionPolicy.h"
#include "logic_actor/ActorAsyncProjectionPolicy.h"
#include "logic_actor/SupervisionTreePacket.h"
#include "logic_actor/SemanticBlocklistGate.h"
#include "logic_actor/LogicActorAcceptanceReport.h"

struct Sprint55IntegrationResult {
    bool prologReady = false;
    bool erlangReady = false;
    bool elixirReady = false;
    bool logicPolicyReady = false;
    bool actorPolicyReady = false;
    bool supervisionReady = false;
    bool gateReady = false;
    bool reportReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 779;
    int stepEnd = 788;
    std::vector<std::string> filesAdded;
};

class Sprint55IntegrationSummary {
public:
    static Sprint55IntegrationResult run() {
        Sprint55IntegrationResult out;
        out.filesAdded = {
            "logic_actor/LogicActorPacketTypes.h",
            "logic_actor/PrologAdapterV1.h",
            "logic_actor/ErlangAdapterV1.h",
            "logic_actor/ElixirAdapterV1.h",
            "logic_actor/LogicImperativeProjectionPolicy.h",
            "logic_actor/ActorAsyncProjectionPolicy.h",
            "logic_actor/SupervisionTreePacket.h",
            "logic_actor/SemanticBlocklistGate.h",
            "logic_actor/LogicActorAcceptanceReport.h",
            "mcp/RegisterLogicActorFamilyTools.h",
            "Sprint55IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto pl = PrologAdapterV1::lower("member(X,[1,2]); fail.");
        auto er = ErlangAdapterV1::lower("supervisor:start_link(), spawn(fun loop/0), receive end.");
        auto ex = ElixirAdapterV1::lower("spawn(fn -> :ok end)");
        out.prologReady = pl.sourceLanguage == "prolog";
        out.erlangReady = er.sourceLanguage == "erlang";
        out.elixirReady = ex.sourceLanguage == "elixir";

        auto logic = LogicImperativeProjectionPolicy::choose(pl, "cpp");
        out.logicPolicyReady = logic.reviewRequired;

        auto actor = ActorAsyncProjectionPolicy::choose(er, "cpp");
        out.actorPolicyReady = actor.reviewRequired;

        auto supervision = SupervisionTreePacketModel::build(er, "cpp");
        out.supervisionReady = supervision.supervisionDetected;

        auto gate = SemanticBlocklistGate::evaluate(er, "c");
        out.gateReady = gate.reviewRequired;

        auto report = LogicActorAcceptanceReportModel::build({gate});
        out.reportReady = report.total == 1;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_logic_actor_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.prologReady && out.erlangReady && out.elixirReady && out.logicPolicyReady && out.actorPolicyReady &&
                      out.supervisionReady && out.gateReady && out.reportReady && out.mcpToolReady &&
                      out.stepStart == 779 && out.stepEnd == 788;
        return out;
    }
};
