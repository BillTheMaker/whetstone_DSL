#pragma once
// Step 625: Tool call visualization model

#include <nlohmann/json.hpp>

#include <string>

using json = nlohmann::json;

struct AgentToolCallView {
    std::string toolName;
    std::string summary;
    std::string inputJson;
    std::string outputJson;
    bool expanded = false;
};

class AgentToolCallVisualization {
public:
    static AgentToolCallView build(const std::string& toolName,
                                   const json& input,
                                   const json& output) {
        AgentToolCallView view;
        view.toolName = toolName;
        view.summary = summarize(toolName, input);
        view.inputJson = input.dump(2);
        view.outputJson = output.dump(2);
        return view;
    }

    static std::string inlineLabel(const AgentToolCallView& view) {
        const std::string chevron = view.expanded ? "v" : ">";
        return "[TOOL: " + view.toolName + "] " + chevron + " " + view.summary;
    }

    static void toggleExpanded(AgentToolCallView* view) {
        if (!view) return;
        view->expanded = !view->expanded;
    }

private:
    static std::string summarize(const std::string& toolName, const json& input) {
        if (toolName == "whetstone_mutate") {
            std::string type = input.value("type", "mutation");
            std::string nodeId = input.value("nodeId", "?");
            std::string property = input.value("property", "");
            std::string value = input.contains("value") ? input["value"].dump() : "null";
            if (!property.empty()) return type + " " + nodeId + "." + property + " -> " + value;
            return type + " " + nodeId + " -> " + value;
        }
        if (toolName == "whetstone_generate_code") {
            std::string spec = input.value("spec", "");
            if (spec.size() > 40) spec = spec.substr(0, 40) + "...";
            return "generate from spec: " + spec;
        }
        if (toolName == "whetstone_run_pipeline") {
            std::string src = input.value("sourceLanguage", "?");
            std::string tgt = input.value("targetLanguage", "?");
            return "pipeline " + src + " -> " + tgt;
        }
        return "call";
    }
};
