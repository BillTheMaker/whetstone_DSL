#pragma once
// Step 834: Regression watchlist for promoted pairs.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct WatchlistEntry {
    std::string pairId;
    std::string riskReason;
    std::string addedAt;
    bool active = true;
};

struct WatchlistSummary {
    int totalEntries = 0;
    int activeEntries = 0;
    std::vector<WatchlistEntry> entries;
};

class RegressionWatchlist {
public:
    bool add(const WatchlistEntry& e, std::string* error = nullptr) {
        if (e.pairId.empty()) { if (error) *error = "pair_id_missing"; return false; }
        for (const auto& ex : entries_)
            if (ex.pairId == e.pairId && ex.active) {
                if (error) *error = "entry_duplicate";
                return false;
            }
        entries_.push_back(e);
        return true;
    }

    bool remove(const std::string& pairId, std::string* error = nullptr) {
        for (auto& e : entries_)
            if (e.pairId == pairId && e.active) { e.active = false; return true; }
        if (error) *error = "entry_not_found";
        return false;
    }

    WatchlistSummary summarize() const {
        WatchlistSummary s;
        s.entries = entries_;
        s.totalEntries = static_cast<int>(entries_.size());
        for (const auto& e : entries_) if (e.active) ++s.activeEntries;
        return s;
    }

    static nlohmann::json toJson(const WatchlistSummary& s) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : s.entries)
            arr.push_back({{"pair_id", e.pairId}, {"active", e.active}, {"risk", e.riskReason}});
        return {{"total", s.totalEntries}, {"active", s.activeEntries}, {"entries", arr}};
    }

private:
    std::vector<WatchlistEntry> entries_;
};
