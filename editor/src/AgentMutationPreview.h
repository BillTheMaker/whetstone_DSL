#pragma once
// Step 626: AST mutation preview model

#include "DiffUtils.h"

#include <string>
#include <vector>

struct AgentMutationPreview {
    std::string previewId;
    std::string mutationJson;
    std::string beforeCode;
    std::string afterCode;
    LineDiff diff;
    bool hasChanges = false;
};

class AgentMutationPreviewBuilder {
public:
    static AgentMutationPreview build(const std::string& previewId,
                                      const std::string& mutationJson,
                                      const std::string& beforeCode,
                                      const std::string& afterCode) {
        AgentMutationPreview out;
        out.previewId = previewId;
        out.mutationJson = mutationJson;
        out.beforeCode = beforeCode;
        out.afterCode = afterCode;
        out.diff = buildLineDiff(beforeCode, afterCode);
        out.hasChanges = !out.diff.beforeLines.empty() || !out.diff.afterLines.empty();
        return out;
    }

    static std::vector<std::string> beforeLines(const AgentMutationPreview& preview) {
        return splitLines(preview.beforeCode);
    }

    static std::vector<std::string> afterLines(const AgentMutationPreview& preview) {
        return splitLines(preview.afterCode);
    }
};
