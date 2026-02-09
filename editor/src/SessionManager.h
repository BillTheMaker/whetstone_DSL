#pragma once
// Step 117: Session persistence

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct SessionBufferState {
    std::string path;
    std::string language;
    std::string mode;
    int cursorLine = 1;
    int cursorCol = 1;
    std::vector<int> foldedLines;
};

struct SessionData {
    std::string workspaceRoot;
    std::string activePath;
    std::string layoutPreset;
    std::string imguiIni;
    std::vector<SessionBufferState> buffers;
};

inline nlohmann::json sessionToJson(const SessionData& session) {
    nlohmann::json j;
    j["workspaceRoot"] = session.workspaceRoot;
    j["activePath"] = session.activePath;
    j["layoutPreset"] = session.layoutPreset;
    j["imguiIni"] = session.imguiIni;
    j["buffers"] = nlohmann::json::array();
    for (const auto& b : session.buffers) {
        nlohmann::json item;
        item["path"] = b.path;
        item["language"] = b.language;
        item["mode"] = b.mode;
        item["cursorLine"] = b.cursorLine;
        item["cursorCol"] = b.cursorCol;
        item["foldedLines"] = b.foldedLines;
        j["buffers"].push_back(std::move(item));
    }
    return j;
}

inline SessionData sessionFromJson(const nlohmann::json& j) {
    SessionData session;
    session.workspaceRoot = j.value("workspaceRoot", "");
    session.activePath = j.value("activePath", "");
    session.layoutPreset = j.value("layoutPreset", "VSCode");
    session.imguiIni = j.value("imguiIni", "");
    if (j.contains("buffers") && j["buffers"].is_array()) {
        for (const auto& item : j["buffers"]) {
            SessionBufferState b;
            b.path = item.value("path", "");
            b.language = item.value("language", "");
            b.mode = item.value("mode", "structured");
            b.cursorLine = item.value("cursorLine", 1);
            b.cursorCol = item.value("cursorCol", 1);
            if (item.contains("foldedLines") && item["foldedLines"].is_array()) {
                for (const auto& ln : item["foldedLines"]) {
                    if (ln.is_number_integer()) b.foldedLines.push_back(ln.get<int>());
                }
            }
            if (!b.path.empty()) session.buffers.push_back(std::move(b));
        }
    }
    return session;
}
