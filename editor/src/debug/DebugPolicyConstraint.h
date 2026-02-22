#pragma once
// Step 1529: debug policy constraint model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugPolicyConstraint {
    std::string mode = "safe";
    int maxFilesTouched = 3;
    int maxLineChanges = 40;
    bool requireTests = true;
    std::vector<std::string> forbiddenPaths;
};

class DebugPolicyConstraintModel {
public:
    static DebugPolicyConstraint forMode(const std::string& mode) {
        DebugPolicyConstraint c;
        c.mode = mode;
        if (mode == "strict") {
            c.maxFilesTouched = 2;
            c.maxLineChanges = 25;
            c.requireTests = true;
            c.forbiddenPaths = {"editor/src/mcp/", "editor/src/MCPServer.h"};
        } else if (mode == "normal") {
            c.maxFilesTouched = 4;
            c.maxLineChanges = 60;
            c.requireTests = true;
            c.forbiddenPaths = {"editor/src/mcp/"};
        } else {
            c.maxFilesTouched = 3;
            c.maxLineChanges = 40;
            c.requireTests = true;
            c.forbiddenPaths = {"editor/src/mcp/"};
        }
        return c;
    }

    static nlohmann::json toJson(const DebugPolicyConstraint& c) {
        return {
            {"mode", c.mode},
            {"max_files_touched", c.maxFilesTouched},
            {"max_line_changes", c.maxLineChanges},
            {"require_tests", c.requireTests},
            {"forbidden_paths", c.forbiddenPaths}
        };
    }
};
