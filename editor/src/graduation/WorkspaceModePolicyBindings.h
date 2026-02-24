#pragma once
// Step 1650: Workspace mode policy bindings.
#include <string>
#include <nlohmann/json.hpp>

struct WorkspaceModePolicyBindings {
    std::string workspaceId;
    std::string defaultMode;
    bool locked = false;
    bool valid = false;
};

class WorkspaceModePolicyBindingsFactory {
public:
    static WorkspaceModePolicyBindings make(const std::string& workspaceId,
                                            const std::string& defaultMode,
                                            bool locked) {
        bool valid = !workspaceId.empty()
            && (defaultMode == "text_first" || defaultMode == "ast_first" || defaultMode == "hybrid");
        return {workspaceId, defaultMode, locked, valid};
    }

    static nlohmann::json toJson(const WorkspaceModePolicyBindings& v) {
        return {{"workspace_id", v.workspaceId},
                {"default_mode", v.defaultMode},
                {"locked", v.locked},
                {"valid", v.valid}};
    }
};
