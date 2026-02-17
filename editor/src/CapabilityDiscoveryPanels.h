#pragma once
// Step 586: Capability Discovery Panels

#include "ProductizationValidationUtil.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct CapabilityCategory {
    std::string categoryId;
    std::string title;
    int operationCount = 0;
    std::vector<std::string> recommendations;
};

class CapabilityDiscoveryPanels {
public:
    bool registerCategory(const std::string& categoryId,
                          const std::string& title,
                          std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredProductIds(categoryId, title)) {
            if (categoryId.empty()) return fail(error, "category_id_missing");
            return fail(error, "category_title_missing");
        }
        if (categories_.count(categoryId) != 0) return fail(error, "category_duplicate");
        CapabilityCategory c;
        c.categoryId = categoryId;
        c.title = title;
        categories_[categoryId] = c;
        return true;
    }

    bool recordOperation(const std::string& categoryId, std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = categories_.find(categoryId);
        if (it == categories_.end()) return fail(error, "category_missing");
        ++it->second.operationCount;
        return true;
    }

    bool addRecommendation(const std::string& categoryId,
                           const std::string& recommendation,
                           std::string* error) {
        if (!error) return false;
        error->clear();
        auto it = categories_.find(categoryId);
        if (it == categories_.end()) return fail(error, "category_missing");
        if (recommendation.empty()) return fail(error, "recommendation_missing");
        for (const auto& r : it->second.recommendations) {
            if (r == recommendation) return fail(error, "recommendation_duplicate");
        }
        it->second.recommendations.push_back(recommendation);
        return true;
    }

    std::vector<CapabilityCategory> sortedByOperationCountDesc() const {
        std::vector<CapabilityCategory> out;
        for (const auto& kv : categories_) out.push_back(kv.second);
        std::stable_sort(out.begin(), out.end(), [](const CapabilityCategory& a, const CapabilityCategory& b) {
            if (a.operationCount != b.operationCount) return a.operationCount > b.operationCount;
            return a.categoryId < b.categoryId;
        });
        return out;
    }

    std::vector<std::string> recommendationsFor(const std::string& categoryId) const {
        auto it = categories_.find(categoryId);
        if (it == categories_.end()) return {};
        return it->second.recommendations;
    }

    int operationCountFor(const std::string& categoryId) const {
        auto it = categories_.find(categoryId);
        if (it == categories_.end()) return 0;
        return it->second.operationCount;
    }

private:
    std::map<std::string, CapabilityCategory> categories_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
