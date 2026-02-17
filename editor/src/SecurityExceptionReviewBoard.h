#pragma once
// Step 600: Security Exception Review Board

#include <map>
#include <string>
#include <vector>

enum class ExceptionDecision {
    Pending,
    Approved,
    Rejected
};

struct SecurityExceptionCase {
    std::string caseId;
    std::string operation;
    std::string requester;
    std::string justification;
    ExceptionDecision decision = ExceptionDecision::Pending;
    std::string reviewer;
};

class SecurityExceptionReviewBoard {
public:
    bool submit(const std::string& caseId,
                const std::string& operation,
                const std::string& requester,
                const std::string& justification,
                std::string* error) {
        if (!error) return false;
        error->clear();
        if (caseId.empty()) return fail(error, "case_id_missing");
        if (operation.empty()) return fail(error, "operation_missing");
        if (requester.empty()) return fail(error, "requester_missing");
        if (justification.empty()) return fail(error, "justification_missing");
        if (cases_.count(caseId) != 0) return fail(error, "case_duplicate");
        cases_[caseId] = {caseId, operation, requester, justification, ExceptionDecision::Pending, ""};
        order_.push_back(caseId);
        return true;
    }

    bool decide(const std::string& caseId,
                ExceptionDecision decision,
                const std::string& reviewer,
                std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = cases_.find(caseId);
        if (it == cases_.end()) return fail(error, "case_missing");
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        if (decision == ExceptionDecision::Pending) return fail(error, "decision_invalid");
        it->second.decision = decision;
        it->second.reviewer = reviewer;
        return true;
    }

    std::vector<SecurityExceptionCase> pending() const {
        std::vector<SecurityExceptionCase> out;
        for (const auto& id : order_) {
            const auto& c = cases_.at(id);
            if (c.decision == ExceptionDecision::Pending) out.push_back(c);
        }
        return out;
    }

    int approvedCount() const {
        int n = 0;
        for (const auto& id : order_) if (cases_.at(id).decision == ExceptionDecision::Approved) ++n;
        return n;
    }

    int rejectedCount() const {
        int n = 0;
        for (const auto& id : order_) if (cases_.at(id).decision == ExceptionDecision::Rejected) ++n;
        return n;
    }

private:
    std::map<std::string, SecurityExceptionCase> cases_;
    std::vector<std::string> order_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
