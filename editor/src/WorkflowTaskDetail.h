#pragma once

#include "WorkflowState.h"

#include <string>
#include <vector>

struct TaskDetailView {
    std::string itemId;
    std::string title;
    std::string status;
    std::string workerType;
    std::string priority;
    std::string skeletonCode;
    std::string generatedCode;
    std::string diffSummary;
    std::string routingExplanation;
    std::string rejectionFeedback;
    std::vector<RejectionAttempt> rejectionHistory;
    std::vector<std::string> annotationTags;
};

class WorkflowTaskDetail {
public:
    static std::optional<TaskDetailView> build(const WorkflowState& wf,
                                               const std::string& itemId) {
        auto item = wf.queue.getItem(itemId);
        if (!item.has_value()) return std::nullopt;
        TaskDetailView v;
        v.itemId = item->id;
        v.title = item->nodeName.empty() ? item->nodeId : item->nodeName;
        v.status = item->status;
        v.workerType = item->workerType;
        v.priority = item->priority;
        v.skeletonCode = buildSkeletonStub(*item);
        v.generatedCode = item->result.generatedCode;
        v.diffSummary = buildDiffSummary(v.skeletonCode, v.generatedCode);
        v.routingExplanation = buildRoutingExplanation(*item);
        v.rejectionFeedback = item->rejectionFeedback;
        v.rejectionHistory = item->rejectionHistory;
        v.annotationTags = buildAnnotationTags(*item);
        return v;
    }

    static bool approve(WorkflowState& wf, const std::string& itemId,
                        const std::string& reviewer = "human") {
        auto item = wf.queue.getItem(itemId);
        if (!item.has_value()) return false;
        if (item->status != WI_REVIEW) return false;
        WorkItem updated = *item;
        if (!transitionWorkItem(updated, WI_COMPLETE)) return false;
        wf.recordChange(itemId, WI_REVIEW, WI_COMPLETE, reviewer, "approved");
        return wf.queue.updateItem(itemId, updated);
    }

    static bool reject(WorkflowState& wf, const std::string& itemId,
                       const std::string& feedback,
                       const std::string& reviewer = "human") {
        if (feedback.empty()) return false;
        auto item = wf.queue.getItem(itemId);
        if (!item.has_value()) return false;
        if (item->status != WI_REVIEW) return false;
        bool ok = wf.queue.reject(itemId, feedback);
        if (!ok) return false;
        wf.recordChange(itemId, WI_REVIEW, WI_READY, reviewer, feedback);
        return true;
    }

private:
    static std::string buildSkeletonStub(const WorkItem& wi) {
        std::string name = wi.nodeName.empty() ? wi.nodeId : wi.nodeName;
        std::string lang = languageFromPath(wi.bufferId);
        if (lang == "python") return "def " + name + "():\n    pass\n";
        if (lang == "cpp") return "void " + name + "() {\n}\n";
        return name + "() {}";
    }

    static std::string languageFromPath(const std::string& path) {
        auto dot = path.find_last_of('.');
        if (dot == std::string::npos) return "";
        std::string ext = path.substr(dot);
        if (ext == ".cpp" || ext == ".hpp" || ext == ".h") return "cpp";
        if (ext == ".py") return "python";
        return "";
    }

    static int lineCount(const std::string& text) {
        if (text.empty()) return 0;
        int lines = 1;
        for (char c : text) if (c == '\n') ++lines;
        return lines;
    }

    static std::string buildDiffSummary(const std::string& skeleton,
                                        const std::string& generated) {
        int s = lineCount(skeleton);
        int g = lineCount(generated);
        int delta = g - s;
        std::string sign = delta >= 0 ? "+" : "";
        return "line_delta=" + sign + std::to_string(delta) +
               " skeleton_lines=" + std::to_string(s) +
               " generated_lines=" + std::to_string(g);
    }

    static std::string buildRoutingExplanation(const WorkItem& wi) {
        std::string reason = "worker=" + wi.workerType;
        if (!wi.contextWidth.empty()) reason += " context=" + wi.contextWidth;
        if (wi.reviewRequired) reason += " review=required";
        if (!wi.priority.empty()) reason += " priority=" + wi.priority;
        return reason;
    }

    static std::vector<std::string> buildAnnotationTags(const WorkItem& wi) {
        std::vector<std::string> tags;
        if (!wi.contextWidth.empty()) tags.push_back("ContextWidth:" + wi.contextWidth);
        if (!wi.workerType.empty()) tags.push_back("Automatability:" + wi.workerType);
        if (wi.reviewRequired) tags.push_back("Review:required");
        if (!wi.priority.empty()) tags.push_back("Priority:" + wi.priority);
        return tags;
    }
};
