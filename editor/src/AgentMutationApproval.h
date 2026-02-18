#pragma once
// Step 627: Inline accept/reject/modify controls

#include <string>
#include <vector>

enum class MutationDecision {
    Pending,
    Accepted,
    Rejected,
    Modified
};

struct MutationApprovalRecord {
    std::string previewId;
    MutationDecision decision = MutationDecision::Pending;
    std::string reason;
    std::string modifiedMutationJson;
};

class AgentMutationApproval {
public:
    static void ensureRecord(std::vector<MutationApprovalRecord>* records,
                             const std::string& previewId) {
        if (!records) return;
        if (findMutable(records, previewId)) return;
        records->push_back({previewId, MutationDecision::Pending, "", ""});
    }

    static bool accept(std::vector<MutationApprovalRecord>* records,
                       const std::string& previewId) {
        auto* record = ensureAndGet(records, previewId);
        if (!record) return false;
        record->decision = MutationDecision::Accepted;
        record->reason.clear();
        return true;
    }

    static bool reject(std::vector<MutationApprovalRecord>* records,
                       const std::string& previewId,
                       const std::string& reason) {
        auto* record = ensureAndGet(records, previewId);
        if (!record || reason.empty()) return false;
        record->decision = MutationDecision::Rejected;
        record->reason = reason;
        return true;
    }

    static bool modify(std::vector<MutationApprovalRecord>* records,
                       const std::string& previewId,
                       const std::string& modifiedMutationJson) {
        auto* record = ensureAndGet(records, previewId);
        if (!record || modifiedMutationJson.empty()) return false;
        record->decision = MutationDecision::Modified;
        record->modifiedMutationJson = modifiedMutationJson;
        record->reason.clear();
        return true;
    }

    static const MutationApprovalRecord* find(const std::vector<MutationApprovalRecord>& records,
                                              const std::string& previewId) {
        for (const auto& record : records) {
            if (record.previewId == previewId) return &record;
        }
        return nullptr;
    }

    static std::string decisionText(MutationDecision decision) {
        if (decision == MutationDecision::Pending) return "pending";
        if (decision == MutationDecision::Accepted) return "accepted";
        if (decision == MutationDecision::Rejected) return "rejected";
        return "modified";
    }

    static std::string rejectionMessage(const MutationApprovalRecord& record) {
        return "rejected: " + record.reason;
    }

private:
    static MutationApprovalRecord* ensureAndGet(std::vector<MutationApprovalRecord>* records,
                                                const std::string& previewId) {
        if (!records || previewId.empty()) return nullptr;
        ensureRecord(records, previewId);
        return findMutable(records, previewId);
    }

    static MutationApprovalRecord* findMutable(std::vector<MutationApprovalRecord>* records,
                                               const std::string& previewId) {
        if (!records) return nullptr;
        for (auto& record : *records) {
            if (record.previewId == previewId) return &record;
        }
        return nullptr;
    }
};
