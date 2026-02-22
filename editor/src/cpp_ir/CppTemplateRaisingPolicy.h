#pragma once
// Step 712: generics/template raising policy.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppTemplateDecision {
    std::string nodeId;
    std::string templateForm; // class_template, fn_template, concrete
    bool conceptConstrained = false;
};

class CppTemplateRaisingPolicy {
public:
    static std::vector<CppTemplateDecision> raise(const SemanticCoreIR& ir,
                                                  const std::string& profile = "safe-first") {
        std::vector<CppTemplateDecision> out;
        for (const auto& n : ir.nodes) {
            if (!n.metadata.contains("genericParams")) continue;
            CppTemplateDecision d;
            d.nodeId = n.id;
            d.templateForm = (n.kind == IRNodeKind::Type) ? "class_template" : "fn_template";
            d.conceptConstrained = profile != "interop-first";
            out.push_back(std::move(d));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.nodeId < b.nodeId; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<CppTemplateDecision>& v) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& d : v) {
            j.push_back({{"nodeId", d.nodeId}, {"templateForm", d.templateForm}, {"conceptConstrained", d.conceptConstrained}});
        }
        return j;
    }
};
