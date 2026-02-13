#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "SemannoFormat.h"

struct SemannoSidecarSaveResult {
    bool success = false;
    std::string path;
    int annotationCount = 0;
    std::string error;
};

struct SemannoSidecarLoadResult {
    bool success = false;
    std::vector<SemannoEntry> entries;
    int count = 0;
    std::string error;
};

class SemannoSidecar {
public:
    // Generates the path for a .semanno sidecar file.
    // e.g., /path/to/project/src/file.py -> /path/to/project/.whetstone/src/file.py.semanno
    static std::filesystem::path getSidecarPath(const std::filesystem::path& workspaceRoot, const std::filesystem::path& filePath) {
        std::filesystem::path relativePath;
        if (filePath.is_absolute()) {
            std::error_code ec;
            relativePath = std::filesystem::relative(filePath, workspaceRoot, ec);
            if (ec || relativePath.empty()) {
                relativePath = filePath.filename();
            }
        } else {
            relativePath = filePath;
        }

        std::filesystem::path sidecarDir = workspaceRoot / ".whetstone";
        return sidecarDir / (relativePath.generic_string() + ".semanno");
    }

    // Saves all semantic annotations from an AST module to a .semanno sidecar file.
    static SemannoSidecarSaveResult saveSemannoSidecar(const std::string& workspaceRoot, const std::string& filePath, const Module* module) {
        SemannoSidecarSaveResult result;
        if (!module) {
            result.error = "No AST to save";
            return result;
        }

        std::vector<std::pair<int, std::string>> entries;
        collectEntries(module, entries);
        result.annotationCount = static_cast<int>(entries.size());

        auto path = getSidecarPath(workspaceRoot, filePath);
        result.path = path.string();
        if (entries.empty()) {
            try {
                std::filesystem::create_directories(path.parent_path());
                std::ofstream file(path);
                if (!file.is_open()) {
                    result.error = "Cannot write to " + path.string();
                    return result;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                result.error = e.what();
                return result;
            }
            result.success = true;
            return result;
        }

        // Sort by line number to ensure a stable format
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        try {
            std::filesystem::create_directories(path.parent_path());

            std::ofstream file(path);
            if (!file.is_open()) {
                result.error = "Cannot write to " + path.string();
                return result;
            }

            for (const auto& entry : entries) {
                file << "L" << entry.first << ": " << entry.second << "\n";
            }
            result.success = true;
            return result;
        } catch (const std::filesystem::filesystem_error& e) {
            result.error = e.what();
            return result;
        }
    }

    // Loads semantic annotations from a .semanno sidecar file.
    static SemannoSidecarLoadResult loadSemannoSidecar(const std::string& workspaceRoot, const std::string& filePath) {
        SemannoSidecarLoadResult result;
        auto path = getSidecarPath(workspaceRoot, filePath);

        std::ifstream file(path);
        if (!file.is_open()) {
            result.error = "No sidecar file: " + path.string();
            return result;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] != 'L') continue;

            auto semanno = SemannoParser::parse(line);
            if (!semanno.type.empty()) {
                result.entries.push_back(semanno);
            }
        }
        result.count = static_cast<int>(result.entries.size());
        result.success = true;
        return result;
    }

private:
    // Helper to recursively traverse the AST and collect annotation entries.
    static void collectEntries(const ASTNode* node, std::vector<std::pair<int, std::string>>& entries) {
        if (!node) return;

        // An annotation's line number is determined by its parent node
        if (node->parent && node->parent->spanStartLine >= 0) {
            std::string semannoString = SemannoEmitter::emit(node);
            if (!semannoString.empty()) {
                // Link annotation comments to their parent source line in the sidecar.
                entries.push_back({node->parent->spanStartLine, semannoString});
            }
        }

        // Recurse through all children
        for (const auto* child : node->allChildren()) {
            collectEntries(child, entries);
        }
    }
};

inline std::string semannoSidecarPath(const std::string& workspaceRoot,
                                      const std::string& filePath) {
    return SemannoSidecar::getSidecarPath(workspaceRoot, filePath).string();
}

inline SemannoSidecarSaveResult saveSemannoSidecar(const std::string& workspaceRoot,
                                                   const std::string& filePath,
                                                   const Module* module) {
    return SemannoSidecar::saveSemannoSidecar(workspaceRoot, filePath, module);
}

inline SemannoSidecarLoadResult loadSemannoSidecar(const std::string& workspaceRoot,
                                                   const std::string& filePath) {
    return SemannoSidecar::loadSemannoSidecar(workspaceRoot, filePath);
}
