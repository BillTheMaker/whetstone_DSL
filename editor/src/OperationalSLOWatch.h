#pragma once
// Step 607: Operational SLO Watch

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

struct SLOIndicator {
    std::string indicatorId;
    std::string serviceId;
    int targetPercent = 0;
    int observedPercent = 0;
    int errorBudgetBurn = 0;
};

class OperationalSLOWatch {
public:
    bool upsert(const SLOIndicator& indicator, std::string* error) {
        if (!error) return false;
        error->clear();
        if (indicator.indicatorId.empty()) return failWith(error, "indicator_id_missing");
        if (indicator.serviceId.empty()) return failWith(error, "service_id_missing");
        if (indicator.targetPercent < 0 || indicator.targetPercent > 100) return failWith(error, "target_percent_invalid");
        if (indicator.observedPercent < 0 || indicator.observedPercent > 100) return failWith(error, "observed_percent_invalid");
        if (indicator.errorBudgetBurn < 0) return failWith(error, "error_budget_burn_invalid");
        if (items_.count(indicator.indicatorId) == 0) order_.push_back(indicator.indicatorId);
        items_[indicator.indicatorId] = indicator;
        return true;
    }

    bool breaching(const std::string& indicatorId, std::string* error) const {
        if (!error) return false;
        error->clear();
        auto it = items_.find(indicatorId);
        if (it == items_.end()) return failWith(error, "indicator_missing");
        const auto& i = it->second;
        return i.observedPercent < i.targetPercent || i.errorBudgetBurn > 100;
    }

    int serviceBreachCount(const std::string& serviceId) const {
        int count = 0;
        for (const auto& id : order_) {
            const auto& i = items_.at(id);
            if (i.serviceId == serviceId &&
                (i.observedPercent < i.targetPercent || i.errorBudgetBurn > 100)) ++count;
        }
        return count;
    }

    std::vector<SLOIndicator> byService(const std::string& serviceId) const {
        std::vector<SLOIndicator> out;
        for (const auto& id : order_) {
            const auto& i = items_.at(id);
            if (serviceId.empty() || i.serviceId == serviceId) out.push_back(i);
        }
        return out;
    }

private:
    std::map<std::string, SLOIndicator> items_;
    std::vector<std::string> order_;
};
