#pragma once
// Step 1539: debug checklist item model.

#include <string>

#include <nlohmann/json.hpp>

struct DebugChecklistItem {
    int index = 0;
    std::string text;
    bool done = false;
};

class DebugChecklistItemModel {
public:
    static DebugChecklistItem make(int index, const std::string& text, bool done) {
        return {index, text, done};
    }

    static nlohmann::json toJson(const DebugChecklistItem& i) {
        return {{"index", i.index}, {"text", i.text}, {"done", i.done}};
    }
};
