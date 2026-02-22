#pragma once
// Step 804: Memory layout compatibility checker.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

struct LayoutPacket {
    std::string layout;
    bool compatible = true;
    std::string reason;
};

class MemoryLayoutChecker {
public:
    static LayoutPacket assess(const AbiPacket& a, const AbiPacket& b) {
        LayoutPacket p;
        p.layout = a.memoryLayout + "/" + b.memoryLayout;
        if (a.memoryLayout != b.memoryLayout) {
            p.compatible = false;
            p.reason = "layout_mismatch";
        }
        return p;
    }

    static nlohmann::json toJson(const LayoutPacket& p) {
        return {{"layout", p.layout}, {"compatible", p.compatible}, {"reason", p.reason}};
    }
};
