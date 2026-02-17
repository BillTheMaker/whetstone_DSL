#pragma once
// Step 611: On-Call Coverage Planner

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

struct OnCallShift {
    std::string shiftId;
    std::string serviceId;
    std::string primaryEngineer;
    std::string secondaryEngineer;
    int startHour = 0;
    int endHour = 0;
};

class OnCallCoveragePlanner {
public:
    bool addShift(const OnCallShift& shift, std::string* error) {
        if (!error) return false;
        error->clear();
        if (shift.shiftId.empty()) return failWith(error, "shift_id_missing");
        if (shift.serviceId.empty()) return failWith(error, "service_id_missing");
        if (shift.primaryEngineer.empty()) return failWith(error, "primary_missing");
        if (shift.secondaryEngineer.empty()) return failWith(error, "secondary_missing");
        if (shift.startHour < 0 || shift.startHour > 23) return failWith(error, "start_hour_invalid");
        if (shift.endHour < 0 || shift.endHour > 24) return failWith(error, "end_hour_invalid");
        if (shift.endHour <= shift.startHour) return failWith(error, "shift_range_invalid");
        if (shifts_.count(shift.shiftId) != 0) return failWith(error, "shift_duplicate");
        shifts_[shift.shiftId] = shift;
        order_.push_back(shift.shiftId);
        return true;
    }

    bool covered(const std::string& serviceId, int hour, std::string* error) const {
        if (!error) return false;
        error->clear();
        if (serviceId.empty()) return failWith(error, "service_id_missing");
        if (hour < 0 || hour > 23) return failWith(error, "hour_invalid");
        for (const auto& id : order_) {
            const auto& s = shifts_.at(id);
            if (s.serviceId == serviceId && hour >= s.startHour && hour < s.endHour) return true;
        }
        return false;
    }

    int shiftCount(const std::string& serviceId) const {
        int count = 0;
        for (const auto& id : order_) {
            const auto& s = shifts_.at(id);
            if (serviceId.empty() || s.serviceId == serviceId) ++count;
        }
        return count;
    }

    int coverageGaps(const std::string& serviceId) const {
        if (serviceId.empty()) return 24;
        int gaps = 0;
        for (int h = 0; h < 24; ++h) {
            bool any = false;
            for (const auto& id : order_) {
                const auto& s = shifts_.at(id);
                if (s.serviceId == serviceId && h >= s.startHour && h < s.endHour) {
                    any = true;
                    break;
                }
            }
            if (!any) ++gaps;
        }
        return gaps;
    }

private:
    std::map<std::string, OnCallShift> shifts_;
    std::vector<std::string> order_;
};
