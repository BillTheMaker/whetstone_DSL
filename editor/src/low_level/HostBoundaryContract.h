#pragma once
// Step 805: Host boundary + FFI contract generator.

#include <string>

#include <nlohmann/json.hpp>

#include "AbiCallingConventionModel.h"

struct HostContractPacket {
    bool hostBoundary = false;
    bool needsReview = false;
    std::string contract;
};

class HostBoundaryContract {
public:
    static HostContractPacket build(const AbiPacket& packet) {
        HostContractPacket out;
        out.hostBoundary = packet.hostBoundary;
        out.needsReview = packet.hostBoundary || packet.callingConvention == "stdcall";
        out.contract = packet.language + ":" + packet.callingConvention;
        return out;
    }

    static nlohmann::json toJson(const HostContractPacket& p) {
        return {{"host_boundary", p.hostBoundary}, {"needs_review", p.needsReview}, {"contract", p.contract}};
    }
};
