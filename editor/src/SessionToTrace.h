#pragma once
// Step 232: Session-to-trace converter

#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "TraceGenerator.h"

using json = nlohmann::json;

class SessionTraceConverter {
public:
    void setGapSeconds(int seconds) { gapSeconds_ = seconds; }

    std::vector<Trace> convert(const json& session) const {
        std::vector<Trace> traces;
        if (!session.is_object()) return traces;

        std::string sessionId = session.value("sessionId", "session");
        std::string language = session.value("metadata", json::object()).value("language", "unknown");

        std::vector<Item> items = collectItems(session);
        if (items.empty()) return traces;
        std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
            return a.timestamp < b.timestamp;
        });

        Trace current;
        int traceIndex = 0;
        current.id = sessionId + "_" + std::to_string(traceIndex);
        current.scenario = "session_replay";
        current.difficulty = "intermediate";
        current.language = language;

        std::string prevTs;
        for (const auto& item : items) {
            if (!prevTs.empty() && gapSeconds_ > 0) {
                int gap = secondsBetween(prevTs, item.timestamp);
                if (gap > gapSeconds_ && !current.steps.empty()) {
                    finalizeTrace(current, traces, sessionId, ++traceIndex, language);
                    prevTs.clear();
                }
            }
            prevTs = item.timestamp;

            if (item.kind == ItemKind::Event) {
                std::string message = eventToUserMessage(item.eventType, item.payload);
                if (!message.empty()) {
                    current.steps.push_back({"user", message, "", json(), json()});
                }
            } else if (item.kind == ItemKind::Call) {
                std::string tool = mapMethodToTool(item.method);
                if (tool.empty()) continue;
                current.steps.push_back({"assistant", "Thinking about the next action.", "", json(), json()});
                TraceStep call;
                call.role = "tool_call";
                call.toolName = tool;
                call.toolInput = item.params;
                current.steps.push_back(call);

                TraceStep result;
                result.role = "tool_result";
                result.toolName = tool;
                result.toolOutput = item.response;
                current.steps.push_back(result);

                current.toolsUsed.push_back(tool);
                current.toolCallCount++;
                if (item.response.contains("error")) {
                    current.success = false;
                }
            }
        }

        if (!current.steps.empty()) {
            traces.push_back(current);
        }
        return traces;
    }

private:
    enum class ItemKind { Event, Call };
    struct Item {
        ItemKind kind;
        std::string timestamp;
        std::string eventType;
        json payload;
        std::string method;
        json params;
        json response;
    };

    int gapSeconds_ = 300;

    static int secondsBetween(const std::string& a, const std::string& b) {
        // ISO-like timestamps sort lexicographically, parse only to seconds precision
        auto toSeconds = [](const std::string& ts) {
            if (ts.size() < 19) return 0;
            int year = std::stoi(ts.substr(0, 4));
            int mon = std::stoi(ts.substr(5, 2));
            int day = std::stoi(ts.substr(8, 2));
            int hour = std::stoi(ts.substr(11, 2));
            int min = std::stoi(ts.substr(14, 2));
            int sec = std::stoi(ts.substr(17, 2));
            std::tm tm{};
            tm.tm_year = year - 1900;
            tm.tm_mon = mon - 1;
            tm.tm_mday = day;
            tm.tm_hour = hour;
            tm.tm_min = min;
            tm.tm_sec = sec;
            return (int)std::mktime(&tm);
        };
        int ta = toSeconds(a);
        int tb = toSeconds(b);
        return tb - ta;
    }

    static std::vector<Item> collectItems(const json& session) {
        std::vector<Item> items;
        if (session.contains("events") && session["events"].is_array()) {
            for (const auto& ev : session["events"]) {
                Item item;
                item.kind = ItemKind::Event;
                item.timestamp = ev.value("timestamp", "");
                item.eventType = ev.value("type", "");
                item.payload = ev.value("payload", json::object());
                items.push_back(std::move(item));
            }
        }
        if (session.contains("calls") && session["calls"].is_array()) {
            for (const auto& call : session["calls"]) {
                Item item;
                item.kind = ItemKind::Call;
                item.timestamp = call.value("timestamp", "");
                item.method = call.value("method", call.value("request", json::object()).value("method", ""));
                item.params = call.value("request", json::object()).value("params", json::object());
                item.response = call.value("response", json::object());
                items.push_back(std::move(item));
            }
        }
        return items;
    }

    static void finalizeTrace(Trace& current,
                              std::vector<Trace>& out,
                              const std::string& sessionId,
                              int traceIndex,
                              const std::string& language) {
        if (!current.steps.empty()) out.push_back(current);
        current = {};
        current.id = sessionId + "_" + std::to_string(traceIndex);
        current.scenario = "session_replay";
        current.difficulty = "intermediate";
        current.language = language;
    }

    static std::string eventToUserMessage(const std::string& type, const json& payload) {
        if (type == "file_opened") {
            return "Opened file " + payload.value("path", "(unknown)") + ".";
        }
        if (type == "buffer_switched") {
            return "Switched buffer to " + payload.value("path", "(unknown)") + ".";
        }
        if (type == "theme_changed") {
            return "Changed theme.";
        }
        if (type == "file_saved") {
            return "Saved file " + payload.value("path", "(unknown)") + ".";
        }
        if (type == "notification_posted") {
            return "Noted: " + payload.value("detail", "");
        }
        return "";
    }

    static std::string mapMethodToTool(const std::string& method) {
        if (method == "getAST") return "whetstone_get_ast";
        if (method == "applyMutation") return "whetstone_mutate";
        if (method == "applyBatch") return "whetstone_batch_mutate";
        if (method == "getInScopeSymbols") return "whetstone_get_scope";
        if (method == "getCallHierarchy") return "whetstone_get_call_hierarchy";
        if (method == "getAnnotationSuggestions") return "whetstone_suggest_annotations";
        if (method == "applyAnnotationSuggestion") return "whetstone_apply_annotation";
        if (method == "generateCode") return "whetstone_generate_code";
        if (method == "runPipeline") return "whetstone_run_pipeline";
        if (method == "projectLanguage") return "whetstone_project_language";
        return "";
    }
};
