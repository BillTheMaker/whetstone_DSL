#pragma once
// Step 811: Build/runtime assumption inference.

#include <string>
#include <nlohmann/json.hpp>

struct AssumptionPacket {
    std::string category;
    bool runtime;  // true runtime assumption, false build
    std::string detail;
};

class AssumptionInference {
public:
    static AssumptionPacket analyze(const std::string& source) {
        AssumptionPacket p;
        if (source.find("Makefile") != std::string::npos) {
            p.category = "build";
            p.runtime = false;
            p.detail = "expects make build";
        } else {
            p.category = "runtime";
            p.runtime = true;
            p.detail = "expects env var";
        }
        return p;
    }

    static nlohmann::json toJson(const AssumptionPacket& p) {
        return {{"category", p.category}, {"runtime", p.runtime}, {"detail", p.detail}};
    }
};
