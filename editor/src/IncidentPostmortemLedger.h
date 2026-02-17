#pragma once
// Step 608: Incident Postmortem Ledger

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

enum class PostmortemStatus {
    Open,
    Completed
};

struct PostmortemRecord {
    std::string incidentId;
    std::string owner;
    int correctiveActions = 0;
    int completedActions = 0;
    PostmortemStatus status = PostmortemStatus::Open;
};

class IncidentPostmortemLedger {
public:
    bool open(const PostmortemRecord& record, std::string* error) {
        if (!error) return false;
        error->clear();
        if (record.incidentId.empty()) return failWith(error, "incident_id_missing");
        if (record.owner.empty()) return failWith(error, "owner_missing");
        if (record.correctiveActions < 0) return failWith(error, "corrective_actions_invalid");
        if (record.completedActions < 0) return failWith(error, "completed_actions_invalid");
        if (record.completedActions > record.correctiveActions) return failWith(error, "completed_actions_exceed_total");
        if (record.status != PostmortemStatus::Open) return failWith(error, "status_invalid");
        if (records_.count(record.incidentId) != 0) return failWith(error, "incident_duplicate");
        records_[record.incidentId] = record;
        order_.push_back(record.incidentId);
        return true;
    }

    bool updateActions(const std::string& incidentId,
                       int completedActions,
                       std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = records_.find(incidentId);
        if (it == records_.end()) return failWith(error, "incident_missing");
        if (completedActions < 0) return failWith(error, "completed_actions_invalid");
        if (completedActions > it->second.correctiveActions) return failWith(error, "completed_actions_exceed_total");
        it->second.completedActions = completedActions;
        if (it->second.completedActions == it->second.correctiveActions) {
            it->second.status = PostmortemStatus::Completed;
        } else {
            it->second.status = PostmortemStatus::Open;
        }
        return true;
    }

    int completionPercent(const std::string& incidentId, std::string* error) const {
        if (!error) return 0;
        error->clear();
        auto it = records_.find(incidentId);
        if (it == records_.end()) {
            failWith(error, "incident_missing");
            return 0;
        }
        if (it->second.correctiveActions == 0) return 100;
        return (it->second.completedActions * 100) / it->second.correctiveActions;
    }

    int openCount() const {
        int count = 0;
        for (const auto& id : order_) if (records_.at(id).status == PostmortemStatus::Open) ++count;
        return count;
    }

    int completedCount() const {
        int count = 0;
        for (const auto& id : order_) if (records_.at(id).status == PostmortemStatus::Completed) ++count;
        return count;
    }

private:
    std::map<std::string, PostmortemRecord> records_;
    std::vector<std::string> order_;
};
