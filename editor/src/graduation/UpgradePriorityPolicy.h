#pragma once
// Step 880: Priority policy for pair upgrades.
#include <string>
#include <nlohmann/json.hpp>

struct PriorityPolicyResult {
    std::string pairId;
    std::string priorityClass; // "critical","revenue","coverage","experimental"
    int priorityLevel = 0;     // critical=4, revenue=3, coverage=2, experimental=1
    bool preemptsExperimental = false;
};

class UpgradePriorityPolicy {
public:
    static PriorityPolicyResult classify(const std::string& pairId,
                                          const std::string& requestedClass) {
        PriorityPolicyResult r;
        r.pairId = pairId;
        if (requestedClass == "critical") {
            r.priorityClass = "critical"; r.priorityLevel = 4; r.preemptsExperimental = true;
        } else if (requestedClass == "revenue") {
            r.priorityClass = "revenue"; r.priorityLevel = 3; r.preemptsExperimental = false;
        } else if (requestedClass == "coverage") {
            r.priorityClass = "coverage"; r.priorityLevel = 2; r.preemptsExperimental = false;
        } else {
            r.priorityClass = "experimental"; r.priorityLevel = 1; r.preemptsExperimental = false;
        }
        return r;
    }

    static bool validate(const std::string& priorityClass) {
        return priorityClass == "critical" || priorityClass == "revenue" ||
               priorityClass == "coverage" || priorityClass == "experimental";
    }

    static nlohmann::json toJson(const PriorityPolicyResult& r) {
        return {{"pair_id", r.pairId}, {"priority_class", r.priorityClass},
                {"priority_level", r.priorityLevel},
                {"preempts_experimental", r.preemptsExperimental}};
    }
};
