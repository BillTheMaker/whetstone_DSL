#pragma once
// Step 720: deterministic test vector specification format.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct TestVector {
    std::string id;
    nlohmann::json input;
    nlohmann::json expected;
};

class TestVectorSpec {
public:
    static std::vector<TestVector> normalize(std::vector<TestVector> vectors) {
        std::sort(vectors.begin(), vectors.end(), [](const TestVector& a, const TestVector& b) {
            return a.id < b.id;
        });
        return vectors;
    }

    static nlohmann::json toJson(const std::vector<TestVector>& vectors) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& v : vectors) {
            arr.push_back({{"id", v.id}, {"input", v.input}, {"expected", v.expected}});
        }
        return arr;
    }

    static std::vector<TestVector> fromJson(const nlohmann::json& j) {
        std::vector<TestVector> out;
        if (!j.is_array()) return out;
        for (const auto& v : j) {
            out.push_back({
                v.value("id", ""),
                v.value("input", nlohmann::json::object()),
                v.value("expected", nlohmann::json::object())
            });
        }
        return normalize(std::move(out));
    }
};
