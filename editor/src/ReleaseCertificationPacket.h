#pragma once
// Step 604: Release Certification Packet

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

struct CertificationCheck {
    std::string checkId;
    std::string category;
    bool passed = false;
    std::string evidenceRef;
};

class ReleaseCertificationPacket {
public:
    bool recordCheck(const CertificationCheck& check, std::string* error) {
        if (!error) return false;
        error->clear();
        if (check.checkId.empty()) return failWith(error, "check_id_missing");
        if (check.category.empty()) return failWith(error, "category_missing");
        if (check.evidenceRef.empty()) return failWith(error, "evidence_ref_missing");
        if (checks_.count(check.checkId) != 0) return failWith(error, "check_duplicate");
        checks_[check.checkId] = check;
        order_.push_back(check.checkId);
        return true;
    }

    bool setResult(const std::string& checkId, bool passed, std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = checks_.find(checkId);
        if (it == checks_.end()) return failWith(error, "check_missing");
        it->second.passed = passed;
        return true;
    }

    int passedCount() const {
        int count = 0;
        for (const auto& id : order_) if (checks_.at(id).passed) ++count;
        return count;
    }

    int failedCount() const {
        int count = 0;
        for (const auto& id : order_) if (!checks_.at(id).passed) ++count;
        return count;
    }

    bool allPassed() const {
        return !order_.empty() && failedCount() == 0;
    }

    std::vector<CertificationCheck> byCategory(const std::string& category) const {
        std::vector<CertificationCheck> out;
        for (const auto& id : order_) {
            const auto& check = checks_.at(id);
            if (category.empty() || check.category == category) out.push_back(check);
        }
        return out;
    }

private:
    std::map<std::string, CertificationCheck> checks_;
    std::vector<std::string> order_;
};
