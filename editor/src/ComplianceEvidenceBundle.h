#pragma once
// Step 601: Compliance Evidence Bundle

#include <map>
#include <string>
#include <vector>

struct ComplianceEvidenceItem {
    std::string itemId;
    std::string controlId;
    std::string artifactPath;
    std::string summary;
};

class ComplianceEvidenceBundle {
public:
    bool addEvidence(const ComplianceEvidenceItem& item, std::string* error) {
        if (!error) return false;
        error->clear();
        if (item.itemId.empty()) return fail(error, "item_id_missing");
        if (item.controlId.empty()) return fail(error, "control_id_missing");
        if (item.artifactPath.empty()) return fail(error, "artifact_path_missing");
        if (item.summary.empty()) return fail(error, "summary_missing");
        if (items_.count(item.itemId) != 0) return fail(error, "item_duplicate");
        items_[item.itemId] = item;
        order_.push_back(item.itemId);
        return true;
    }

    std::vector<ComplianceEvidenceItem> byControl(const std::string& controlId) const {
        std::vector<ComplianceEvidenceItem> out;
        for (const auto& id : order_) {
            const auto& item = items_.at(id);
            if (controlId.empty() || item.controlId == controlId) out.push_back(item);
        }
        return out;
    }

    int controlCoverageCount() const {
        std::map<std::string, bool> controls;
        for (const auto& id : order_) controls[items_.at(id).controlId] = true;
        return static_cast<int>(controls.size());
    }

    int artifactCount() const {
        return static_cast<int>(order_.size());
    }

private:
    std::map<std::string, ComplianceEvidenceItem> items_;
    std::vector<std::string> order_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
