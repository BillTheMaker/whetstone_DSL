#pragma once
// Step 606: Rollback Drill Tracker

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

enum class DrillOutcome {
    Scheduled,
    Passed,
    Failed
};

struct RollbackDrillRecord {
    std::string drillId;
    std::string environmentId;
    int recoveryMinutes = 0;
    DrillOutcome outcome = DrillOutcome::Scheduled;
    std::string notes;
};

class RollbackDrillTracker {
public:
    bool schedule(const RollbackDrillRecord& drill, std::string* error) {
        if (!error) return false;
        error->clear();
        if (drill.drillId.empty()) return failWith(error, "drill_id_missing");
        if (drill.environmentId.empty()) return failWith(error, "environment_id_missing");
        if (drill.recoveryMinutes < 0) return failWith(error, "recovery_minutes_invalid");
        if (drill.outcome != DrillOutcome::Scheduled) return failWith(error, "outcome_invalid");
        if (records_.count(drill.drillId) != 0) return failWith(error, "drill_duplicate");
        records_[drill.drillId] = drill;
        order_.push_back(drill.drillId);
        return true;
    }

    bool complete(const std::string& drillId,
                  DrillOutcome outcome,
                  int recoveryMinutes,
                  const std::string& notes,
                  std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = records_.find(drillId);
        if (it == records_.end()) return failWith(error, "drill_missing");
        if (outcome == DrillOutcome::Scheduled) return failWith(error, "outcome_invalid");
        if (recoveryMinutes < 0) return failWith(error, "recovery_minutes_invalid");
        it->second.outcome = outcome;
        it->second.recoveryMinutes = recoveryMinutes;
        it->second.notes = notes;
        return true;
    }

    int passCount() const {
        int count = 0;
        for (const auto& id : order_) if (records_.at(id).outcome == DrillOutcome::Passed) ++count;
        return count;
    }

    int failCount() const {
        int count = 0;
        for (const auto& id : order_) if (records_.at(id).outcome == DrillOutcome::Failed) ++count;
        return count;
    }

    int averageRecoveryMinutes() const {
        int total = 0;
        int count = 0;
        for (const auto& id : order_) {
            const auto& record = records_.at(id);
            if (record.outcome == DrillOutcome::Passed || record.outcome == DrillOutcome::Failed) {
                total += record.recoveryMinutes;
                ++count;
            }
        }
        return count == 0 ? 0 : total / count;
    }

    std::vector<RollbackDrillRecord> byEnvironment(const std::string& environmentId) const {
        std::vector<RollbackDrillRecord> out;
        for (const auto& id : order_) {
            const auto& record = records_.at(id);
            if (environmentId.empty() || record.environmentId == environmentId) out.push_back(record);
        }
        return out;
    }

private:
    std::map<std::string, RollbackDrillRecord> records_;
    std::vector<std::string> order_;
};
