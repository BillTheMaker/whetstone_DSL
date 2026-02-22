#pragma once
// Step 706: unsafe block risk packet generator.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct RustUnsafeRiskPacket {
    int unsafeBlockCount = 0;
    int rawPointerOps = 0;
    int ffiCalls = 0;
    bool reviewRequired = false;
    float confidence = 1.0f;
    std::vector<std::string> reasons;
};

class RustUnsafeRiskPacketGenerator {
public:
    static RustUnsafeRiskPacket analyze(const std::string& src) {
        RustUnsafeRiskPacket p;
        p.unsafeBlockCount = count(src, "unsafe {");
        p.rawPointerOps = count(src, "*const") + count(src, "*mut");
        p.ffiCalls = count(src, "extern \"C\"");

        if (p.unsafeBlockCount > 0) p.reasons.push_back("unsafe_blocks_present");
        if (p.rawPointerOps > 0) p.reasons.push_back("raw_pointer_ops_present");
        if (p.ffiCalls > 0) p.reasons.push_back("ffi_boundary_present");

        p.reviewRequired = !p.reasons.empty();
        p.confidence = p.reviewRequired ? 0.65f : 0.95f;
        return p;
    }

    static nlohmann::json toJson(const RustUnsafeRiskPacket& p) {
        return {
            {"unsafeBlockCount", p.unsafeBlockCount},
            {"rawPointerOps", p.rawPointerOps},
            {"ffiCalls", p.ffiCalls},
            {"reviewRequired", p.reviewRequired},
            {"confidence", p.confidence},
            {"reasons", p.reasons}
        };
    }

private:
    static int count(const std::string& s, const std::string& needle) {
        int out = 0;
        size_t p = 0;
        while (true) {
            p = s.find(needle, p);
            if (p == std::string::npos) break;
            ++out;
            p += needle.size();
        }
        return out;
    }
};
