#pragma once
// Step 684: Per-session MCP tool call instrumentation.

#include <string>
#include <vector>

struct ToolCallRecord {
    std::string toolName;
    int inputChars = 0;
    int outputChars = 0;
    int estimatedTokens = 0;
    long long durationMs = 0;
    bool wasFileRead = false;
};

struct SessionRecord {
    std::string sessionId;
    std::string taskDescription;
    std::vector<ToolCallRecord> calls;
    int totalToolCalls = 0;
    int totalEstimatedTokens = 0;
    int fileReadCount = 0;
    long long totalDurationMs = 0;
};

class AgentSessionRecorder {
public:
    void start(const std::string& sessionId, const std::string& taskDescription) {
        active_ = true;
        current_ = SessionRecord{};
        current_.sessionId = sessionId;
        current_.taskDescription = taskDescription;
    }

    void record(const ToolCallRecord& call) {
        if (!active_) return;
        current_.calls.push_back(call);
        current_.totalToolCalls++;
        current_.totalEstimatedTokens += call.estimatedTokens;
        current_.totalDurationMs += call.durationMs;
        if (call.wasFileRead) current_.fileReadCount++;
    }

    SessionRecord finish() {
        SessionRecord out = current_;
        active_ = false;
        current_ = SessionRecord{};
        return out;
    }

    bool isActive() const { return active_; }
    const SessionRecord& current() const { return current_; }

    static ToolCallRecord makeRecord(const std::string& toolName,
                                     const std::string& inputJson,
                                     const std::string& outputJson,
                                     long long durationMs) {
        ToolCallRecord out;
        out.toolName = toolName;
        out.inputChars = static_cast<int>(inputJson.size());
        out.outputChars = static_cast<int>(outputJson.size());
        out.estimatedTokens = (out.inputChars + out.outputChars) / 4;
        out.durationMs = durationMs;
        out.wasFileRead = isFileReadTool(toolName);
        return out;
    }

private:
    static bool contains(const std::string& haystack, const std::string& needle) {
        return haystack.find(needle) != std::string::npos;
    }

    static bool isFileReadTool(const std::string& toolName) {
        return contains(toolName, "read") ||
               contains(toolName, "get_ast") ||
               contains(toolName, "assemble_context") ||
               contains(toolName, "workspace_list");
    }

    bool active_ = false;
    SessionRecord current_;
};

