#pragma once
// Step 799: ABI and calling-convention canonical model.

#include <string>

#include <nlohmann/json.hpp>

struct AbiPacket {
    std::string language;
    std::string callingConvention;
    std::string memoryLayout;
    bool hostBoundary = false;
};

class AbiCallingConventionModel {
public:
    static AbiPacket describe(const std::string& source, const std::string& lang) {
        AbiPacket p;
        p.language = lang;
        p.callingConvention = source.find("stdcall") != std::string::npos ? "stdcall" : "cdecl";
        p.memoryLayout = source.find("packed") != std::string::npos ? "packed" : "default";
        p.hostBoundary = source.find("import") != std::string::npos || source.find("export") != std::string::npos;
        return p;
    }

    static nlohmann::json toJson(const AbiPacket& p) {
        return {{"language", p.language}, {"calling_convention", p.callingConvention}, {"memory_layout", p.memoryLayout}, {"host_boundary", p.hostBoundary}};
    }
};
