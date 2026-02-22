#pragma once
// Step 748: Sprint 51 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "systems/CAdapterV1.h"
#include "systems/GoAdapterV1.h"
#include "systems/JavaAdapterV1.h"
#include "systems/CRaisingAdapterV1.h"
#include "systems/GoRaisingAdapterV1.h"
#include "systems/JavaRaisingAdapterV1.h"
#include "systems/SystemsCompatibilityMatrix.h"
#include "systems/SystemsFamilyAcceptanceReport.h"

struct Sprint51IntegrationResult {
    bool cLowerReady = false;
    bool goLowerReady = false;
    bool javaLowerReady = false;
    bool cRaiseReady = false;
    bool goRaiseReady = false;
    bool javaRaiseReady = false;
    bool matrixReady = false;
    bool reportReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 739;
    int stepEnd = 748;
    std::vector<std::string> filesAdded;
};

class Sprint51IntegrationSummary {
public:
    static Sprint51IntegrationResult run() {
        Sprint51IntegrationResult out;
        out.filesAdded = {
            "systems/CAdapterV1.h",
            "systems/GoAdapterV1.h",
            "systems/JavaAdapterV1.h",
            "systems/CRaisingAdapterV1.h",
            "systems/GoRaisingAdapterV1.h",
            "systems/JavaRaisingAdapterV1.h",
            "systems/SystemsCompatibilityMatrix.h",
            "systems/SystemsFamilyAcceptanceReport.h",
            "mcp/RegisterSystemsFamilyTools.h",
            "Sprint51IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto cL = CLoweringAdapterV1::lower("int main(){}");
        auto gL = GoLoweringAdapterV1::lower("package main");
        auto jL = JavaLoweringAdapterV1::lower("class A{}");
        out.cLowerReady = cL.sourceLanguage == "c";
        out.goLowerReady = gL.sourceLanguage == "go";
        out.javaLowerReady = jL.sourceLanguage == "java";

        auto cR = CRaisingAdapterV1::raise(cL.irSummary, "safe-first");
        auto gR = GoRaisingAdapterV1::raise(gL.irSummary, "safe-first");
        auto jR = JavaRaisingAdapterV1::raise(jL.irSummary, "safe-first");
        out.cRaiseReady = cR.targetLanguage == "c";
        out.goRaiseReady = gR.targetLanguage == "go";
        out.javaRaiseReady = jR.targetLanguage == "java";

        auto pairs = SystemsCompatibilityMatrix::defaultPairs();
        out.matrixReady = !pairs.empty();
        out.reportReady = SystemsFamilyAcceptanceReportModel::build(pairs).pairCount == (int)pairs.size();

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_systems_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.cLowerReady && out.goLowerReady && out.javaLowerReady && out.cRaiseReady && out.goRaiseReady &&
                      out.javaRaiseReady && out.matrixReady && out.reportReady && out.mcpToolReady &&
                      out.stepStart == 739 && out.stepEnd == 748;
        return out;
    }
};
