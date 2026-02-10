#pragma once
// Step 162: Help panel markdown renderer

#include "imgui.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

struct HelpSection {
    std::string title;
    std::string content;
};

struct HelpPanelState {
    int selected = 0;
    std::vector<HelpSection> sections;
    std::string lastRoot;
};

static bool loadHelpFile(const std::string& path, std::string& out) {
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    return true;
}

static std::vector<HelpSection> parseHelpSections(const std::string& text) {
    std::vector<HelpSection> sections;
    std::istringstream ss(text);
    std::string line;
    HelpSection current;
    while (std::getline(ss, line)) {
        if (line.rfind("# ", 0) == 0) {
            if (!current.title.empty() || !current.content.empty()) {
                sections.push_back(current);
                current = HelpSection();
            }
            current.title = line.substr(2);
        } else {
            current.content += line + "\n";
        }
    }
    if (!current.title.empty() || !current.content.empty()) {
        sections.push_back(current);
    }
    if (sections.empty()) {
        sections.push_back({"Documentation", text});
    }
    return sections;
}

static void renderMarkdownLine(const std::string& line, ImFont* monoFont) {
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

static void renderMarkdown(const std::string& text, ImFont* monoFont) {
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        renderMarkdownLine(line, monoFont);
    }
}

static void loadHelpContent(HelpPanelState& state, const std::string& root) {
    if (state.lastRoot == root && !state.sections.empty()) return;
    state.sections.clear();
    state.selected = 0;
    state.lastRoot = root;

    std::string content;
    if (!root.empty()) {
        if (!loadHelpFile(root + "\\docs\\help.md", content)) {
            loadHelpFile(root + "/docs/help.md", content);
        }
    }
    if (content.empty()) {
        content = "# Documentation\n\nDocumentation not found.\n";
    }
    state.sections = parseHelpSections(content);
}

static void renderHelpPanel(HelpPanelState& state,
                            const std::string& workspaceRoot,
                            ImFont* monoFont) {
    loadHelpContent(state, workspaceRoot);
    if (state.sections.empty()) {
        ImGui::TextDisabled("(no documentation)");
        return;
    }

    ImGui::Columns(2, "help_cols", true);
    ImGui::BeginChild("help_nav", ImVec2(0, 0), true);
    for (size_t i = 0; i < state.sections.size(); ++i) {
        const auto& section = state.sections[i];
        bool selected = (int)i == state.selected;
        if (ImGui::Selectable(section.title.c_str(), selected)) {
            state.selected = (int)i;
        }
    }
    ImGui::EndChild();
    ImGui::NextColumn();

    ImGui::BeginChild("help_body", ImVec2(0, 0), false);
    const auto& section = state.sections[std::min((int)state.sections.size() - 1, state.selected)];
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "%s", section.title.c_str());
    ImGui::Separator();
    renderMarkdown(section.content, monoFont);
    ImGui::EndChild();
    ImGui::Columns(1);
}
