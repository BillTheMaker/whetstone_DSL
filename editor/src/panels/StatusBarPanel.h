#pragma once

#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../AgentTaskStatusOverlay.h"
#include "../ThemeEngine.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <psapi.h>
#include <windows.h>
#endif

static std::string detectGitBranch(const std::string& root) {
    if (root.empty()) return "-";
    std::filesystem::path headPath = std::filesystem::path(root) / ".git" / "HEAD";
    std::ifstream in(headPath.string());
    if (!in.is_open()) return "-";

    std::string line;
    std::getline(in, line);
    const std::string refPrefix = "ref: refs/heads/";
    if (line.rfind(refPrefix, 0) == 0) return line.substr(refPrefix.size());
    if (!line.empty()) return line.substr(0, 8);
    return "-";
}

static void toggleBufferMode(EditorState& state) {
    if (!state.active()) return;
    BufferManager::BufferMode next =
        state.active()->bufferMode == BufferManager::BufferMode::Text
            ? BufferManager::BufferMode::Structured
            : BufferManager::BufferMode::Text;
    state.setActiveBufferMode(next);
}

static std::string detectLineEnding(const std::string& text) {
    if (text.find("\r\n") != std::string::npos) return "CRLF";
    return "LF";
}

static size_t processMemoryBytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(),
                             reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
                             sizeof(pmc))) {
        return static_cast<size_t>(pmc.WorkingSetSize);
    }
#endif
    return 0;
}

static std::string formatMemory(size_t bytes) {
    if (bytes == 0) return "-";
    const double mb = 1024.0 * 1024.0;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f MB", bytes / mb);
    return buf;
}

static ImVec4 statusBarBackgroundColor(const EditorState& state) {
    bool hasError = false;
    for (const auto& d : state.whetstoneDiagnostics) {
        if (d.severity == 1) {
            hasError = true;
            break;
        }
    }
    if (!hasError) {
        for (const auto& d : state.emacsDiagnostics) {
            if (d.severity == 1) {
                hasError = true;
                break;
            }
        }
    }

    if (state.build.runInProgress) return ImVec4(0.10f, 0.35f, 0.25f, 1.0f);
    if (hasError) return ImVec4(0.45f, 0.12f, 0.12f, 1.0f);
    return ImVec4(0.10f, 0.12f, 0.16f, 1.0f);
}

static void renderStatusBarLeftCluster(EditorState& state) {
    std::string modeLabel = state.active()
        ? (state.active()->bufferMode == BufferManager::BufferMode::Text ? "Text Mode" : "Structured Mode")
        : "-";
    if (ImGui::Button(modeLabel.c_str())) toggleBufferMode(state);

    if (state.active() &&
        state.active()->bufferMode == BufferManager::BufferMode::Text &&
        state.active()->fileSizeBytes > 0) {
        ImGui::SameLine(0, 6);
        if (ImGui::Button("Reopen Structured")) {
            state.setActiveBufferMode(BufferManager::BufferMode::Structured);
        }
    }

    ImGui::SameLine(0, 10);
    const char* langLabel = state.active() ? state.active()->language.c_str() : "-";
    if (ImGui::Button(langLabel)) ImGui::OpenPopup("##langPopup");
    if (ImGui::BeginPopup("##langPopup")) {
        const char* langs[] = {"python", "cpp", "elisp", "javascript", "typescript", "java", "rust", "go", "org"};
        const char* labels[] = {"Python", "C++", "Elisp", "JavaScript", "TypeScript", "Java", "Rust", "Go", "Org"};
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
}

static void renderStatusBarCenter(EditorState& state, float windowWidth) {
    std::string centerText = "(no notifications)";
    if (const Notification* n = state.notifications.latest()) centerText = n->message;

    float centerWidth = ImGui::CalcTextSize(centerText.c_str()).x;
    float centerX = std::max(0.0f, (windowWidth * 0.5f) - (centerWidth * 0.5f));
    ImGui::SetCursorPosX(centerX);
    ImGui::TextUnformatted(centerText.c_str());
    if (ImGui::IsItemClicked()) {
        state.notifications.showHistory = !state.notifications.showHistory;
    }
}

static void renderRecordingAndAgentIndicator(EditorState& state) {
    ImGui::SameLine(0, 10);
    const bool recording = state.agent.workflowRecorder.isRecording();
    if (recording) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.20f, 0.20f, 1.0f));
    }
    if (ImGui::Button("REC")) state.toggleSessionRecording();
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(recording ? "Session recording active" : "Start session recording");
    }
    if (recording) ImGui::PopStyleColor(2);

    AgentTaskStatusSummary taskSummary =
        AgentTaskStatusOverlay::buildSummary(state.agent.taskSlots);
    if (!taskSummary.hasWork) return;

    ImGui::SameLine(0, 10);
    if (ImGui::Button(taskSummary.label.c_str())) {
        state.ui.showAgentChatPanel = true;
    }
}

static std::string buildStatusBarRightText(EditorState& state) {
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

    std::string rightText = state.active()
        ? ("Ln " + std::to_string(state.active()->cursorLine) +
           ", Col " + std::to_string(state.active()->cursorCol))
        : "Ln -, Col -";
    if (selChars > 0) {
        rightText += " | Sel " + std::to_string(selChars) + "c " + std::to_string(selLines) + "l";
    }
    if (state.active() && state.active()->largeFileMode) rightText += " | Large File";

    size_t memBytes = processMemoryBytes();
    if (memBytes > 0) rightText += " | Mem " + formatMemory(memBytes);

    rightText += " | Zoom " + std::to_string(zoomPercent(state.settings.getFontSize(),
                                                          state.baseFontSize)) + "%";
    rightText += " | Git " + detectGitBranch(state.workspaceRoot);
    return rightText;
}

static void renderStatusBarRight(EditorState& state, float windowWidth) {
    std::string rightText = buildStatusBarRightText(state);
    float rightWidth = ImGui::CalcTextSize(rightText.c_str()).x;
    ImGui::SetCursorPosX(std::max(0.0f, windowWidth - rightWidth - 8.0f));
    ImGui::TextUnformatted(rightText.c_str());
}

static void renderStatusBar(EditorState& state) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiWindowFlags sbFlags = ImGuiWindowFlags_NoDecoration |
                               ImGuiWindowFlags_NoMove |
                               ImGuiWindowFlags_NoScrollbar |
                               ImGuiWindowFlags_NoSavedSettings;
    const float sbHeight = ImGui::GetFrameHeight() + 2;
    const ImVec2 sbPos(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - sbHeight);
    const ImVec2 sbSize(viewport->WorkSize.x, sbHeight);

    ImGui::SetNextWindowPos(sbPos);
    ImGui::SetNextWindowSize(sbSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, statusBarBackgroundColor(state));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::Begin("##StatusBar", nullptr, sbFlags);
    ImGui::PushFont(state.uiFont);

    const float windowWidth = ImGui::GetWindowWidth();
    renderStatusBarLeftCluster(state);
    renderStatusBarCenter(state, windowWidth);
    renderRecordingAndAgentIndicator(state);
    renderStatusBarRight(state, windowWidth);

    ImGui::PopFont();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}
