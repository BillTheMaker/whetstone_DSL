#pragma once
// Step 715: STL algorithm lifting from intent tags.

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

class CppAlgorithmLifting {
public:
    static std::set<std::string> lift(const SemanticCoreIR& ir) {
        std::set<std::string> out;
        for (const auto& n : ir.nodes) {
            for (const auto& t : n.intentTags) {
                if (t == "algorithmic") out.insert("std::transform");
                if (t == "pure") out.insert("std::accumulate");
                if (t == "io") out.insert("std::for_each");
            }
        }
        return out;
    }

    static nlohmann::json toJson(const std::set<std::string>& algos) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& a : algos) j.push_back(a);
        return j;
    }
};
