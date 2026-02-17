#pragma once
// Step 521: Notification + Status Signaling Refresh

#include <string>
#include <vector>
#include <algorithm>

enum class NotifyLevel {
    Info,
    Warn,
    Error,
    Critical
};

struct NotificationStyle {
    float borderAlpha = 0.0f;
    float textContrast = 1.0f;
    float iconContrast = 1.0f;
    float defaultDurationMs = 0.0f;
    int maxStack = 5;
};

struct NotificationMessage {
    NotifyLevel level = NotifyLevel::Info;
    std::string text;
    float createdMs = 0.0f;
    float durationMs = 0.0f;
    bool dismissed = false;
};

class NotificationStatusRefresh {
public:
    static NotificationStyle styleFor(NotifyLevel level) {
        switch (level) {
            case NotifyLevel::Info: return {0.28f, 5.4f, 3.8f, 2400.0f, 5};
            case NotifyLevel::Warn: return {0.34f, 5.6f, 4.0f, 3200.0f, 5};
            case NotifyLevel::Error: return {0.42f, 6.0f, 4.5f, 4200.0f, 6};
            case NotifyLevel::Critical: return {0.52f, 7.0f, 5.0f, 0.0f, 8};
        }
        return {0.3f, 5.0f, 3.5f, 2400.0f, 5};
    }

    static bool shouldExpire(const NotificationMessage& m, float nowMs) {
        if (m.dismissed) return true;
        if (m.durationMs <= 0.0f) return false; // persistent critical
        return nowMs - m.createdMs >= m.durationMs;
    }

    static std::vector<NotificationMessage> applyStackPolicy(
            const std::vector<NotificationMessage>& in,
            int maxStack) {
        if ((int)in.size() <= maxStack) return in;
        std::vector<NotificationMessage> out = in;
        out.erase(out.begin(), out.begin() + ((int)out.size() - maxStack));
        return out;
    }

    static NotificationMessage makeMessage(NotifyLevel level,
                                           const std::string& text,
                                           float nowMs) {
        NotificationMessage m;
        m.level = level;
        m.text = text;
        m.createdMs = nowMs;
        m.durationMs = styleFor(level).defaultDurationMs;
        return m;
    }

    static bool readable(const NotificationStyle& s) {
        return s.textContrast >= 4.5f && s.iconContrast >= 3.0f;
    }
};
