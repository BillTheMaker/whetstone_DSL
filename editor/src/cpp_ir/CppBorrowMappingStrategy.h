#pragma once
// Step 710: borrow semantics to references/views.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppBorrowDecision {
    std::string fromId;
    std::string toId;
    std::string cppForm; // const_ref, mut_ref, span_view
};

class CppBorrowMappingStrategy {
public:
    static std::vector<CppBorrowDecision> map(const SemanticCoreIR& ir,
                                              const std::string& profile = "safe-first") {
        std::vector<CppBorrowDecision> out;
        for (const auto& e : ir.edges) {
            if (e.relation != "borrows" && e.relation != "borrows_mut") continue;
            CppBorrowDecision d;
            d.fromId = e.fromId;
            d.toId = e.toId;
            if (e.relation == "borrows_mut") d.cppForm = "mut_ref";
            else if (profile == "perf-first") d.cppForm = "span_view";
            else d.cppForm = "const_ref";
            out.push_back(std::move(d));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
            if (a.fromId != b.fromId) return a.fromId < b.fromId;
            return a.toId < b.toId;
        });
        return out;
    }

    static nlohmann::json toJson(const std::vector<CppBorrowDecision>& d) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& x : d) j.push_back({{"fromId", x.fromId}, {"toId", x.toId}, {"cppForm", x.cppForm}});
        return j;
    }
};
