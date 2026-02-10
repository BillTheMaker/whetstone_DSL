#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../ThemeEngine.h"
#include <fstream>
#include <filesystem>

static std::string detectGitBranch(const std::string& root) {
    if (root.empty()) return "-";
    std::filesystem::path headPath = std::filesystem::path(root) / ".git" / "HEAD";
    std::ifstream in(headPath.string());
    if (!in.is_open()) return "-";
    std::string line;
    std::getline(in, line);
    const std::string refPrefix = "ref: refs/heads/";
    if (line.rfind(refPrefix, 0) == 0) {
        return line.substr(refPrefix.size());
    }
    if (!line.empty()) return line.substr(0, 8);
    return "-";
}

static void toggleBufferMode(EditorState& state) {
    if (!state.active()) return;
    state.active()->bufferMode = state.active()->bufferMode == BufferManager::BufferMode::Text
        ? BufferManager::BufferMode::Structured
        : BufferManager::BufferMode::Text;
    state.buffers.setBufferMode(state.active()->path, state.active()->bufferMode);
    if (state.active()->bufferMode == BufferManager::BufferMode::Text) {
        state.suggestions.clear();
        state.whetstoneDiagnostics.clear();
        state.analysisPending = false;
    } else {
        state.onTextChanged();
    }
}

static std::string detectLineEnding(const std::string& text) {
    if (text.find("\r\n") != std::string::npos) return "CRLF";
    return "LF";
}

static void renderStatusBar(EditorState& state) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags sbFlags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings;
    float sbHeight = ImGui::GetFrameHeight() + 2;
    ImVec2 sbPos(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - sbHeight);
    ImVec2 sbSize(viewport->WorkSize.x, sbHeight);
    ImGui::SetNextWindowPos(sbPos);
    ImGui::SetNextWindowSize(sbSize);
    ImVec4 bg = ImVec4(0.10f, 0.12f, 0.16f, 1.0f);
    bool hasError = false;
    for (const auto& d : state.whetstoneDiagnostics) {
        if (d.severity == 1) { hasError = true; break; }
    }
    if (!hasError) {
        for (const auto& d : state.emacsDiagnostics) {
            if (d.severity == 1) { hasError = true; break; }
        }
    }
    if (state.build.runInProgress) {
        bg = ImVec4(0.10f, 0.35f, 0.25f, 1.0f);
    } else if (hasError) {
        bg = ImVec4(0.45f, 0.12f, 0.12f, 1.0f);
    }
    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::Begin("##StatusBar", nullptr, sbFlags);
    ImGui::PushFont(state.uiFont);

    // Left cluster: mode, language, encoding, line ending
    std::string modeLabel = state.active()
        ? (state.active()->bufferMode == BufferManager::BufferMode::Text ? "Text" : "Structured")
        : "-";
    if (ImGui::Button(modeLabel.c_str())) {
        toggleBufferMode(state);
    }
    ImGui::SameLine(0, 10);
    const char* langLabel = state.active() ? state.active()->language.c_str() : "-";
    if (ImGui::Button(langLabel)) {
        ImGui::OpenPopup("##langPopup");
    }
    if (ImGui::BeginPopup("##langPopup")) {
        const char* langs[] = {"python","cpp","elisp","javascript","typescript","java","rust","go","org"};
        const char* labels[] = {"Python","C++","Elisp","JavaScript","TypeScript","Java","Rust","Go","Org"};
        for (int i = 0; i < IM_ARRAYSIZE(langs); ++i) {
            if (ImGui::MenuItem(labels[i])) state.setLanguage(langs[i]);
        }
        ImGui::EndPopup();
    }
    ImGui::SameLine(0, 10);
    if (ImGui::Button("UTF-8")) {
        state.notify(NotificationLevel::Info, "Encoding settings not yet configurable.");
    }
    ImGui::SameLine(0, 10);
    std::string lineEnding = state.active() ? detectLineEnding(state.active()->editBuf) : "-";
    if (ImGui::Button(lineEnding.c_str())) {
        state.notify(NotificationLevel::Info, "Line ending toggle not yet implemented.");
    }

    // Center: notifications
    std::string centerText = "(no notifications)";
    if (const Notification* n = state.notifications.latest()) {
        centerText = n->message;
    }
    float centerWidth = ImGui::CalcTextSize(centerText.c_str()).x;
    float windowWidth = ImGui::GetWindowWidth();
    float centerX = std::max(0.0f, (windowWidth * 0.5f) - (centerWidth * 0.5f));
    ImGui::SetCursorPosX(centerX);
    ImGui::TextUnformatted(centerText.c_str());
    if (ImGui::IsItemClicked()) {
        state.notifications.showHistory = !state.notifications.showHistory;
    }

    // Right cluster: Ln/Col, selection, zoom, git
    int selStart = -1;
    int selEnd = -1;
    int selChars = 0;
    int selLines = 0;
    if (state.active() && state.active()->widget.hasSelectionRange()) {
        state.active()->widget.getSelectionRange(selStart, selEnd);
        if (selStart >= 0 && selEnd > selStart) {
            selChars = selEnd - selStart;
            selLines = 1;
            for (int i = selStart; i < selEnd && i < (int)state.active()->editBuf.size(); ++i) {
                if (state.active()->editBuf[i] == '\n') ++selLines;
            }
        }
    }
    std::string rightText;
    if (state.active()) {
        rightText = "Ln " + std::to_string(state.active()->cursorLine) +
                    ", Col " + std::to_string(state.active()->cursorCol);
    } else {
        rightText = "Ln -, Col -";
    }
    if (selChars > 0) {
        rightText += " | Sel " + std::to_string(selChars) + "c " +
                     std::to_string(selLines) + "l";
    }
    rightText += " | Zoom " + std::to_string(zoomPercent(state.settings.getFontSize(),
                                                         state.baseFontSize)) + "%";
    rightText += " | Git " + detectGitBranch(state.workspaceRoot);

    float rightWidth = ImGui::CalcTextSize(rightText.c_str()).x;
    ImGui::SetCursorPosX(std::max(0.0f, windowWidth - rightWidth - 8.0f));
    ImGui::TextUnformatted(rightText.c_str());

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
