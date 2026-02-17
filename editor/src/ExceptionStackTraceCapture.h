#pragma once
// Step 557: Exception + Stack Trace Capture

#include <string>
#include <vector>

#include "DebugValidationUtil.h"

struct StackFrame {
    std::string functionName;
    std::string filePath;
    int line = 0;
    std::string navigationAnchor;
};

struct ExceptionEvent {
    std::string type;
    std::string message;
    std::string threadId;
    std::vector<StackFrame> frames;
};

struct ExceptionCaptureResult {
    bool valid = false;
    ExceptionEvent event;
    std::vector<std::string> errors;
};

class ExceptionStackTraceCapture {
public:
    static ExceptionCaptureResult capture(const std::string& type,
                                          const std::string& message,
                                          const std::string& threadId,
                                          const std::vector<StackFrame>& rawFrames) {
        ExceptionCaptureResult out;

        if (type.empty()) out.errors.push_back("missing_type");
        if (message.empty()) out.errors.push_back("missing_message");
        if (threadId.empty()) out.errors.push_back("missing_thread_id");

        out.event.type = type;
        out.event.message = message;
        out.event.threadId = threadId;
        out.event.frames = normalizeFrames(rawFrames, out.errors);

        if (out.event.frames.empty()) out.errors.push_back("missing_stack_frames");
        out.valid = out.errors.empty();
        return out;
    }

private:
    static std::vector<StackFrame> normalizeFrames(const std::vector<StackFrame>& frames,
                                                   std::vector<std::string>& errors) {
        std::vector<StackFrame> out;
        int index = 0;
        for (const auto& f : frames) {
            if (!hasRequiredDebugIds(f.functionName, f.filePath) || !isPositiveLine(f.line)) {
                errors.push_back("invalid_frame:" + std::to_string(index));
                ++index;
                continue;
            }
            StackFrame normalized = f;
            normalized.navigationAnchor = makeAnchor(f.filePath, f.line);
            out.push_back(normalized);
            ++index;
        }
        return out;
    }

    static std::string makeAnchor(const std::string& filePath, int line) {
        return filePath + ":" + std::to_string(line);
    }
};
