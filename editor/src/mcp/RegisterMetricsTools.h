// Step 687: MCP wiring for session metrics and A/B comparison
//
// Registers:
//   whetstone_start_recording
//   whetstone_get_metrics
//
// This file is #included inside the MCPServer class body.

#include "../ABTestComparison.h"

    void registerMetricsTools() {
        tools_.push_back({"whetstone_start_recording",
            "Begin recording MCP tool-call metrics for a new session.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"},
                    {"description", "Unique session identifier."}}},
                {"task_description", {{"type", "string"},
                    {"description", "Task being executed in this session."}}}
            }}, {"required", json::array({"session_id", "task_description"})}}
        });
        toolHandlers_["whetstone_start_recording"] =
            [this](const json& args) {
                return runStartRecording(args);
            };

        tools_.push_back({"whetstone_get_metrics",
            "Finalize current session metrics and optionally compare against a baseline.",
            {{"type", "object"}, {"properties", {
                {"session_id", {{"type", "string"}}},
                {"baseline", {{"type", "object"},
                    {"description", "Optional baseline SessionRecord JSON."}}},
                {"quality_rating", {{"type", "integer"},
                    {"description", "Quality rating (0-100) for this session."}}},
                {"baseline_quality", {{"type", "integer"},
                    {"description", "Baseline quality rating if baseline is provided."}}}
            }}, {"required", json::array({"session_id"})}}
        });
        toolHandlers_["whetstone_get_metrics"] =
            [this](const json& args) {
                return runGetMetrics(args);
            };
    }

    json runStartRecording(const json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) {
            return {{"success", false}, {"error", "session_id_missing"}};
        }
        if (!args.contains("task_description") || !args["task_description"].is_string()) {
            return {{"success", false}, {"error", "task_description_missing"}};
        }

        const std::string sessionId = args["session_id"].get<std::string>();
        const std::string taskDescription = args["task_description"].get<std::string>();
        sessionRecorder_.start(sessionId, taskDescription);
        recordingActive_ = true;
        recordingSessionId_ = sessionId;
        return {{"success", true}, {"session_id", sessionId}};
    }

    json runGetMetrics(const json& args) {
        if (!args.contains("session_id") || !args["session_id"].is_string()) {
            return {{"success", false}, {"error", "session_id_missing"}};
        }
        const std::string sessionId = args["session_id"].get<std::string>();
        if (!recordingActive_ || sessionId != recordingSessionId_) {
            return {{"success", false}, {"error", "session_not_active"}};
        }

        SessionRecord session = sessionRecorder_.finish();
        recordingActive_ = false;
        recordingSessionId_.clear();

        json response = {
            {"success", true},
            {"session", sessionToJson(session)}
        };

        if (args.contains("baseline") && args["baseline"].is_object()) {
            SessionRecord baseline = parseSessionFromJson(args["baseline"]);
            const int quality = args.value("quality_rating", 0);
            const int baselineQuality = args.value("baseline_quality", 0);
            ABComparisonResult cmp = ABTestComparison::compare(
                session.taskDescription.empty() ? session.sessionId : session.taskDescription,
                baseline,
                session,
                baselineQuality,
                quality
            );
            response["comparison"] = comparisonToJson(cmp);
        }

        return response;
    }

    static json sessionToJson(const SessionRecord& s) {
        json calls = json::array();
        for (const auto& c : s.calls) {
            calls.push_back({
                {"tool_name", c.toolName},
                {"input_chars", c.inputChars},
                {"output_chars", c.outputChars},
                {"estimated_tokens", c.estimatedTokens},
                {"duration_ms", c.durationMs},
                {"was_file_read", c.wasFileRead}
            });
        }
        return {
            {"session_id", s.sessionId},
            {"task_description", s.taskDescription},
            {"calls", calls},
            {"total_tool_calls", s.totalToolCalls},
            {"total_estimated_tokens", s.totalEstimatedTokens},
            {"file_read_count", s.fileReadCount},
            {"total_duration_ms", s.totalDurationMs}
        };
    }

    static SessionRecord parseSessionFromJson(const json& j) {
        SessionRecord s;
        s.sessionId = j.value("session_id", "");
        s.taskDescription = j.value("task_description", "");
        s.totalToolCalls = j.value("total_tool_calls", 0);
        s.totalEstimatedTokens = j.value("total_estimated_tokens", 0);
        s.fileReadCount = j.value("file_read_count", 0);
        s.totalDurationMs = j.value("total_duration_ms", 0LL);
        if (j.contains("calls") && j["calls"].is_array()) {
            for (const auto& c : j["calls"]) {
                ToolCallRecord r;
                r.toolName = c.value("tool_name", "");
                r.inputChars = c.value("input_chars", 0);
                r.outputChars = c.value("output_chars", 0);
                r.estimatedTokens = c.value("estimated_tokens", 0);
                r.durationMs = c.value("duration_ms", 0LL);
                r.wasFileRead = c.value("was_file_read", false);
                s.calls.push_back(r);
            }
        }
        return s;
    }

    static json comparisonToJson(const ABComparisonResult& c) {
        return {
            {"task_id", c.taskId},
            {"baseline_tokens", c.baselineTokens},
            {"whetstone_tokens", c.whetstoneTokens},
            {"token_reduction_pct", c.tokenReductionPct},
            {"baseline_file_reads", c.baselineFileReads},
            {"whetstone_file_reads", c.whetstoneFileReads},
            {"file_read_reduction_pct", c.fileReadReductionPct},
            {"baseline_duration_ms", c.baselineDurationMs},
            {"whetstone_duration_ms", c.whetstoneDurationMs},
            {"duration_delta_pct", c.durationDeltaPct},
            {"baseline_quality", c.baselineQuality},
            {"whetstone_quality", c.whetstoneQuality},
            {"quality_delta", c.qualityDelta},
            {"whetstone_won", c.whetstoneWon}
        };
    }

