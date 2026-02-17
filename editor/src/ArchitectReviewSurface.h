#pragma once
// Step 577: Architect Review Surface for Intake

#include "RequirementNormalizationConflictDetector.h"

#include <map>
#include <string>
#include <vector>

enum class ReviewDecision {
    Pending,
    Approved,
    Modified,
    Rejected
};

struct ConstraintReviewItem {
    std::string requirementId;
    std::string originalText;
    std::string reviewedText;
    ReviewDecision decision = ReviewDecision::Pending;
    std::string reviewer;
};

struct ConstraintReviewSummary {
    int pending = 0;
    int approved = 0;
    int modified = 0;
    int rejected = 0;
    bool allDecided = false;
};

class ArchitectReviewSurface {
public:
    bool loadConstraints(const std::vector<NormalizedRequirement>& requirements, std::string* error) {
        if (!error) return false;
        error->clear();
        items_.clear();
        for (const auto& requirement : requirements) {
            if (requirement.kind != NormalizedRequirementKind::Constraint) continue;
            ConstraintReviewItem item;
            item.requirementId = requirement.requirementId;
            item.originalText = requirement.normalizedText;
            item.reviewedText = requirement.normalizedText;
            items_[item.requirementId] = item;
        }
        if (items_.empty()) return fail(error, "no_constraints_to_review");
        return true;
    }

    bool approve(const std::string& requirementId,
                 const std::string& reviewer,
                 std::string* error) {
        if (!error) return false;
        error->clear();
        auto* item = find(requirementId);
        if (!item) return fail(error, "constraint_not_found");
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        item->decision = ReviewDecision::Approved;
        item->reviewer = reviewer;
        item->reviewedText = item->originalText;
        return true;
    }

    bool modify(const std::string& requirementId,
                const std::string& reviewedText,
                const std::string& reviewer,
                std::string* error) {
        if (!error) return false;
        error->clear();
        auto* item = find(requirementId);
        if (!item) return fail(error, "constraint_not_found");
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        if (reviewedText.empty()) return fail(error, "reviewed_text_missing");
        item->decision = ReviewDecision::Modified;
        item->reviewer = reviewer;
        item->reviewedText = reviewedText;
        return true;
    }

    bool reject(const std::string& requirementId,
                const std::string& reviewer,
                std::string* error) {
        if (!error) return false;
        error->clear();
        auto* item = find(requirementId);
        if (!item) return fail(error, "constraint_not_found");
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        item->decision = ReviewDecision::Rejected;
        item->reviewer = reviewer;
        return true;
    }

    ConstraintReviewSummary summarize() const {
        ConstraintReviewSummary summary;
        for (const auto& kv : items_) {
            const auto decision = kv.second.decision;
            if (decision == ReviewDecision::Pending) ++summary.pending;
            if (decision == ReviewDecision::Approved) ++summary.approved;
            if (decision == ReviewDecision::Modified) ++summary.modified;
            if (decision == ReviewDecision::Rejected) ++summary.rejected;
        }
        summary.allDecided = summary.pending == 0 && !items_.empty();
        return summary;
    }

    std::vector<ConstraintReviewItem> items() const {
        std::vector<ConstraintReviewItem> out;
        for (const auto& kv : items_) out.push_back(kv.second);
        return out;
    }

private:
    std::map<std::string, ConstraintReviewItem> items_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    ConstraintReviewItem* find(const std::string& requirementId) {
        auto it = items_.find(requirementId);
        if (it == items_.end()) return nullptr;
        return &it->second;
    }
};
