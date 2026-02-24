#pragma once
// Step 1649: Authoring mode schema and state model.
#include <string>
#include <nlohmann/json.hpp>

struct AuthoringModeStateModel {
    std::string workspaceId;
    std::string mode;
    bool astProjectionEnabled = true;
    bool valid = false;
};

class AuthoringModeStateModelFactory {
public:
    static AuthoringModeStateModel make(const std::string& workspaceId,
                                        const std::string& mode,
                                        bool astProjectionEnabled) {
        bool valid = !workspaceId.empty()
            && (mode == "text_first" || mode == "ast_first" || mode == "hybrid");
        return {workspaceId, mode, astProjectionEnabled, valid};
    }

    static nlohmann::json toJson(const AuthoringModeStateModel& v) {
        return {{"workspace_id", v.workspaceId},
                {"mode", v.mode},
                {"ast_projection_enabled", v.astProjectionEnabled},
                {"valid", v.valid}};
    }
};
