#pragma once
// Step 820: Ambiguity triage UI model and API.

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class AmbiguitySeverity { Low, Medium, High };
enum class AmbiguityStatus { Open, UnderReview, Resolved, Deferred };

struct AmbiguityItem {
    std::string itemId;
    std::string description;
    std::string location;
    AmbiguitySeverity severity = AmbiguitySeverity::Medium;
    AmbiguityStatus status = AmbiguityStatus::Open;
    std::string assignedTo;
    std::string resolution;
};

struct AmbiguityTriageSummary {
    int total = 0;
    int open = 0;
    int underReview = 0;
    int resolved = 0;
    int deferred = 0;
    bool hasHighSeverityOpen = false;
};

class AmbiguityTriageModel {
public:
    bool addItem(const AmbiguityItem& item, std::string* error) {
        if (!error) return false;
        error->clear();
        if (item.itemId.empty()) return fail(error, "item_id_missing");
        if (item.description.empty()) return fail(error, "description_missing");
        if (items_.count(item.itemId)) return fail(error, "item_duplicate");
        items_[item.itemId] = item;
        order_.push_back(item.itemId);
        return true;
    }

    bool assign(const std::string& itemId, const std::string& reviewer, std::string* error) {
        if (!error) return false;
        error->clear();
        if (reviewer.empty()) return fail(error, "reviewer_missing");
        auto* it = find(itemId);
        if (!it) return fail(error, "item_not_found");
        it->assignedTo = reviewer;
        it->status = AmbiguityStatus::UnderReview;
        return true;
    }

    bool resolve(const std::string& itemId, const std::string& resolution, std::string* error) {
        if (!error) return false;
        error->clear();
        if (resolution.empty()) return fail(error, "resolution_missing");
        auto* it = find(itemId);
        if (!it) return fail(error, "item_not_found");
        if (it->assignedTo.empty()) return fail(error, "not_assigned");
        it->resolution = resolution;
        it->status = AmbiguityStatus::Resolved;
        return true;
    }

    bool defer(const std::string& itemId, std::string* error) {
        if (!error) return false;
        error->clear();
        auto* it = find(itemId);
        if (!it) return fail(error, "item_not_found");
        it->status = AmbiguityStatus::Deferred;
        return true;
    }

    AmbiguityTriageSummary summarize() const {
        AmbiguityTriageSummary s;
        for (const auto& id : order_) {
            const auto& item = items_.at(id);
            ++s.total;
            if (item.severity == AmbiguitySeverity::High && item.status == AmbiguityStatus::Open)
                s.hasHighSeverityOpen = true;
            switch (item.status) {
                case AmbiguityStatus::Open: ++s.open; break;
                case AmbiguityStatus::UnderReview: ++s.underReview; break;
                case AmbiguityStatus::Resolved: ++s.resolved; break;
                case AmbiguityStatus::Deferred: ++s.deferred; break;
            }
        }
        return s;
    }

    std::vector<AmbiguityItem> items() const {
        std::vector<AmbiguityItem> out;
        for (const auto& id : order_) out.push_back(items_.at(id));
        return out;
    }

    static nlohmann::json toJson(const AmbiguityTriageSummary& s) {
        return {{"total", s.total}, {"open", s.open}, {"under_review", s.underReview},
                {"resolved", s.resolved}, {"deferred", s.deferred},
                {"has_high_severity_open", s.hasHighSeverityOpen}};
    }

private:
    std::map<std::string, AmbiguityItem> items_;
    std::vector<std::string> order_;

    static bool fail(std::string* e, const char* code) { *e = code; return false; }
    AmbiguityItem* find(const std::string& id) {
        auto it = items_.find(id);
        return it == items_.end() ? nullptr : &it->second;
    }
};
