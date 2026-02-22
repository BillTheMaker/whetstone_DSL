#pragma once
// Step 709: ownership mapping policy engine.

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SemanticCoreIR.h"

struct CppOwnershipDecision {
    std::string nodeId;
    std::string strategy; // unique_ptr, shared_ptr, value
    std::string reason;
};

class CppOwnershipMappingPolicy {
public:
    static std::vector<CppOwnershipDecision> map(const SemanticCoreIR& ir,
                                                 const std::string& profile = "safe-first") {
        std::vector<CppOwnershipDecision> out;
        for (const auto& n : ir.nodes) {
            if (n.kind != IRNodeKind::OwnershipRegion && n.kind != IRNodeKind::Type) continue;
            CppOwnershipDecision d;
            d.nodeId = n.id;
            if (hasTag(n, "shared")) {
                d.strategy = "shared_ptr";
                d.reason = "shared_access_detected";
            } else if (hasTag(n, "stateful") || profile == "safe-first") {
                d.strategy = "unique_ptr";
                d.reason = "exclusive_owner_default";
            } else {
                d.strategy = "value";
                d.reason = "perf_value_default";
            }
            if (profile == "interop-first" && d.strategy == "unique_ptr") {
                d.strategy = "value";
                d.reason = "interop_abi_friendly";
            }
            out.push_back(std::move(d));
        }
        std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.nodeId < b.nodeId; });
        return out;
    }

    static nlohmann::json toJson(const std::vector<CppOwnershipDecision>& d) {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& x : d) j.push_back({{"nodeId", x.nodeId}, {"strategy", x.strategy}, {"reason", x.reason}});
        return j;
    }

private:
    static bool hasTag(const IRNode& n, const std::string& tag) {
        for (const auto& t : n.intentTags) if (t == tag) return true;
        return false;
    }
};
