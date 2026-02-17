#pragma once

#include "WorkflowTaskDetail.h"

#include <optional>
#include <string>
#include <vector>

struct ReviewDiffLine {
    char kind = ' '; // ' ' unchanged, '-' removed, '+' added, '~' modified
    std::string left;
    std::string right;
    std::string note;
};

struct ReviewComparisonModel {
    std::string itemId;
    std::string title;
    std::string status;
    std::string leftLabel;
    std::string rightLabel;
    std::vector<ReviewDiffLine> diff;
    std::vector<std::string> changeNotes;
    bool canApprove = false;
    bool canReject = false;
    std::string approveShortcut;
    std::string rejectShortcut;
};

class ReviewComparisonView {
public:
    static std::optional<ReviewComparisonModel> build(const WorkflowState& wf,
                                                      const std::string& itemId) {
        auto detail = WorkflowTaskDetail::build(wf, itemId);
        if (!detail.has_value()) return std::nullopt;

        ReviewComparisonModel m;
        m.itemId = detail->itemId;
        m.title = detail->title;
        m.status = detail->status;
        m.leftLabel = "skeleton";
        m.rightLabel = "generated";
        m.diff = buildDiffLines(detail->skeletonCode, detail->generatedCode);
        m.changeNotes = buildChangeNotes(*detail);
        m.canApprove = (detail->status == WI_REVIEW);
        m.canReject = (detail->status == WI_REVIEW);
        m.approveShortcut = approveShortcut();
        m.rejectShortcut = rejectShortcut();
        return m;
    }

    static bool approve(WorkflowState& wf, const std::string& itemId,
                        const std::string& reviewer = "human") {
        return WorkflowTaskDetail::approve(wf, itemId, reviewer);
    }

    static bool reject(WorkflowState& wf, const std::string& itemId,
                       const std::string& feedback,
                       const std::string& reviewer = "human") {
        return WorkflowTaskDetail::reject(wf, itemId, feedback, reviewer);
    }

    static std::string approveShortcut() { return "Ctrl+Shift+A"; }

    static std::string rejectShortcut() { return "Ctrl+Shift+R"; }

private:
    static std::vector<std::string> splitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::string cur;
        for (char c : text) {
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) lines.push_back(cur);
        return lines;
    }

    static std::vector<ReviewDiffLine> buildDiffLines(const std::string& left,
                                                      const std::string& right) {
        auto l = splitLines(left);
        auto r = splitLines(right);
        size_t n = l.size() > r.size() ? l.size() : r.size();
        std::vector<ReviewDiffLine> out;
        for (size_t i = 0; i < n; ++i) {
            std::string ls = i < l.size() ? l[i] : "";
            std::string rs = i < r.size() ? r[i] : "";
            if (i >= l.size()) {
                out.push_back({'+', "", rs, "added"});
            } else if (i >= r.size()) {
                out.push_back({'-', ls, "", "removed"});
            } else if (ls == rs) {
                out.push_back({' ', ls, rs, "unchanged"});
            } else {
                out.push_back({'~', ls, rs, "modified"});
            }
        }
        return out;
    }

    static std::vector<std::string> buildChangeNotes(const TaskDetailView& detail) {
        std::vector<std::string> notes;
        if (!detail.routingExplanation.empty()) {
            notes.push_back("routing: " + detail.routingExplanation);
        }
        if (!detail.rejectionFeedback.empty()) {
            notes.push_back("latest-feedback: " + detail.rejectionFeedback);
        }
        for (const auto& t : detail.annotationTags) {
            notes.push_back("annotation: " + t);
        }
        return notes;
    }
};
