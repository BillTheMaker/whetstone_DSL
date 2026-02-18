#pragma once
// Step 619: Per-project MCP config (.whetstone.json)

#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct MCPProjectConfigData {
    std::string workspace;
    std::string defaultLanguage;
    std::string agentRole;
    std::string mcpWorkspaceAlias;
};

struct MCPProjectConfigLoadResult {
    bool found = false;
    std::string sourceWorkspaceRoot;
    MCPProjectConfigData config;
    std::string error;
};

class MCPProjectConfig {
public:
    static bool parseText(const std::string& text,
                          MCPProjectConfigData* out,
                          std::string* error) {
        if (!out || !error) return false;
        error->clear();
        if (text.empty()) {
            *error = "config_empty";
            return false;
        }

        json parsed;
        try {
            parsed = json::parse(text);
        } catch (...) {
            *error = "config_json_parse_failed";
            return false;
        }
        return parseJson(parsed, out, error);
    }

    static bool parseJson(const json& parsed,
                          MCPProjectConfigData* out,
                          std::string* error) {
        if (!out || !error) return false;
        error->clear();
        if (!parsed.is_object()) {
            *error = "config_not_object";
            return false;
        }

        MCPProjectConfigData data;
        if (!readOptionalString(parsed, "workspace", &data.workspace, error)) return false;
        if (!readOptionalString(parsed, "defaultLanguage", &data.defaultLanguage, error)) return false;
        if (!readOptionalString(parsed, "agentRole", &data.agentRole, error)) return false;
        if (!readOptionalString(parsed, "mcpWorkspaceAlias", &data.mcpWorkspaceAlias, error)) return false;
        *out = data;
        return true;
    }

    static MCPProjectConfigLoadResult loadFromWorkspace(const std::string& workspaceRoot) {
        MCPProjectConfigLoadResult out;
        if (workspaceRoot.empty()) {
            out.error = "workspace_missing";
            return out;
        }
        out.sourceWorkspaceRoot = workspaceRoot;

        std::filesystem::path path = std::filesystem::path(workspaceRoot) / ".whetstone.json";
        if (!std::filesystem::exists(path)) return out;
        out.found = true;

        std::ifstream in(path);
        if (!in.is_open()) {
            out.error = "config_open_failed";
            return out;
        }

        std::string text((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
        if (!parseText(text, &out.config, &out.error)) return out;
        return out;
    }

    static MCPProjectConfigLoadResult discoverFromCwd(const std::string& startDir = "") {
        MCPProjectConfigLoadResult out;
        std::filesystem::path current = startDir.empty()
            ? std::filesystem::current_path()
            : std::filesystem::path(startDir);
        if (!std::filesystem::exists(current)) {
            out.error = "start_dir_not_found";
            return out;
        }

        current = std::filesystem::weakly_canonical(current);
        while (true) {
            std::filesystem::path candidate = current / ".whetstone.json";
            if (std::filesystem::exists(candidate)) {
                return loadFromWorkspace(current.string());
            }
            if (current == current.root_path()) break;
            std::filesystem::path parent = current.parent_path();
            if (parent == current) break;
            current = parent;
        }
        return out;
    }

private:
    static bool readOptionalString(const json& parsed,
                                   const char* key,
                                   std::string* out,
                                   std::string* error) {
        if (!parsed.contains(key)) return true;
        if (!parsed[key].is_string()) {
            *error = std::string("config_field_invalid_type:") + key;
            return false;
        }
        *out = parsed[key].get<std::string>();
        return true;
    }
};
