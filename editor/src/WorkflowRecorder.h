#pragma once
// Step 158: Agent workflow recording

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class WorkflowRecorder {
public:
    struct RecordingConfig {
        bool recordAllSessions = false;
        bool recordControlMethods = false;
        bool recordEditorEvents = true;
        bool autoRecording = false;
    };

    struct WorkflowCall {
        json request;
        json response;
        std::string timestamp;
        std::string sessionId;
        std::string method;
    };

    struct WorkflowEvent {
        std::string type;
        json payload;
        std::string timestamp;
    };

    void startRecording(const std::string& name, const std::string& sessionId) {
        startRecording(name, sessionId, RecordingConfig{}, json::object());
    }

    void startRecording(const std::string& name,
                        const std::string& sessionId,
                        const RecordingConfig& config,
                        const json& metadata) {
        name_ = name.empty() ? "workflow" : name;
        sessionId_ = sessionId;
        calls_.clear();
        events_.clear();
        recording_ = true;
        createdAt_ = currentTimestamp();
        stoppedAt_.clear();
        config_ = config;
        metadata_ = metadata;
    }

    json stopRecording() {
        recording_ = false;
        stoppedAt_ = currentTimestamp();
        return exportWorkflow();
    }

    bool isRecording() const { return recording_; }
    bool isReplaying() const { return replaying_; }

    void setReplaying(bool replaying) { replaying_ = replaying; }
    bool isAutoRecording() const { return config_.autoRecording; }

    void record(const std::string& sessionId,
                const json& request,
                const json& response) {
        if (!recording_ || replaying_) return;
        if (!config_.recordAllSessions && !sessionId_.empty() && sessionId != sessionId_) return;
        std::string method = request.value("method", "");
        if (!config_.recordControlMethods && isControlMethod(method)) return;
        WorkflowCall call;
        call.request = request;
        call.response = response;
        call.timestamp = currentTimestamp();
        call.sessionId = sessionId;
        call.method = method;
        calls_.push_back(std::move(call));
    }

    void recordEvent(const std::string& type, const json& payload) {
        if (!recording_ || replaying_ || !config_.recordEditorEvents) return;
        WorkflowEvent ev;
        ev.type = type;
        ev.payload = payload;
        ev.timestamp = currentTimestamp();
        events_.push_back(std::move(ev));
    }

    json exportWorkflow() const {
        json out;
        out["name"] = name_;
        out["sessionId"] = sessionId_;
        out["createdAt"] = createdAt_;
        out["stoppedAt"] = stoppedAt_;
        out["metadata"] = metadata_;
        json arr = json::array();
        for (const auto& c : calls_) {
            arr.push_back({
                {"timestamp", c.timestamp},
                {"sessionId", c.sessionId},
                {"method", c.method},
                {"request", c.request},
                {"response", c.response}
            });
        }
        out["calls"] = arr;
        json events = json::array();
        for (const auto& e : events_) {
            events.push_back({
                {"timestamp", e.timestamp},
                {"type", e.type},
                {"payload", e.payload}
            });
        }
        out["events"] = events;
        return out;
    }

    bool loadWorkflow(const json& workflow) {
        if (!workflow.contains("calls")) return false;
        name_ = workflow.value("name", "workflow");
        sessionId_ = workflow.value("sessionId", "");
        createdAt_ = workflow.value("createdAt", "");
        stoppedAt_ = workflow.value("stoppedAt", "");
        metadata_ = workflow.value("metadata", json::object());
        calls_.clear();
        for (const auto& c : workflow["calls"]) {
            WorkflowCall call;
            call.timestamp = c.value("timestamp", "");
            call.sessionId = c.value("sessionId", "");
            call.method = c.value("method", "");
            call.request = c.value("request", json::object());
            call.response = c.value("response", json::object());
            calls_.push_back(std::move(call));
        }
        events_.clear();
        if (workflow.contains("events") && workflow["events"].is_array()) {
            for (const auto& e : workflow["events"]) {
                WorkflowEvent ev;
                ev.timestamp = e.value("timestamp", "");
                ev.type = e.value("type", "");
                ev.payload = e.value("payload", json::object());
                events_.push_back(std::move(ev));
            }
        }
        return true;
    }

    std::vector<json> buildReplayRequests() const {
        std::vector<json> out;
        int id = 1;
        for (const auto& c : calls_) {
            json req = c.request;
            req["jsonrpc"] = "2.0";
            req["id"] = id++;
            out.push_back(std::move(req));
        }
        return out;
    }

    void clear() {
        name_.clear();
        sessionId_.clear();
        createdAt_.clear();
        stoppedAt_.clear();
        calls_.clear();
        events_.clear();
        metadata_.clear();
        recording_ = false;
        config_ = RecordingConfig{};
    }

private:
    static std::string currentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
            << '.' << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }

    static bool isControlMethod(const std::string& method) {
        return method == "startWorkflowRecording" ||
               method == "stopWorkflowRecording" ||
               method == "replayWorkflow" ||
               method == "getWorkflowRecording";
    }

    std::string name_;
    std::string sessionId_;
    std::string createdAt_;
    std::string stoppedAt_;
    json metadata_;
    std::vector<WorkflowCall> calls_;
    std::vector<WorkflowEvent> events_;
    bool recording_ = false;
    bool replaying_ = false;
    RecordingConfig config_;
};
