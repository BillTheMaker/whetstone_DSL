#pragma once
// Step 609: Change Freeze Calendar

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

struct FreezeWindow {
    std::string windowId;
    std::string environmentId;
    int startDay = 0;
    int endDay = 0;
    std::string reason;
};

class ChangeFreezeCalendar {
public:
    bool addWindow(const FreezeWindow& window, std::string* error) {
        if (!error) return false;
        error->clear();
        if (window.windowId.empty()) return failWith(error, "window_id_missing");
        if (window.environmentId.empty()) return failWith(error, "environment_id_missing");
        if (window.startDay < 0) return failWith(error, "start_day_invalid");
        if (window.endDay < 0) return failWith(error, "end_day_invalid");
        if (window.endDay < window.startDay) return failWith(error, "window_range_invalid");
        if (window.reason.empty()) return failWith(error, "reason_missing");
        if (windows_.count(window.windowId) != 0) return failWith(error, "window_duplicate");
        windows_[window.windowId] = window;
        order_.push_back(window.windowId);
        return true;
    }

    bool frozen(const std::string& environmentId, int day, std::string* error) const {
        if (!error) return false;
        error->clear();
        if (environmentId.empty()) return failWith(error, "environment_id_missing");
        if (day < 0) return failWith(error, "day_invalid");
        for (const auto& id : order_) {
            const auto& w = windows_.at(id);
            if (w.environmentId == environmentId && day >= w.startDay && day <= w.endDay) return true;
        }
        return false;
    }

    int windowCount(const std::string& environmentId) const {
        int count = 0;
        for (const auto& id : order_) {
            if (environmentId.empty() || windows_.at(id).environmentId == environmentId) ++count;
        }
        return count;
    }

    std::vector<FreezeWindow> activeOnDay(int day) const {
        std::vector<FreezeWindow> out;
        for (const auto& id : order_) {
            const auto& w = windows_.at(id);
            if (day >= w.startDay && day <= w.endDay) out.push_back(w);
        }
        return out;
    }

private:
    std::map<std::string, FreezeWindow> windows_;
    std::vector<std::string> order_;
};
