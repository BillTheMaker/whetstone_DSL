#pragma once
// Step 559: Call Stack and Frame Inspector

#include <map>
#include <string>
#include <vector>

struct FrameScope {
    int frameIndex = 0;
    std::string functionName;
    std::string filePath;
    int line = 0;
    std::map<std::string, std::string> variables;
};

struct FrameInspectorResult {
    bool ok = false;
    int selectedIndex = -1;
    FrameScope selected;
    std::vector<std::string> errors;
};

class CallStackFrameInspector {
public:
    static FrameInspectorResult inspect(const std::vector<FrameScope>& frames,
                                        int requestedIndex) {
        FrameInspectorResult out;
        if (frames.empty()) {
            out.errors.push_back("empty_stack");
            return out;
        }
        if (requestedIndex < 0 || requestedIndex >= (int)frames.size()) {
            out.errors.push_back("invalid_frame_index");
            return out;
        }
        if (!isFrameValid(frames[requestedIndex])) {
            out.errors.push_back("invalid_frame_payload");
            return out;
        }

        out.ok = true;
        out.selectedIndex = requestedIndex;
        out.selected = frames[requestedIndex];
        return out;
    }

    static std::vector<std::string> variableNames(const FrameScope& frame) {
        std::vector<std::string> names;
        for (const auto& kv : frame.variables) names.push_back(kv.first);
        return names;
    }

private:
    static bool isFrameValid(const FrameScope& f) {
        return f.frameIndex >= 0 && !f.functionName.empty() &&
               !f.filePath.empty() && f.line > 0;
    }
};
