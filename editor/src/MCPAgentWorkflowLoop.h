#pragma once

#include "MCPServer.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

class MCPAgentWorkflowLoop {
public:
    struct Config {
        std::string projectName = "workflow-project";
        std::string skeletonName = "skeleton";
        std::string language = "cpp";
    };

    struct Blocker {
        std::string itemId;
        std::string kind;
        std::string prompt;
    };

    struct AgentSubmission {
        std::string generatedCode;
        double confidence = 0.8;
        std::string reasoning;
    };

    struct Report {
        bool success = false;
        int blockersProcessed = 0;
        std::vector<std::string> toolCalls;
        json finalProgress = json::object();
    };

    using AgentHandler = std::function<AgentSubmission(const Blocker&)>;

    explicit MCPAgentWorkflowLoop(MCPServer& server) : server_(server) {}

    Report run(const Config& config, AgentHandler handler) {
        Report r;
        ensureRequiredToolsPresent();

        callTool("whetstone_infer_annotations", json::object(), r);
        callTool("whetstone_create_skeleton",
                 {{"name", config.skeletonName}, {"language", config.language}}, r);
        callTool("whetstone_create_workflow", {{"projectName", config.projectName}}, r);
        callTool("whetstone_orchestrate_run_deterministic", json::object(), r);

        json blockersResp = callTool("whetstone_get_blockers", json::object(), r);
        auto blockers = parseBlockers(blockersResp);
        for (const auto& b : blockers) {
            AgentSubmission sub = handler ? handler(b) : AgentSubmission{};
            json result = {
                {"generatedCode", sub.generatedCode},
                {"confidence", sub.confidence},
                {"reasoning", sub.reasoning}
            };
            callTool("whetstone_submit_result",
                     {{"itemId", b.itemId}, {"result", result}}, r);
            r.blockersProcessed++;
        }

        r.finalProgress = callTool("whetstone_get_progress", json::object(), r);
        r.success = !r.finalProgress.contains("error");
        return r;
    }

    static std::vector<Blocker> parseBlockers(const json& response) {
        std::vector<Blocker> out;
        if (!response.is_object()) return out;
        if (!response.contains("blockers") || !response["blockers"].is_array()) return out;

        for (const auto& b : response["blockers"]) {
            Blocker item;
            item.itemId = b.value("itemId", b.value("id", ""));
            item.kind = b.value("kind", b.value("type", ""));
            item.prompt = b.value("prompt", b.value("reason", ""));
            if (!item.itemId.empty()) out.push_back(item);
        }
        return out;
    }

private:
    MCPServer& server_;

    void ensureRequiredToolsPresent() const {
        const std::vector<std::string> required = {
            "whetstone_infer_annotations",
            "whetstone_create_skeleton",
            "whetstone_create_workflow",
            "whetstone_orchestrate_run_deterministic",
            "whetstone_get_blockers",
            "whetstone_submit_result",
            "whetstone_get_progress"
        };
        for (const auto& name : required) {
            if (!hasTool(name)) {
                throw std::runtime_error("Missing required MCP tool: " + name);
            }
        }
    }

    bool hasTool(const std::string& name) const {
        for (const auto& t : server_.getTools()) if (t.name == name) return true;
        return false;
    }

    json callTool(const std::string& name, const json& args, Report& r) {
        r.toolCalls.push_back(name);
        json req = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "tools/call"},
            {"params", {
                {"name", name},
                {"arguments", args}
            }}
        };

        json resp = server_.handleRequest(req);
        if (!resp.contains("result")) return json{{"error", "missing result"}};
        auto result = resp["result"];
        if (result.value("isError", false)) return json{{"error", "tool call failed"}};
        if (!result.contains("content") || !result["content"].is_array() || result["content"].empty()) {
            return json{{"error", "missing content"}};
        }

        std::string text = result["content"][0].value("text", "");
        try {
            return json::parse(text);
        } catch (...) {
            return json{{"raw", text}};
        }
    }
};
