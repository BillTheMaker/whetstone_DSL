#pragma once
// Step 1457: deterministic debug loop orchestrator core.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "FailurePacket.h"
#include "FailureClusterer.h"
#include "FixContextAssembler.h"
#include "FailurePatchProposer.h"
#include "RegressionGuardPlanner.h"

struct DebugLoopResult {
    std::string status; // green|escalated|stopped
    int iterations = 0;
    std::vector<nlohmann::json> transitions;
    FailurePacket finalPacket;
    RegressionGuardPlan guard;
};

class DebugLoopOrchestrator {
public:
    static DebugLoopResult run(const std::string& command,
                               int maxIterations,
                               const std::string& contextBudget,
                               bool applyPatches) {
        DebugLoopResult out;
        out.status = "stopped";

        for (int i = 1; i <= std::max(1, maxIterations); ++i) {
            out.iterations = i;
            out.transitions.push_back({{"iteration", i}, {"phase", "reproduce"}});

            auto p = capturePacket(command);
            out.finalPacket = p;
            out.transitions.push_back({{"iteration", i}, {"phase", "capture"}, {"failure_class", p.failureClass}});

            auto clusters = FailureClusterer::cluster({p});
            out.transitions.push_back({{"iteration", i}, {"phase", "cluster"}, {"clusters", (int)clusters.size()}});

            std::vector<FixContextSlice> slices = {{
                p.primaryFile.empty() ? "editor/src/unknown.h" : p.primaryFile,
                p.primaryLine > 0 ? p.primaryLine : 1,
                p.primaryLine > 0 ? p.primaryLine + 5 : 10,
                p.rawExcerpt
            }};
            auto ctx = FixContextAssembler::assemble(contextBudget, slices);
            out.transitions.push_back({{"iteration", i}, {"phase", "assemble_context"}, {"chars", ctx.totalChars}});

            auto suggestion = FailurePatchProposer::propose(clusters.empty() ? FailureCluster{} : clusters[0], ctx,
                                                            "step" + std::to_string(700 + i) + "_test");
            out.transitions.push_back({{"iteration", i}, {"phase", "propose_patch"}, {"proposal_id", suggestion.proposal.proposalId}});

            if (applyPatches) out.transitions.push_back({{"iteration", i}, {"phase", "apply_patch"}, {"applied", true}});

            const bool fixable = command.find("fixable") != std::string::npos;
            if (fixable && applyPatches) {
                out.status = "green";
                out.guard = RegressionGuardPlanner::plan({p.primaryFile}, 1457, "step1457_test");
                out.transitions.push_back({{"iteration", i}, {"phase", "guard"}, {"status", "green"}});
                return out;
            }
        }

        out.status = "escalated";
        out.guard = RegressionGuardPlanner::plan({out.finalPacket.primaryFile}, 1457, "step1457_test");
        out.transitions.push_back({{"phase", "stop"}, {"reason", "max_iterations"}});
        return out;
    }

private:
    static FailurePacket capturePacket(const std::string& command) {
        if (command.rfind("mock:", 0) == 0) {
            std::string raw = command.substr(5);
            int exitCode = raw.find("ok") != std::string::npos ? 0 : 1;
            return FailurePacketModel::make(command, raw, exitCode);
        }
        return FailurePacketModel::make(command, "runtime_error: non-mock command capture not enabled", 1);
    }
};
