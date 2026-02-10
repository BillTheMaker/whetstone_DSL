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
    struct WorkflowCall {
        json request;
        json response;
        std::string timestamp;
    };

    void startRecording(const std::string& name, const std::string& sessionId) {
        name_ = name.empty() ? "workflow" : name;
        sessionId_ = sessionId;
        calls_.clear();
        recording_ = true;
        createdAt_ = currentTimestamp();
        stoppedAt_.clear();
    }

    json stopRecording() {
        recording_ = false;
        stoppedAt_ = currentTimestamp();
        return exportWorkflow();
    }

    bool isRecording() const { return recording_; }
    bool isReplaying() const { return replaying_; }

    void setReplaying(bool replaying) { replaying_ = replaying; }

    void record(const std::string& sessionId,
                const json& request,
                const json& response) {
        if (!recording_ || replaying_) return;
        if (!sessionId_.empty() && sessionId != sessionId_) return;
        std::string method = request.value("method", "");
        if (isControlMethod(method)) return;
        WorkflowCall call;
        call.request = request;
        call.response = response;
        call.timestamp = currentTimestamp();
        calls_.push_back(std::move(call));
    }

    json exportWorkflow() const {
        json out;
        out["name"] = name_;
        out["sessionId"] = sessionId_;
        out["createdAt"] = createdAt_;
        out["stoppedAt"] = stoppedAt_;
        json arr = json::array();
        for (const auto& c : calls_) {
            arr.push_back({
                {"timestamp", c.timestamp},
                {"request", c.request},
                {"response", c.response}
            });
        }
        out["calls"] = arr;
        return out;
    }

    bool loadWorkflow(const json& workflow) {
        if (!workflow.contains("calls")) return false;
        name_ = workflow.value("name", "workflow");
        sessionId_ = workflow.value("sessionId", "");
        createdAt_ = workflow.value("createdAt", "");
        stoppedAt_ = workflow.value("stoppedAt", "");
        calls_.clear();
        for (const auto& c : workflow["calls"]) {
            WorkflowCall call;
            call.timestamp = c.value("timestamp", "");
            call.request = c.value("request", json::object());
            call.response = c.value("response", json::object());
            calls_.push_back(std::move(call));
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
        recording_ = false;
    }

private:
    static std::string currentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t tt = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &tt);
#else
        localtime_r(&tt, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
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
    std::vector<WorkflowCall> calls_;
    bool recording_ = false;
    bool replaying_ = false;
};
