#pragma once
// Step 1541: debug session handoff model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugSessionHandoff {
    std::string sessionId;
    std::string summary;
    std::vector<std::string> nextActions;
};

class DebugSessionHandoffModel {
public:
    static DebugSessionHandoff build(const std::string& sessionId,
                                     const std::string& summary,
                                     const std::vector<std::string>& nextActions) {
        return {sessionId, summary, nextActions};
    }

    static nlohmann::json toJson(const DebugSessionHandoff& h) {
        return {{"session_id", h.sessionId}, {"summary", h.summary}, {"next_actions", h.nextActions}};
    }
};
