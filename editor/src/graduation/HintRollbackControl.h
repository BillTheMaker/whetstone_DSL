#pragma once
// Step 864: Hint rollback and suppression controls.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct HintSuppression {
    std::string suppressionId;
    std::string pairId;
    std::string reason;
    bool active = true;
};

class HintRollbackControl {
public:
    bool suppress(const HintSuppression& s, std::string* error = nullptr) {
        if (s.pairId.empty()) { if (error) *error = "pair_id_missing"; return false; }
        if (s.reason.empty()) { if (error) *error = "reason_missing"; return false; }
        suppressions_.push_back(s);
        return true;
    }

    bool isSuppressed(const std::string& pairId) const {
        for (const auto& s : suppressions_)
            if (s.pairId == pairId && s.active) return true;
        return false;
    }

    bool lift(const std::string& pairId, std::string* error = nullptr) {
        for (auto& s : suppressions_)
            if (s.pairId == pairId && s.active) { s.active = false; return true; }
        if (error) *error = "suppression_not_found";
        return false;
    }

    int activeSuppressionsCount() const {
        int n = 0;
        for (const auto& s : suppressions_) if (s.active) ++n;
        return n;
    }

    static nlohmann::json toJson(const HintSuppression& s) {
        return {{"suppression_id", s.suppressionId}, {"pair_id", s.pairId},
                {"reason", s.reason}, {"active", s.active}};
    }

private:
    std::vector<HintSuppression> suppressions_;
};
