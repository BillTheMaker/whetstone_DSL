#pragma once
// Step 764: Async model bridge across managed-family targets.

#include <string>

#include <nlohmann/json.hpp>

#include "ManagedPacketTypes.h"

struct AsyncBridgePacket {
    std::string sourceModel;
    std::string targetModel;
    std::string bridgeStrategy;
    bool runtimeShimRequired = false;
};

class AsyncModelBridge {
public:
    static AsyncBridgePacket plan(const std::string& sourceLang,
                                  const std::string& targetLang,
                                  const ManagedLoweringPacket& p) {
        AsyncBridgePacket out;
        out.sourceModel = (sourceLang == "kotlin") ? "coroutines" : "task";
        out.targetModel = (targetLang == "kotlin") ? "coroutines" : "task";
        out.runtimeShimRequired = p.asyncSignalCount > 0 && out.sourceModel != out.targetModel;
        out.bridgeStrategy = out.runtimeShimRequired ? "adapter_shim" : "direct_mapping";
        return out;
    }

    static nlohmann::json toJson(const AsyncBridgePacket& p) {
        return {
            {"source_model", p.sourceModel},
            {"target_model", p.targetModel},
            {"bridge_strategy", p.bridgeStrategy},
            {"runtime_shim_required", p.runtimeShimRequired}
        };
    }
};
