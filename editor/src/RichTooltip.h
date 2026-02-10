#pragma once

#include "imgui.h"
#include "AnimationUtils.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

struct RichTooltipState {
    bool pinned = false;
    ImVec2 pinnedPos = ImVec2(0, 0);
    std::string pinnedContent;
};

inline std::unordered_map<std::string, RichTooltipState>& richTooltipStates() {
    static std::unordered_map<std::string, RichTooltipState> states;
    return states;
}

inline void renderMarkdownLine(const std::string& line, ImFont* monoFont) {
    std::string trimmed = line;
    while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n')) {
        trimmed.pop_back();
    }
    if (trimmed.rfind("## ", 0) == 0) {
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s", trimmed.substr(3).c_str());
    } else if (trimmed.rfind("### ", 0) == 0) {
        ImGui::TextColored(ImVec4(0.6f, 0.85f, 0.95f, 1.0f), "%s", trimmed.substr(4).c_str());
    } else if (trimmed.rfind("- ", 0) == 0) {
        ImGui::BulletText("%s", trimmed.substr(2).c_str());
    } else if (trimmed.rfind("```", 0) == 0) {
        ImGui::TextDisabled("%s", trimmed.c_str());
    } else if (trimmed.rfind("`", 0) == 0 && trimmed.size() > 1) {
        ImGui::PushFont(monoFont);
        ImGui::TextUnformatted(trimmed.c_str());
        ImGui::PopFont();
    } else {
        ImGui::TextWrapped("%s", trimmed.c_str());
    }
}

inline void renderMarkdown(const std::string& text, ImFont* monoFont) {
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        renderMarkdownLine(line, monoFont);
    }
}

inline void renderRichTooltip(const std::string& id,
                              const std::string& content,
                              bool hovered,
                              bool reduceMotion,
                              ImFont* monoFont,
                              float maxWidth = 420.0f,
                              float maxHeight = 240.0f) {
    auto& state = richTooltipStates()[id];
    const double now = ImGui::GetTime();
    float alpha = AnimationUtils::tooltipAlpha(id, hovered, now, reduceMotion);
    if (alpha > 0.01f && hovered && !state.pinned) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + maxWidth);
        renderMarkdown(content, monoFont);
        ImGui::PopTextWrapPos();
        ImGui::Separator();
        if (ImGui::SmallButton("Pin")) {
            state.pinned = true;
            state.pinnedPos = ImGui::GetMousePos();
            state.pinnedContent = content;
        }
        ImGui::EndTooltip();
        ImGui::PopStyleVar();
    }

    if (state.pinned) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings;
        ImGui::SetNextWindowPos(state.pinnedPos, ImGuiCond_FirstUseEver);
        bool open = true;
        if (ImGui::Begin(("Tooltip##" + id).c_str(), &open, flags)) {
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + maxWidth);
            ImGui::BeginChild(("##tip_scroll_" + id).c_str(), ImVec2(maxWidth, maxHeight), false);
            renderMarkdown(state.pinnedContent, monoFont);
            ImGui::EndChild();
            ImGui::PopTextWrapPos();
            if (ImGui::SmallButton("Close")) {
                open = false;
            }
        }
        ImGui::End();
        if (!open) {
            state.pinned = false;
            state.pinnedContent.clear();
        }
    }
}
