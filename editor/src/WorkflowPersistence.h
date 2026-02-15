#pragma once
// Step 323: Workflow Sidecar Persistence
//
// Save and load workflow state to .whetstone/<project>.workflow.json
// so workflows survive across sessions.

#include "WorkflowState.h"
#include <string>
#include <optional>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// --- SaveResult ---

struct SaveResult {
    bool success = false;
    std::string path;
    int bytesWritten = 0;
    int itemCount = 0;
};

// --- Path helper ---

inline std::string workflowSidecarPath(const std::string& workspaceRoot,
                                        const std::string& projectName) {
    return workspaceRoot + "/.whetstone/" + projectName + ".workflow.json";
}

// --- Create parent directories recursively ---

inline bool ensureDirectoryExists(const std::string& path) {
    // Find last slash to get directory
    auto pos = path.rfind('/');
    if (pos == std::string::npos) return true;
    std::string dir = path.substr(0, pos);

    // Simple recursive mkdir
    std::string current;
    for (size_t i = 0; i < dir.size(); ++i) {
        current += dir[i];
        if (dir[i] == '/' || i == dir.size() - 1) {
            mkdir(current.c_str(), 0755);
        }
    }
    // Check if directory exists
    struct stat st{};
    return stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

// --- Save workflow ---

inline SaveResult saveWorkflow(const std::string& workspaceRoot,
                                const WorkflowState& state) {
    SaveResult result;
    result.path = workflowSidecarPath(workspaceRoot, state.projectName);

    if (!ensureDirectoryExists(result.path)) {
        return result;
    }

    json j = state.toJson();
    std::string content = j.dump(2);

    std::ofstream ofs(result.path);
    if (!ofs.is_open()) return result;

    ofs << content;
    ofs.close();

    if (ofs.fail()) return result;

    result.success = true;
    result.bytesWritten = static_cast<int>(content.size());
    result.itemCount = state.queue.size();
    return result;
}

// --- Load workflow ---

inline std::optional<WorkflowState> loadWorkflow(const std::string& workspaceRoot,
                                                  const std::string& projectName) {
    std::string path = workflowSidecarPath(workspaceRoot, projectName);

    std::ifstream ifs(path);
    if (!ifs.is_open()) return std::nullopt;

    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string content = oss.str();

    if (content.empty()) return std::nullopt;

    try {
        json j = json::parse(content);
        return WorkflowState::fromJson(j);
    } catch (...) {
        return std::nullopt;
    }
}

// --- Delete workflow ---

inline bool deleteWorkflow(const std::string& workspaceRoot,
                            const std::string& projectName) {
    std::string path = workflowSidecarPath(workspaceRoot, projectName);
    return std::remove(path.c_str()) == 0;
}
