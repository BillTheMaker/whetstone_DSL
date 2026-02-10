#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

enum class UIEventType {
    ASTChanged,
    BufferSwitched,
    DiagnosticsUpdated,
    ThemeChanged,
    SettingsChanged,
    FileModified,
    AgentConnected,
    NotificationPosted
};

struct UIEvent {
    UIEventType type = UIEventType::ASTChanged;
    std::string path;
    std::string detail;
    double timestamp = 0.0;
};

class UIEventBus {
public:
    using Handler = std::function<void(const UIEvent&)>;

    int subscribe(UIEventType type, Handler handler) {
        int id = ++nextId;
        handlers[keyFor(type)].push_back({id, std::move(handler)});
        return id;
    }

    void publish(UIEventType type,
                 const std::string& path = {},
                 const std::string& detail = {},
                 double nowSeconds = 0.0) {
        UIEvent ev;
        ev.type = type;
        ev.path = path;
        ev.detail = detail;
        ev.timestamp = nowSeconds;
        dispatch(ev);
    }

    void publishDebounced(UIEventType type,
                          const std::string& path,
                          const std::string& detail,
                          double nowSeconds,
                          double delaySeconds) {
        PendingEvent& pendingEvent = pending[keyFor(type)];
        pendingEvent.event.type = type;
        pendingEvent.event.path = path;
        pendingEvent.event.detail = detail;
        pendingEvent.event.timestamp = nowSeconds;
        pendingEvent.readyAt = nowSeconds + delaySeconds;
    }

    void tick(double nowSeconds) {
        std::vector<int> ready;
        ready.reserve(pending.size());
        for (const auto& kv : pending) {
            if (nowSeconds >= kv.second.readyAt) {
                ready.push_back(kv.first);
            }
        }
        for (int key : ready) {
            auto it = pending.find(key);
            if (it == pending.end()) continue;
            dispatch(it->second.event);
            pending.erase(it);
        }
    }

private:
    struct HandlerEntry {
        int id = 0;
        Handler fn;
    };

    struct PendingEvent {
        UIEvent event;
        double readyAt = 0.0;
    };

    int nextId = 0;
    std::unordered_map<int, std::vector<HandlerEntry>> handlers;
    std::unordered_map<int, PendingEvent> pending;

    static int keyFor(UIEventType type) {
        return static_cast<int>(type);
    }

    void dispatch(const UIEvent& ev) {
        auto it = handlers.find(keyFor(ev.type));
        if (it == handlers.end()) return;
        for (const auto& entry : it->second) {
            if (entry.fn) entry.fn(ev);
        }
    }
};
