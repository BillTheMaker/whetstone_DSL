#pragma once
// Step 711: trait-to-interface/composition raising.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppTraitArtifact {
    std::string name;
    std::string kind; // interface/composition
};

class CppTraitRaising {
public:
    static std::vector<CppTraitArtifact> raise(const SemanticCoreIR& ir,
                                               const std::string& profile = "safe-first") {
        std::vector<CppTraitArtifact> out;
        for (const auto& n : ir.nodes) {
            if (n.kind != IRNodeKind::Type) continue;
            if (!n.annotations.contains("trait")) continue;
            CppTraitArtifact a;
            a.name = n.name;
            a.kind = profile == "perf-first" ? "composition" : "interface";
            out.push_back(std::move(a));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<CppTraitArtifact>& v) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& a : v) j.push_back({{"name", a.name}, {"kind", a.kind}});
        return j;
    }
};
