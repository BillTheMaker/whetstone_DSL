#pragma once
// Step 623: Sprint 36 Integration Summary

#include "MCPServer.h"

#include <string>
#include <vector>

struct Sprint36IntegrationResult {
    bool initializeOk = false;
    bool intakeOk = false;
    bool generateOk = false;
    bool queueOk = false;
    bool queueReady = false;
    int taskCount = 0;
    int readyCount = 0;
    int escalateCount = 0;
    std::vector<std::string> blockers;
};

class Sprint36IntegrationSummary {
public:
    static Sprint36IntegrationResult run(const std::string& markdown) {
        Sprint36IntegrationResult out;
        MCPServer mcp;

        json initReq = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "initialize"},
            {"params", {{"protocolVersion", "2024-11-05"}}}
        };
        json initResp = mcp.handleRequest(initReq);
        out.initializeOk = initResp.contains("result") &&
                           initResp["result"].contains("instructions") &&
                           initResp["result"]["instructions"].is_string() &&
                           !initResp["result"].value("instructions", "").empty();

        json intake = callTool(mcp, "whetstone_architect_intake", {{"markdown", markdown}});
        out.intakeOk = intake.value("success", false);
        if (!out.intakeOk) return out;

        json generated = callTool(mcp, "whetstone_generate_taskitems", {
            {"normalizedRequirements", intake["normalizedRequirements"]},
            {"conflicts", intake["conflicts"]}
        });
        out.generateOk = generated.value("success", false);
        if (!out.generateOk) return out;

        json queue = callTool(mcp, "whetstone_queue_ready", {
            {"tasks", generated["tasks"]},
            {"normalizedRequirements", intake["normalizedRequirements"]}
        });
        out.queueOk = queue.value("success", false);
        if (!out.queueOk) return out;

        out.queueReady = queue.value("ready", false);
        out.taskCount = (int)generated["tasks"].size();
        out.readyCount = queue.value("readyCount", 0);
        out.escalateCount = queue.value("escalateCount", 0);
        for (const auto& blocker : queue["blockers"]) {
            out.blockers.push_back(blocker.get<std::string>());
        }
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
