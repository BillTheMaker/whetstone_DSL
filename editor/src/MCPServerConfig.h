#pragma once

#include "MCPServer.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class MCPServerConfig {
public:
    struct Discovery {
        std::string binaryPath;
        std::string workspaceRoot;
        std::string primaryLanguage;
        int toolCount = 0;
        std::map<std::string, int> categoryCounts;
    };

    static std::string resolveBinaryPath(
            const std::vector<std::string>& candidates = {},
            const std::string& pathEnv = "") {
        for (const auto& c : candidates) {
            if (isExecutable(c) || fileExists(c)) return std::filesystem::absolute(c).string();
        }

        const std::vector<std::string> defaults = {
            "editor/build-native/whetstone_mcp",
            "editor/build-native/whetstone_mcp.exe",
            "build-native/whetstone_mcp",
            "build-native/whetstone_mcp.exe",
            "whetstone_mcp"
        };
        for (const auto& c : defaults) {
            if (isExecutable(c) || fileExists(c)) return std::filesystem::absolute(c).string();
        }

        std::string pathValue = pathEnv.empty() ? getenvString("PATH") : pathEnv;
        auto pathEntries = splitPath(pathValue);
        for (const auto& entry : pathEntries) {
            std::filesystem::path full = std::filesystem::path(entry) / "whetstone_mcp";
            if (isExecutable(full.string()) || fileExists(full.string())) return full.string();
            std::filesystem::path fullExe = std::filesystem::path(entry) / "whetstone_mcp.exe";
            if (isExecutable(fullExe.string()) || fileExists(fullExe.string())) return fullExe.string();
        }
        return "whetstone_mcp";
    }

    static std::string detectWorkspaceRoot(const std::string& startDir) {
        if (startDir.empty()) return "";
        std::filesystem::path cur = std::filesystem::absolute(startDir);
        while (true) {
            if (std::filesystem::exists(cur / ".git") ||
                std::filesystem::exists(cur / "CMakeLists.txt") ||
                std::filesystem::exists(cur / "progress.md")) {
                return cur.string();
            }
            if (cur == cur.root_path()) break;
            cur = cur.parent_path();
        }
        return std::filesystem::absolute(startDir).string();
    }

    static std::string detectPrimaryLanguage(
            const std::string& workspaceRoot,
            size_t maxFilesToScan = 4000) {
        if (workspaceRoot.empty() || !std::filesystem::exists(workspaceRoot)) return "cpp";

        std::unordered_map<std::string, int> extCount;
        size_t scanned = 0;
        for (const auto& e : std::filesystem::recursive_directory_iterator(workspaceRoot)) {
            if (scanned >= maxFilesToScan) break;
            if (!e.is_regular_file()) continue;
            auto ext = e.path().extension().string();
            if (ext.empty()) continue;
            extCount[toLower(ext)]++;
            scanned++;
        }

        if (extCount.empty()) return "cpp";

        const std::vector<std::pair<std::string, std::string>> mapping = {
            {".py", "python"}, {".cpp", "cpp"}, {".cc", "cpp"}, {".cxx", "cpp"},
            {".cs", "csharp"}, {".fs", "fsharp"}, {".vb", "vbnet"},
            {".sql", "postgresql"}, {".js", "javascript"}, {".ts", "typescript"},
            {".java", "java"}, {".rs", "rust"}, {".go", "go"}, {".c", "c"}
        };

        std::string bestLang = "cpp";
        int bestCount = -1;
        for (const auto& [ext, lang] : mapping) {
            auto it = extCount.find(ext);
            if (it != extCount.end() && it->second > bestCount) {
                bestCount = it->second;
                bestLang = lang;
            }
        }
        return bestLang;
    }

    static json buildMcpConfig(
            const std::string& binaryPath,
            const std::string& workspaceRoot,
            const std::string& language) {
        json args = json::array();
        if (!workspaceRoot.empty()) {
            args.push_back("--workspace");
            args.push_back(workspaceRoot);
        }
        if (!language.empty()) {
            args.push_back("--language");
            args.push_back(language);
        }

        return {
            {"mcpServers", {
                {"whetstone", {
                    {"command", binaryPath},
                    {"args", args}
                }}
            }}
        };
    }

    static bool writeMcpConfigFile(const std::string& outputPath, const json& config) {
        std::ofstream out(outputPath);
        if (!out.is_open()) return false;
        out << config.dump(2) << "\n";
        return true;
    }

    static json buildServerManifest(const MCPServer& server) {
        std::map<std::string, std::vector<json>> categories;
        for (const auto& t : server.getTools()) {
            std::string cat = categorizeTool(t.name);
            categories[cat].push_back({
                {"name", t.name},
                {"description", t.description}
            });
        }

        json byCategory = json::object();
        for (const auto& [k, v] : categories) byCategory[k] = v;

        return {
            {"toolCount", (int)server.getTools().size()},
            {"categories", byCategory}
        };
    }

    static Discovery discover(const std::string& startDir) {
        Discovery d;
        d.binaryPath = resolveBinaryPath();
        d.workspaceRoot = detectWorkspaceRoot(startDir);
        d.primaryLanguage = detectPrimaryLanguage(d.workspaceRoot);

        MCPServer server;
        d.toolCount = (int)server.getTools().size();
        auto manifest = buildServerManifest(server);
        if (manifest.contains("categories")) {
            for (auto it = manifest["categories"].begin(); it != manifest["categories"].end(); ++it) {
                d.categoryCounts[it.key()] = (int)it.value().size();
            }
        }
        return d;
    }

    static std::string categorizeTool(const std::string& toolName) {
        const std::string n = toLower(toolName);
        if (n.find("whetstone_get_ast") == 0 || n.find("batch_mutate") != std::string::npos ||
            n.find("call_hierarchy") != std::string::npos || n.find("_mutate") != std::string::npos) {
            return "AST";
        }
        if (n.find("diagnostic") != std::string::npos || n.find("quick_fix") != std::string::npos) {
            return "Diagnostics";
        }
        if (n.find("workflow") != std::string::npos || n.find("workitem") != std::string::npos ||
            n.find("review") != std::string::npos) {
            return "Workflow";
        }
        if (n.find("route") != std::string::npos || n.find("orchestrate") != std::string::npos) {
            return "Routing";
        }
        if (n.find("file_") != std::string::npos || n.find("save_") != std::string::npos) {
            return "FileOps";
        }
        if (n.find("workspace") != std::string::npos || n.find("open_file") != std::string::npos ||
            n.find("close_file") != std::string::npos || n.find("buffer") != std::string::npos ||
            n.find("project") != std::string::npos) {
            return "Project";
        }
        return "Misc";
    }

private:
    static std::string getenvString(const std::string& key) {
        const char* v = std::getenv(key.c_str());
        return v ? std::string(v) : "";
    }

    static bool isExecutable(const std::string& path) {
        if (path.empty()) return false;
        std::error_code ec;
        auto p = std::filesystem::path(path);
        if (!std::filesystem::exists(p, ec) || std::filesystem::is_directory(p, ec)) return false;
        auto perms = std::filesystem::status(p, ec).permissions();
        if (ec) return false;
        using perms_t = std::filesystem::perms;
        return (perms & perms_t::owner_exec) != perms_t::none ||
               (perms & perms_t::group_exec) != perms_t::none ||
               (perms & perms_t::others_exec) != perms_t::none;
    }

    static bool fileExists(const std::string& path) {
        if (path.empty()) return false;
        std::error_code ec;
        auto p = std::filesystem::path(path);
        return std::filesystem::exists(p, ec) && !std::filesystem::is_directory(p, ec);
    }

    static std::vector<std::string> splitPath(const std::string& path) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : path) {
            if (c == ':') {
                if (!cur.empty()) out.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) out.push_back(cur);
        return out;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return out;
    }
};
