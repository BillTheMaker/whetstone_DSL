#pragma once
// Step 630: Chat session persistence

#include "AgentChatPanelModel.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct AgentChatSavedSession {
    std::string timestamp;
    AgentChatState chat;
};

class AgentChatSessionPersistence {
public:
    static std::string sessionStorePath(const std::string& workspaceRoot) {
        fs::path root = workspaceRoot.empty() ? fs::path(".") : fs::path(workspaceRoot);
        return (root / ".whetstone.chat_sessions.json").string();
    }

    static bool saveSession(const std::string& workspaceRoot,
                            const std::string& projectFile,
                            const std::string& timestamp,
                            const AgentChatState& chat,
                            std::string* error) {
        if (!error) return false;
        error->clear();
        if (projectFile.empty()) return fail(error, "project_file_missing");
        if (timestamp.empty()) return fail(error, "timestamp_missing");

        json store = readStore(workspaceRoot, error);
        if (!error->empty()) return false;
        if (!store.contains("projects")) store["projects"] = json::object();
        if (!store["projects"].contains(projectFile)) store["projects"][projectFile] = json::array();

        store["projects"][projectFile].push_back({
            {"timestamp", timestamp},
            {"chat", chatToJson(chat)}
        });
        return writeStore(workspaceRoot, store, error);
    }

    static bool loadLatestSession(const std::string& workspaceRoot,
                                  const std::string& projectFile,
                                  AgentChatSavedSession* out,
                                  std::string* error) {
        if (!out || !error) return false;
        error->clear();
        out->timestamp.clear();
        out->chat = AgentChatState{};
        if (projectFile.empty()) return fail(error, "project_file_missing");

        json store = readStore(workspaceRoot, error);
        if (!error->empty()) return false;
        if (!store.contains("projects") ||
            !store["projects"].contains(projectFile) ||
            !store["projects"][projectFile].is_array() ||
            store["projects"][projectFile].empty()) {
            return fail(error, "session_not_found");
        }

        const auto& latest = store["projects"][projectFile].back();
        out->timestamp = latest.value("timestamp", "");
        if (!latest.contains("chat") || !latest["chat"].is_object()) {
            return fail(error, "chat_payload_invalid");
        }
        out->chat = jsonToChat(latest["chat"]);
        return true;
    }

    static std::vector<std::string> listSessionTimestamps(const std::string& workspaceRoot,
                                                          const std::string& projectFile,
                                                          std::string* error) {
        std::vector<std::string> out;
        if (!error) return out;
        error->clear();
        if (projectFile.empty()) {
            *error = "project_file_missing";
            return out;
        }
        json store = readStore(workspaceRoot, error);
        if (!error->empty()) return out;
        if (!store.contains("projects") || !store["projects"].contains(projectFile)) return out;
        for (const auto& session : store["projects"][projectFile]) {
            out.push_back(session.value("timestamp", ""));
        }
        return out;
    }

private:
    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    static json chatToJson(const AgentChatState& chat) {
        json messages = json::array();
        for (const auto& message : chat.messages) {
            messages.push_back({
                {"role", AgentChatPanelModel::roleLabel(message.role)},
                {"content", message.content},
                {"timestamp", message.timestamp}
            });
        }
        return {
            {"systemContext", chat.systemContext},
            {"messages", messages}
        };
    }

    static AgentChatState jsonToChat(const json& value) {
        AgentChatState out;
        out.systemContext = value.value("systemContext", "");
        if (!value.contains("messages") || !value["messages"].is_array()) return out;
        for (const auto& message : value["messages"]) {
            AgentChatRole role = AgentChatRole::Tool;
            const std::string roleText = message.value("role", "tool");
            if (roleText == "user") role = AgentChatRole::User;
            else if (roleText == "assistant") role = AgentChatRole::Assistant;
            out.messages.push_back({
                role,
                message.value("content", ""),
                message.value("timestamp", "")
            });
        }
        return out;
    }

    static json readStore(const std::string& workspaceRoot, std::string* error) {
        const std::string path = sessionStorePath(workspaceRoot);
        if (!fs::exists(path)) return json::object();
        std::ifstream in(path);
        if (!in.is_open()) {
            *error = "session_store_open_failed";
            return json::object();
        }
        try {
            json parsed = json::parse(in);
            if (!parsed.is_object()) {
                *error = "session_store_invalid";
                return json::object();
            }
            return parsed;
        } catch (...) {
            *error = "session_store_parse_failed";
            return json::object();
        }
    }

    static bool writeStore(const std::string& workspaceRoot,
                           const json& store,
                           std::string* error) {
        fs::path path = sessionStorePath(workspaceRoot);
        if (path.has_parent_path()) fs::create_directories(path.parent_path());
        std::ofstream out(path);
        if (!out.is_open()) return fail(error, "session_store_write_failed");
        out << store.dump(2);
        return true;
    }
};
