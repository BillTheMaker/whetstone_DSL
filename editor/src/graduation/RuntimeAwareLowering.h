#pragma once
// Step 892: Runtime-aware lowering extension points.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct LoweringExtensionPoint {
    std::string pointId;
    std::string runtimeId;
    std::string hook;        // e.g. "pre_lower", "post_lower", "emit_guard"
    bool active = false;
};

struct RuntimeLoweringPlan {
    std::string pairId;
    std::string targetRuntime;
    std::vector<LoweringExtensionPoint> points;
    bool runtimePackAvailable = false;
};

class RuntimeAwareLowering {
public:
    static RuntimeLoweringPlan plan(const std::string& pairId,
                                     const std::string& targetRuntime,
                                     bool packAvailable) {
        RuntimeLoweringPlan p;
        p.pairId = pairId;
        p.targetRuntime = targetRuntime;
        p.runtimePackAvailable = packAvailable;
        if (packAvailable) {
            p.points.push_back({"EP-1", targetRuntime, "pre_lower", true});
            p.points.push_back({"EP-2", targetRuntime, "post_lower", true});
        } else {
            p.points.push_back({"EP-1", "generic", "pre_lower", false});
        }
        return p;
    }

    static nlohmann::json toJson(const RuntimeLoweringPlan& p) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& ep : p.points)
            arr.push_back({{"point_id", ep.pointId}, {"hook", ep.hook},
                           {"active", ep.active}});
        return {{"pair_id", p.pairId}, {"target_runtime", p.targetRuntime},
                {"runtime_pack_available", p.runtimePackAvailable},
                {"extension_points", arr}};
    }
};
