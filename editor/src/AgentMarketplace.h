#pragma once
// Step 159: Agent marketplace UI

#include "imgui.h"
#include "AgentRegistry.h"
#include <string>
#include <algorithm>

struct AgentMarketplaceState {
    char filterBuf[128] = {};
    char registryPathBuf[260] = {};
    char registryUrlBuf[260] = {};
    std::string selectedAgent;
};

static bool agentMatchesFilter(const AgentDescriptor& agent,
                               const std::string& filter) {
    if (filter.empty()) return true;
    std::string name = agent.name;
    std::string desc = agent.description;
    std::string f = filter;
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    std::transform(desc.begin(), desc.end(), desc.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    std::transform(f.begin(), f.end(), f.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return name.find(f) != std::string::npos || desc.find(f) != std::string::npos;
}

static void renderAgentMarketplace(AgentMarketplaceState& state,
                                   AgentRegistry& registry,
                                   std::string& outputLog,
                                   const std::string& workspaceRoot) {
    if (state.registryPathBuf[0] == '\0') {
        std::string path = workspaceRoot.empty() ? "agents.json"
                                                 : (workspaceRoot + "\\agents.json");
        std::snprintf(state.registryPathBuf, sizeof(state.registryPathBuf), "%s", path.c_str());
    }
    if (state.registryUrlBuf[0] == '\0' && !registry.getRegistryUrl().empty()) {
        std::snprintf(state.registryUrlBuf, sizeof(state.registryUrlBuf), "%s",
                      registry.getRegistryUrl().c_str());
    }

    ImGui::InputText("Filter", state.filterBuf, sizeof(state.filterBuf));
    ImGui::InputText("Registry URL", state.registryUrlBuf, sizeof(state.registryUrlBuf));
    if (ImGui::Button("Set URL")) {
        registry.setRegistryUrl(state.registryUrlBuf);
        outputLog += "[agents] Registry URL set.\n";
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        outputLog += "[agents] STUB: refresh registry from " +
                     std::string(state.registryUrlBuf) + "\n";
    }

    ImGui::InputText("Registry File", state.registryPathBuf, sizeof(state.registryPathBuf));
    if (ImGui::Button("Load")) {
        std::string error;
        if (!registry.loadFromFile(state.registryPathBuf, error)) {
            outputLog += "[agents] Load failed: " + error + "\n";
        } else {
            outputLog += "[agents] Loaded registry.\n";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        std::string error;
        if (!registry.saveToFile(state.registryPathBuf, error)) {
            outputLog += "[agents] Save failed: " + error + "\n";
        } else {
            outputLog += "[agents] Saved registry.\n";
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Available Agents");

    std::string filter = state.filterBuf;
    const auto& agents = registry.list();
    if (agents.empty()) {
        ImGui::TextDisabled("(no agents)");
    } else {
        for (const auto& agent : agents) {
            if (!agentMatchesFilter(agent, filter)) continue;
            std::string label = agent.name;
            if (agent.installed) label += " [installed]";
            bool selected = state.selectedAgent == agent.name;
            if (ImGui::Selectable(label.c_str(), selected)) {
                state.selectedAgent = agent.name;
            }
        }
    }

    ImGui::Separator();
    if (!state.selectedAgent.empty()) {
        AgentDescriptor* selected = registry.find(state.selectedAgent);
        if (selected) {
            ImGui::Text("Name: %s", selected->name.c_str());
            ImGui::TextWrapped("%s", selected->description.c_str());
            ImGui::Text("Endpoint: %s", selected->endpoint.c_str());
            if (!selected->capabilities.empty()) {
                ImGui::Text("Capabilities:");
                for (const auto& cap : selected->capabilities) {
                    ImGui::BulletText("%s", cap.c_str());
                }
            }
            if (!selected->requiredPermissions.empty()) {
                ImGui::Text("Permissions:");
                for (const auto& perm : selected->requiredPermissions) {
                    ImGui::BulletText("%s", perm.c_str());
                }
            }
            if (!selected->installed) {
                if (ImGui::Button("Install Agent")) {
                    selected->installed = true;
                    outputLog += "[agents] Installed " + selected->name +
                                 " (" + selected->endpoint + ")\n";
                }
            } else {
                if (ImGui::Button("Uninstall Agent")) {
                    selected->installed = false;
                    outputLog += "[agents] Uninstalled " + selected->name + "\n";
                }
            }
        }
    }
}
