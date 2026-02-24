#pragma once
// Step 1651: Session-level mode override model.
#include <string>
#include <nlohmann/json.hpp>

struct SessionModeOverrideModel {
    std::string sessionId;
    std::string requestedMode;
    bool inheritedWorkspacePolicy = false;
    bool valid = false;
};

class SessionModeOverrideModelFactory {
public:
    static SessionModeOverrideModel make(const std::string& sessionId,
                                         const std::string& requestedMode,
                                         bool inheritedWorkspacePolicy) {
        bool valid = !sessionId.empty()
            && (requestedMode == "text_first" || requestedMode == "ast_first" || requestedMode == "hybrid");
        return {sessionId, requestedMode, inheritedWorkspacePolicy, valid};
    }

    static nlohmann::json toJson(const SessionModeOverrideModel& v) {
        return {{"session_id", v.sessionId},
                {"requested_mode", v.requestedMode},
                {"inherited_workspace_policy", v.inheritedWorkspacePolicy},
                {"valid", v.valid}};
    }
};
