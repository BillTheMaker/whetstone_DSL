#pragma once
// Step 618: Sprint 36a MCP Intake Integration

#include "MCPServer.h"

#include <string>
#include <vector>

struct Sprint36aIntegrationResult {
    bool success = false;
    std::string stage;
    int taskCount = 0;
    int readyCount = 0;
    int escalateCount = 0;
    std::vector<std::string> blockers;
};

class Sprint36aIntegration {
public:
    static Sprint36aIntegrationResult run(const std::string& markdown) {
        Sprint36aIntegrationResult out;
        MCPServer mcp;

        json intake = callTool(mcp, "whetstone_architect_intake", {{"markdown", markdown}});
        if (!intake.value("success", false)) {
            out.stage = "intake";
            return out;
        }

        json generated = callTool(mcp, "whetstone_generate_taskitems", {
            {"normalizedRequirements", intake["normalizedRequirements"]},
            {"conflicts", intake["conflicts"]}
        });
        if (!generated.value("success", false)) {
            out.stage = "generate";
            return out;
        }

        json queue = callTool(mcp, "whetstone_queue_ready", {
            {"tasks", generated["tasks"]},
            {"normalizedRequirements", intake["normalizedRequirements"]}
        });
        if (!queue.value("success", false)) {
            out.stage = "queue";
            return out;
        }

        out.stage = "done";
        out.taskCount = (int)generated["tasks"].size();
        out.readyCount = queue.value("readyCount", 0);
        out.escalateCount = queue.value("escalateCount", 0);
        for (const auto& blocker : queue["blockers"]) {
            out.blockers.push_back(blocker.get<std::string>());
        }
        out.success = queue.value("ready", false);
        return out;
    }

private:
    static json callTool(MCPServer& mcp, const std::string& name, const json& args) {
        json req = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "tools/call"},
            {"params", {{"name", name}, {"arguments", args}}}
        };
        json resp = mcp.handleRequest(req);
        std::string text = resp["result"]["content"][0].value("text", "{}");
        return json::parse(text);
    }
};
