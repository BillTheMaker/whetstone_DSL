#pragma once

#include "imgui.h"

#include <string>
#include <vector>
#include <functional>

enum class NotificationLevel {
    Info,
    Success,
    Warning,
    Error
};

struct NotificationTarget {
    std::string path;
    int line = -1; // zero-based
    int col = -1;  // zero-based
};

struct Notification {
    int id = 0;
    NotificationLevel level = NotificationLevel::Info;
    std::string message;
    double timestamp = 0.0;
    float durationSeconds = 4.0f;
    bool dismissed = false;
    bool read = false;
    bool hasTarget = false;
    NotificationTarget target;
};

class NotificationSystem {
public:
    float toastDurationSeconds = 4.0f;
    bool showHistory = false;

    void notify(NotificationLevel level, const std::string& message) {
        notifyAt(level, message, ImGui::GetTime(), nullptr);
    }

    void notify(NotificationLevel level, const std::string& message, const NotificationTarget& target) {
        notifyAt(level, message, ImGui::GetTime(), &target);
    }

    void notifyAt(NotificationLevel level, const std::string& message, double nowSeconds) {
        notifyAt(level, message, nowSeconds, nullptr);
    }

    void notifyAt(NotificationLevel level,
                  const std::string& message,
                  double nowSeconds,
                  const NotificationTarget* target) {
        Notification n;
        n.id = nextId++;
        n.level = level;
        n.message = message;
        n.timestamp = nowSeconds;
        n.durationSeconds = toastDurationSeconds;
        if (target) {
            n.hasTarget = true;
            n.target = *target;
        }
        history.push_back(std::move(n));
    }

    void dismiss(int id) {
        for (auto& n : history) {
            if (n.id == id) {
                n.dismissed = true;
                n.read = true;
                return;
            }
        }
    }

    void markAllRead() {
        for (auto& n : history) {
            n.read = true;
        }
    }

    int unreadCount() const {
        int count = 0;
        for (const auto& n : history) {
            if (!n.read) ++count;
        }
        return count;
    }

    const std::vector<Notification>& getHistory() const {
        return history;
    }

    void renderToasts(const std::function<void(const Notification&)>& onNavigate = {}) {
        const double now = ImGui::GetTime();
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 base(viewport->WorkPos.x + viewport->WorkSize.x,
                          viewport->WorkPos.y + viewport->WorkSize.y);
        const float padding = 12.0f;
        const float toastWidth = 360.0f;
        float y = base.y - padding;

        for (int i = (int)history.size() - 1; i >= 0; --i) {
            auto& n = history[i];
            if (n.dismissed) continue;
            if ((now - n.timestamp) > n.durationSeconds) continue;

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoSavedSettings;

            ImGui::SetNextWindowPos(ImVec2(base.x - padding, y), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
            ImGui::SetNextWindowSize(ImVec2(toastWidth, 0.0f), ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 10));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 0.92f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.22f, 0.22f, 0.22f, 0.9f));
            ImGui::Begin(("##toast_" + std::to_string(n.id)).c_str(), nullptr, flags);

            ImVec4 color = levelColor(n.level);
            ImGui::TextColored(color, "%s", levelLabel(n.level));
            ImGui::SameLine(0.0f, 8.0f);
            ImGui::TextWrapped("%s", n.message.c_str());
            if (n.hasTarget) {
                std::string loc = n.target.path;
                if (n.target.line >= 0) {
                    loc += ":" + std::to_string(n.target.line + 1);
                    if (n.target.col >= 0) loc += ":" + std::to_string(n.target.col + 1);
                }
                ImGui::TextDisabled("%s", loc.c_str());
            }

            bool clicked = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
                           ImGui::IsMouseClicked(0);
            ImVec2 size = ImGui::GetWindowSize();
            ImGui::End();
            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar();

            if (clicked) {
                n.read = true;
                n.dismissed = true;
                if (n.hasTarget && onNavigate) {
                    onNavigate(n);
                }
            }

            y -= size.y + 8.0f;
        }
    }

    void renderHistoryWindow(const std::function<void(const Notification&)>& onNavigate = {}) {
        if (!showHistory) {
            historyWasOpen = false;
            return;
        }

        if (!historyWasOpen) {
            markAllRead();
            historyWasOpen = true;
        }

        ImGui::SetNextWindowSize(ImVec2(520, 360), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Notifications", &showHistory)) {
            ImGui::End();
            return;
        }

        if (history.empty()) {
            ImGui::TextDisabled("(no notifications yet)");
            ImGui::End();
            return;
        }

        ImGui::BeginChild("##notificationHistory", ImVec2(0, 0), false);
        for (const auto& n : history) {
            ImGui::PushID(n.id);
            ImVec4 color = levelColor(n.level);
            ImGui::TextColored(color, "%s", levelLabel(n.level));
            ImGui::SameLine();
            if (ImGui::Selectable(n.message.c_str(), false)) {
                if (n.hasTarget && onNavigate) {
                    onNavigate(n);
                }
            }
            if (n.hasTarget) {
                std::string loc = n.target.path;
                if (n.target.line >= 0) {
                    loc += ":" + std::to_string(n.target.line + 1);
                    if (n.target.col >= 0) loc += ":" + std::to_string(n.target.col + 1);
                }
                ImGui::TextDisabled("%s", loc.c_str());
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::End();
    }

private:
    int nextId = 1;
    bool historyWasOpen = false;
    std::vector<Notification> history;

    static const char* levelLabel(NotificationLevel level) {
        switch (level) {
        case NotificationLevel::Success: return "Success";
        case NotificationLevel::Warning: return "Warning";
        case NotificationLevel::Error: return "Error";
        case NotificationLevel::Info:
        default:
            return "Info";
        }
    }

    static ImVec4 levelColor(NotificationLevel level) {
        switch (level) {
        case NotificationLevel::Success: return ImVec4(0.25f, 0.78f, 0.35f, 1.0f);
        case NotificationLevel::Warning: return ImVec4(0.95f, 0.72f, 0.28f, 1.0f);
        case NotificationLevel::Error: return ImVec4(0.90f, 0.35f, 0.35f, 1.0f);
        case NotificationLevel::Info:
        default:
            return ImVec4(0.40f, 0.70f, 0.95f, 1.0f);
        }
    }
};
