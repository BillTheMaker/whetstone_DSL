#pragma once
// Step 1154: Interop certification policy bindings.
#include <string>
#include <nlohmann/json.hpp>

struct InteropCertificationPolicyBindings {
    std::string policyId;
    double minPassRate = 0.0;
    bool requiresOracleMatch = false;
    bool requiresDeterministicReplay = false;
    bool bindingActive = false;
    bool valid = false;
};

class InteropCertificationPolicyBindingsFactory {
public:
    static InteropCertificationPolicyBindings make(const std::string& policyId,
                                                   double minPassRate,
                                                   bool requiresOracleMatch,
                                                   bool requiresDeterministicReplay,
                                                   bool bindingActive) {
        bool validRate = minPassRate >= 0.0 && minPassRate <= 1.0;
        bool valid = !policyId.empty() && validRate && bindingActive &&
                     requiresOracleMatch && requiresDeterministicReplay;
        return {policyId, minPassRate, requiresOracleMatch, requiresDeterministicReplay, bindingActive, valid};
    }

    static nlohmann::json toJson(const InteropCertificationPolicyBindings& p) {
        return {{"policy_id", p.policyId},
                {"min_pass_rate", p.minPassRate},
                {"requires_oracle_match", p.requiresOracleMatch},
                {"requires_deterministic_replay", p.requiresDeterministicReplay},
                {"binding_active", p.bindingActive},
                {"valid", p.valid}};
    }
};
