#pragma once
// Step 734: gate severity policy.

#include <string>

#include <nlohmann/json.hpp>

class GateSeverityPolicy {
public:
    static std::string severityFor(const std::string& gate) {
        if (gate == "security" || gate == "sanitizer") return "block";
        if (gate == "performance" || gate == "supply_chain") return "warn";
        return "info";
    }

    static nlohmann::json toJson(const std::string& gate) {
        return {{"gate", gate}, {"severity", severityFor(gate)}};
    }
};
