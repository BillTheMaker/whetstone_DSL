#pragma once
// Step 778: Sprint 54 integration summary.

#include <algorithm>
#include <string>
#include <vector>

#include "MCPServer.h"
#include "ast_native/SExpressionCanonicalLowering.h"
#include "ast_native/LispAdapterV1.h"
#include "ast_native/SchemeAdapterV1.h"
#include "ast_native/ElispAdapterV1.h"
#include "ast_native/SmalltalkAdapterV1.h"
#include "ast_native/MacroHygieneBoundary.h"
#include "ast_native/EvalRuntimeRiskModel.h"
#include "ast_native/ASTNativeProjectionBenchmark.h"

struct Sprint54IntegrationResult {
    bool canonicalReady = false;
    bool lispReady = false;
    bool schemeReady = false;
    bool elispReady = false;
    bool smalltalkReady = false;
    bool macroReady = false;
    bool evalReady = false;
    bool benchmarkReady = false;
    bool mcpToolReady = false;
    bool success = false;
    int stepStart = 769;
    int stepEnd = 778;
    std::vector<std::string> filesAdded;
};

class Sprint54IntegrationSummary {
public:
    static Sprint54IntegrationResult run() {
        Sprint54IntegrationResult out;
        out.filesAdded = {
            "ast_native/SExpressionCanonicalLowering.h",
            "ast_native/LispAdapterV1.h",
            "ast_native/SchemeAdapterV1.h",
            "ast_native/ElispAdapterV1.h",
            "ast_native/SmalltalkAdapterV1.h",
            "ast_native/MacroHygieneBoundary.h",
            "ast_native/EvalRuntimeRiskModel.h",
            "ast_native/ASTNativeProjectionBenchmark.h",
            "mcp/RegisterASTNativeFamilyTools.h",
            "Sprint54IntegrationSummary.h"
        };
        std::sort(out.filesAdded.begin(), out.filesAdded.end());

        auto canon = SExpressionCanonicalLowering::lower("(define x 1)", "lisp");
        auto li = LispAdapterV1::lower("(defmacro m () nil)");
        auto sc = SchemeAdapterV1::lower("(define (f x) x)");
        auto el = ElispAdapterV1::lower("(defmacro m () nil)");
        auto st = SmalltalkAdapterV1::lower("Object new do: [:x | x]");

        out.canonicalReady = canon.canonicalForm == "sexpr_ir_v1";
        out.lispReady = li.sourceLanguage == "lisp";
        out.schemeReady = sc.sourceLanguage == "scheme";
        out.elispReady = el.sourceLanguage == "elisp";
        out.smalltalkReady = st.sourceLanguage == "smalltalk";

        auto macro = MacroHygieneBoundaryModel::classify(li, "lisp");
        out.macroReady = macro.macroBoundaryPresent;

        auto eval = EvalRuntimeRiskModel::classify("(eval (read))");
        out.evalReady = eval.level != "low";

        auto bench = ASTNativeProjectionBenchmarkSuite::run(10, 1);
        out.benchmarkReady = bench.equivalenceConfidence > 0.0;

        MCPServer mcp;
        auto list = mcp.handleRequest({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}});
        for (const auto& t : list["result"]["tools"]) {
            if (t.value("name", "") == "whetstone_transpile_ast_native_family") {
                out.mcpToolReady = true;
                break;
            }
        }

        out.success = out.canonicalReady && out.lispReady && out.schemeReady && out.elispReady && out.smalltalkReady &&
                      out.macroReady && out.evalReady && out.benchmarkReady && out.mcpToolReady &&
                      out.stepStart == 769 && out.stepEnd == 778;
        return out;
    }
};
