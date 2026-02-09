#pragma once
// Step 116: Project save/load helpers

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "ast/Serialization.h"
#include "ast/Module.h"

struct ProjectBufferInfo {
    std::string path;
    std::string language;
    std::string mode;
};

struct ProjectFile {
    std::string workspaceRoot;
    std::string activePath;
    std::vector<ProjectBufferInfo> buffers;
    std::unique_ptr<Module> ast;
};

inline nlohmann::json projectToJson(const ProjectFile& project) {
    nlohmann::json j;
    j["workspaceRoot"] = project.workspaceRoot;
    j["activePath"] = project.activePath;
    j["buffers"] = nlohmann::json::array();
    for (const auto& b : project.buffers) {
        j["buffers"].push_back({
            {"path", b.path},
            {"language", b.language},
            {"mode", b.mode}
        });
    }
    if (project.ast) {
        j["ast"] = toJson(project.ast.get());
    }
    return j;
}

inline ProjectFile projectFromJson(const nlohmann::json& j) {
    ProjectFile project;
    project.workspaceRoot = j.value("workspaceRoot", "");
    project.activePath = j.value("activePath", "");
    if (j.contains("buffers") && j["buffers"].is_array()) {
        for (const auto& item : j["buffers"]) {
            ProjectBufferInfo info;
            info.path = item.value("path", "");
            info.language = item.value("language", "");
            info.mode = item.value("mode", "structured");
            if (!info.path.empty()) project.buffers.push_back(std::move(info));
        }
    }
    if (j.contains("ast")) {
        ASTNode* node = fromJson(j["ast"]);
        if (node && node->conceptType == "Module") {
            project.ast.reset(static_cast<Module*>(node));
        } else {
            deleteTree(node);
        }
    }
    return project;
}
