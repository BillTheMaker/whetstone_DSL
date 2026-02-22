#pragma once
// Step 758: Sprint 52 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "dynamic/PythonAdapterV2.h"
#include "dynamic/JavaScriptAdapterV2.h"
#include "dynamic/TypeScriptAdapterV2.h"
#include "dynamic/RubyAdapterV1.h"
#include "dynamic/LuaAdapterV1.h"
#include "dynamic/DynamicStrictnessPolicy.h"
#include "dynamic/DynamicRiskClassifier.h"
#include "dynamic/DynamicFamilyAcceptanceReport.h"

struct Sprint52IntegrationResult {
    bool pythonReady = false;
    bool jsReady = false;
    bool tsReady = false;
    bool rubyReady = false;
    bool luaReady = false;
    bool policyReady = false;
    bool riskReady = false;
    bool reportReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 749;
    int stepEnd = 758;
    std::vector<std::string> filesAdded;
};

class Sprint52IntegrationSummary {
public:
    static Sprint52IntegrationResult run() {
        Sprint52IntegrationResult out;
        out.filesAdded = {
            "dynamic/PythonAdapterV2.h",
            "dynamic/JavaScriptAdapterV2.h",
            "dynamic/TypeScriptAdapterV2.h",
            "dynamic/RubyAdapterV1.h",
            "dynamic/LuaAdapterV1.h",
            "dynamic/DynamicStrictnessPolicy.h",
            "dynamic/DynamicRiskClassifier.h",
            "dynamic/DynamicFamilyAcceptanceReport.h",
            "mcp/RegisterDynamicFamilyTools.h",
            "Sprint52IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto py = PythonLoweringAdapterV2::lower("def f(x): return getattr(x, 'a')");
        auto js = JavaScriptLoweringAdapterV2::lower("obj[k]");
        auto ts = TypeScriptLoweringAdapterV2::lower("let x:any = 1");
        auto rb = RubyLoweringAdapterV1::lower("method_missing");
        auto lua = LuaLoweringAdapterV1::lower("setmetatable(t,{})");

        out.pythonReady = py.sourceLanguage == "python";
        out.jsReady = js.sourceLanguage == "javascript";
        out.tsReady = ts.sourceLanguage == "typescript";
        out.rubyReady = rb.sourceLanguage == "ruby";
        out.luaReady = lua.sourceLanguage == "lua";

        auto policy = DynamicStrictnessPolicyEngine::forMode("strict");
        out.policyReady = !policy.allowImplicitAny;

        auto risk = DynamicRiskClassifier::classify(py.dynamicDispatchCount, true, policy);
        out.riskReady = risk.level != "low";

        auto report = DynamicFamilyAcceptanceReportModel::build({py, js}, {risk, risk});
        out.reportReady = report.reviewRequiredCount >= 1;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_dynamic_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.pythonReady && out.jsReady && out.tsReady && out.rubyReady && out.luaReady &&
                      out.policyReady && out.riskReady && out.reportReady && out.mcpToolReady &&
                      out.stepStart == 749 && out.stepEnd == 758;
        return out;
    }
};
