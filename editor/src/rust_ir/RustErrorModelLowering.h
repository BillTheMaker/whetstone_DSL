#pragma once
// Step 703: error model lowering (Result/Option/panic).

#include <string>

#include <nlohmann/json.hpp>

struct RustErrorModel {
    bool usesResult = false;
    bool usesOption = false;
    bool usesPanic = false;
    bool usesQuestionOperator = false;
};

class RustErrorModelLowering {
public:
    static RustErrorModel lower(const std::string& src) {
        RustErrorModel out;
        out.usesResult = src.find("Result<") != std::string::npos;
        out.usesOption = src.find("Option<") != std::string::npos;
        out.usesPanic = src.find("panic!(") != std::string::npos;
        out.usesQuestionOperator = src.find('?') != std::string::npos;
        return out;
    }

    static nlohmann::json toJson(const RustErrorModel& e) {
        return {
            {"usesResult", e.usesResult},
            {"usesOption", e.usesOption},
            {"usesPanic", e.usesPanic},
            {"usesQuestionOperator", e.usesQuestionOperator}
        };
    }
};
