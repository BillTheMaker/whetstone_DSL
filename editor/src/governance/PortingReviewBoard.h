#pragma once
// Step 819: Porting review board model.

#include <map>
#include <string>
#include <vector>

enum class PortingReviewSeverity {
    Low,
    Medium,
    High
};

enum class PortingReviewVerdict {
    Pending,
    Approved,
    Modified,
    Rejected
};

struct PortingReviewIssue {
    std::string issueId;
    std::string description;
    PortingReviewSeverity severity = PortingReviewSeverity::Medium;
    std::string suggestedFix;
    PortingReviewVerdict verdict = PortingReviewVerdict::Pending;
    std::string reviewer;
    std::string rationale;
};

struct PortingReviewSummary {
    int pending = 0;
    int approved = 0;
    int modified = 0;
    int rejected = 0;
    bool allDecided = false;
    bool highSeverityPending = false;
};

class PortingReviewBoard {
public:
    bool addIssue(const PortingReviewIssue& issue, std::string* error) {
        if (!error) return false;
        error->clear();
        if (issue.issueId.empty()) return fail(error, "issue_id_missing");
        if (issue.description.empty()) return fail(error, "description_missing");
        if (issues_.count(issue.issueId) != 0) return fail(error, "issue_duplicate");
        issues_[issue.issueId] = issue;
        order_.push_back(issue.issueId);
        return true;
    }

    bool assignReviewer(const std::string& issueId,
                        const std::string& reviewer,
                        std::string* error) {
        if (!error) return false;
        error->clear();
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        auto* issue = find(issueId);
        if (!issue) return fail(error, "issue_not_found");
        issue->reviewer = reviewer;
        return true;
    }

    bool recordDecision(const std::string& issueId,
                        PortingReviewVerdict verdict,
                        const std::string& rationale,
                        std::string* error) {
        if (!error) return false;
        error->clear();
        auto* issue = find(issueId);
        if (!issue) return fail(error, "issue_not_found");
        if (issue->reviewer.empty()) return fail(error, "reviewer_not_assigned");
        if (rationale.empty()) return fail(error, "rationale_missing");
        issue->verdict = verdict;
        issue->rationale = rationale;
        return true;
    }

    PortingReviewSummary summarize() const {
        PortingReviewSummary summary;
        for (const auto& id : order_) {
            const auto& issue = issues_.at(id);
            if (issue.verdict == PortingReviewVerdict::Pending &&
                issue.severity == PortingReviewSeverity::High) {
                summary.highSeverityPending = true;
            }
            switch (issue.verdict) {
                case PortingReviewVerdict::Pending:
                    ++summary.pending;
                    break;
                case PortingReviewVerdict::Approved:
                    ++summary.approved;
                    break;
                case PortingReviewVerdict::Modified:
                    ++summary.modified;
                    break;
                case PortingReviewVerdict::Rejected:
                    ++summary.rejected;
                    break;
            }
        }
        summary.allDecided = summary.pending == 0 && !order_.empty();
        return summary;
    }

    std::vector<PortingReviewIssue> issues() const {
        std::vector<PortingReviewIssue> out;
        for (const auto& id : order_) out.push_back(issues_.at(id));
        return out;
    }

private:
    std::map<std::string, PortingReviewIssue> issues_;
    std::vector<std::string> order_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    PortingReviewIssue* find(const std::string& issueId) {
        auto it = issues_.find(issueId);
        if (it == issues_.end()) return nullptr;
        return &it->second;
    }
};
