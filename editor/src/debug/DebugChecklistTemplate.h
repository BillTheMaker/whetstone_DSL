#pragma once
// Step 1540: debug checklist template model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "DebugChecklistItem.h"

struct DebugChecklistTemplate {
    std::string failureClass;
    std::vector<DebugChecklistItem> items;
};

class DebugChecklistTemplateModel {
public:
    static DebugChecklistTemplate build(const std::string& failureClass) {
        DebugChecklistTemplate t;
        t.failureClass = failureClass;
        if (failureClass == "compile") {
            t.items = {
                DebugChecklistItemModel::make(1, "Reproduce compile error", false),
                DebugChecklistItemModel::make(2, "Patch smallest compile unit", false),
                DebugChecklistItemModel::make(3, "Run compile regression guard", false)
            };
        } else if (failureClass == "test") {
            t.items = {
                DebugChecklistItemModel::make(1, "Reproduce failing test", false),
                DebugChecklistItemModel::make(2, "Minimize fixture/mocks", false),
                DebugChecklistItemModel::make(3, "Run targeted + guard tests", false)
            };
        } else {
            t.items = {
                DebugChecklistItemModel::make(1, "Capture fresh failure packet", false),
                DebugChecklistItemModel::make(2, "Select top root-cause cluster", false),
                DebugChecklistItemModel::make(3, "Apply guarded fix and verify", false)
            };
        }
        return t;
    }

    static nlohmann::json toJson(const DebugChecklistTemplate& t) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& i : t.items) arr.push_back(DebugChecklistItemModel::toJson(i));
        return {{"failure_class", t.failureClass}, {"items", arr}};
    }
};
