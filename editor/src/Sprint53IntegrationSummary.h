#pragma once
// Step 768: Sprint 53 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "managed/KotlinAdapterV1.h"
#include "managed/CSharpAdapterV1.h"
#include "managed/FSharpAdapterV1.h"
#include "managed/VbNetAdapterV1.h"
#include "managed/NullabilityOptionalityBridge.h"
#include "managed/AsyncModelBridge.h"
#include "managed/ADTPatternCanonicalLowering.h"
#include "managed/ManagedFamilyPromotion.h"

struct Sprint53IntegrationResult {
    bool kotlinReady = false;
    bool csharpReady = false;
    bool fsharpReady = false;
    bool vbnetReady = false;
    bool nullabilityReady = false;
    bool asyncReady = false;
    bool adtReady = false;
    bool promotionReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 759;
    int stepEnd = 768;
    std::vector<std::string> filesAdded;
};

class Sprint53IntegrationSummary {
public:
    static Sprint53IntegrationResult run() {
        Sprint53IntegrationResult out;
        out.filesAdded = {
            "managed/ManagedPacketTypes.h",
            "managed/KotlinAdapterV1.h",
            "managed/CSharpAdapterV1.h",
            "managed/FSharpAdapterV1.h",
            "managed/VbNetAdapterV1.h",
            "managed/NullabilityOptionalityBridge.h",
            "managed/AsyncModelBridge.h",
            "managed/ADTPatternCanonicalLowering.h",
            "managed/ManagedFamilyPromotion.h",
            "mcp/RegisterManagedFamilyTools.h",
            "Sprint53IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto kt = KotlinAdapterV1::lower("sealed class X; suspend fun go(x: String?) = x");
        auto cs = CSharpAdapterV1::lower("public async Task<string?> Go() => null;");
        auto fs = FSharpAdapterV1::lower("type R = A | B\nlet run = async { return 1 }");
        auto vb = VbNetAdapterV1::lower("Public Async Function Go() As Task\nSelect Case x\nEnd Select\nEnd Function");

        out.kotlinReady = kt.sourceLanguage == "kotlin";
        out.csharpReady = cs.sourceLanguage == "csharp";
        out.fsharpReady = fs.sourceLanguage == "fsharp";
        out.vbnetReady = vb.sourceLanguage == "vbnet";

        auto nullBridge = NullabilityOptionalityBridge::fromLowering(cs);
        out.nullabilityReady = nullBridge.canonicalModel == "nullable";

        auto asyncBridge = AsyncModelBridge::plan("kotlin", "csharp", kt);
        out.asyncReady = !asyncBridge.bridgeStrategy.empty();

        auto adt = ADTPatternCanonicalLowering::lower("sealed class S; when(x){}");
        out.adtReady = adt.canonicalShape == "sum_type_with_patterns";

        auto promo = ManagedFamilyPromotionMatrix::evaluate(ManagedFamilyPromotionMatrix::defaultPairs());
        out.promotionReady = promo.betaCount >= 1;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_managed_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.kotlinReady && out.csharpReady && out.fsharpReady && out.vbnetReady &&
                      out.nullabilityReady && out.asyncReady && out.adtReady && out.promotionReady &&
                      out.mcpToolReady && out.stepStart == 759 && out.stepEnd == 768;
        return out;
    }
};
