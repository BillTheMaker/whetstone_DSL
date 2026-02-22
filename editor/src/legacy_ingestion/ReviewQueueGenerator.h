#pragma once
// Step 813: Human review queue generator from ambiguities.

#include <vector>
#include <string>

#include <nlohmann/json.hpp>

struct ReviewItem {
    std::string id;
    std::string level;
    bool urgent;
};

class ReviewQueueGenerator {
public:
    static std::vector<ReviewItem> build(const std::vector<std::string>& ambiguityIds) {
        std::vector<ReviewItem> queue;
        for (size_t i = 0; i < ambiguityIds.size(); ++i) {
            queue.push_back({ambiguityIds[i], i % 2 == 0 ? "high" : "medium", true});
        }
        return queue;
    }

    static nlohmann::json toJson(const ReviewItem& item) {
        return {{"id", item.id}, {"level", item.level}, {"urgent", item.urgent}};
    }
};
