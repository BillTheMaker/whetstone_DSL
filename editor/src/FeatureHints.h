#pragma once

#include "imgui.h"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

struct FeatureHintsState {
    bool disabled = false;
    bool shownThisSession = false;
    std::string activeKey;
    std::string activeMessage;
    std::map<std::string, int> shownCounts;
};

static std::filesystem::path hintsFilePath(const std::string& workspaceRoot) {
    const char* home = std::getenv("USERPROFILE");
    if (!home) home = std::getenv("HOME");
    std::filesystem::path base = home ? std::filesystem::path(home) : std::filesystem::current_path();
    (void)workspaceRoot;
    return base / ".whetstone" / "hints.json";
}

static void loadFeatureHints(FeatureHintsState& state, const std::string& workspaceRoot) {
    std::filesystem::path path = hintsFilePath(workspaceRoot);
    if (!std::filesystem::exists(path)) return;
    std::ifstream in(path.string());
    if (!in.is_open()) return;
    nlohmann::json j;
    in >> j;
    state.disabled = j.value("disabled", false);
    if (j.contains("shown") && j["shown"].is_object()) {
        for (auto it = j["shown"].begin(); it != j["shown"].end(); ++it) {
            state.shownCounts[it.key()] = it.value().get<int>();
        }
    }
}

static void saveFeatureHints(const FeatureHintsState& state, const std::string& workspaceRoot) {
    std::filesystem::path path = hintsFilePath(workspaceRoot);
    std::filesystem::create_directories(path.parent_path());
    nlohmann::json j;
    j["disabled"] = state.disabled;
    j["shown"] = nlohmann::json::object();
    for (const auto& [key, count] : state.shownCounts) {
        j["shown"][key] = count;
    }
    std::ofstream out(path.string(), std::ios::binary);
    if (!out.is_open()) return;
    out << j.dump(2);
}

static void queueFeatureHint(FeatureHintsState& state,
                             const std::string& key,
                             const std::string& message) {
    if (state.disabled || state.shownThisSession) return;
    if (state.shownCounts[key] > 0) return;
    state.activeKey = key;
    state.activeMessage = message;
}

static void renderFeatureHintBar(FeatureHintsState& state, const std::string& workspaceRoot) {
    if (state.activeMessage.empty()) return;

    ImGui::SetNextWindowBgAlpha(0.95f);
    ImGui::Begin("##FeatureHintBar", nullptr,
                 ImGuiWindowFlags_NoDecoration |
                 ImGuiWindowFlags_NoDocking |
                 ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextUnformatted(state.activeMessage.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Got it")) {
        state.shownCounts[state.activeKey] = 1;
        state.shownThisSession = true;
        state.activeKey.clear();
        state.activeMessage.clear();
        saveFeatureHints(state, workspaceRoot);
    }
    ImGui::SameLine();
    if (ImGui::Button("Don't show tips")) {
        state.disabled = true;
        state.shownThisSession = true;
        state.activeKey.clear();
        state.activeMessage.clear();
        saveFeatureHints(state, workspaceRoot);
    }
    ImGui::End();
}
