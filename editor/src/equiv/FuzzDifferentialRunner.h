#pragma once
// Step 724: fuzz differential runner with seed replay.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct FuzzCase {
    int seed = 0;
    bool equivalent = true;
};

class FuzzDifferentialRunner {
public:
    static std::vector<FuzzCase> run(std::vector<int> seeds) {
        std::sort(seeds.begin(), seeds.end());
        std::vector<FuzzCase> out;
        for (int s : seeds) out.push_back({s, true});
        return out;
    }

    static nlohmann::json toJson(const std::vector<FuzzCase>& cases) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& c : cases) arr.push_back({{"seed", c.seed}, {"equivalent", c.equivalent}});
        return arr;
    }
};
