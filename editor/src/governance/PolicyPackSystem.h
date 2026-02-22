#pragma once
// Step 823: Policy pack system (safe-first, interop-first, custom).

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class PolicyPackType { SafeFirst, InteropFirst, Custom };

struct PolicyRule {
    std::string ruleId;
    std::string description;
    bool blocking = true;
};

struct PolicyPack {
    std::string packId;
    PolicyPackType type = PolicyPackType::SafeFirst;
    std::string name;
    std::vector<PolicyRule> rules;
    bool allowWaivers = false;
};

class PolicyPackSystem {
public:
    static PolicyPack buildSafeFirst() {
        PolicyPack p;
        p.packId = "safe-first";
        p.type = PolicyPackType::SafeFirst;
        p.name = "Safe First";
        p.rules = {{"SF-01", "Block on any High severity issue", true},
                   {"SF-02", "Require reviewer sign-off for all changes", true},
                   {"SF-03", "Disallow waivers for High severity without escalation", true}};
        p.allowWaivers = false;
        return p;
    }

    static PolicyPack buildInteropFirst() {
        PolicyPack p;
        p.packId = "interop-first";
        p.type = PolicyPackType::InteropFirst;
        p.name = "Interop First";
        p.rules = {{"IF-01", "Allow temporary waivers for interop gaps", false},
                   {"IF-02", "Require compatibility test for each pair", true}};
        p.allowWaivers = true;
        return p;
    }

    static PolicyPack buildCustom(const std::string& packId, const std::vector<PolicyRule>& rules) {
        PolicyPack p;
        p.packId = packId;
        p.type = PolicyPackType::Custom;
        p.name = packId;
        p.rules = rules;
        p.allowWaivers = true;
        return p;
    }

    static bool validate(const PolicyPack& pack, std::string* error) {
        if (!error) return false;
        error->clear();
        if (pack.packId.empty()) { *error = "pack_id_missing"; return false; }
        if (pack.rules.empty()) { *error = "no_rules"; return false; }
        return true;
    }

    static bool evaluate(const PolicyPack& pack, const std::string& ruleId, std::string* error) {
        if (!error) return false;
        error->clear();
        for (const auto& r : pack.rules) {
            if (r.ruleId == ruleId) return r.blocking;
        }
        *error = "rule_not_found";
        return false;
    }

    static std::string typeStr(PolicyPackType t) {
        switch (t) {
            case PolicyPackType::SafeFirst:    return "safe-first";
            case PolicyPackType::InteropFirst: return "interop-first";
            case PolicyPackType::Custom:       return "custom";
        }
        return "unknown";
    }

    static nlohmann::json toJson(const PolicyPack& p) {
        nlohmann::json rules = nlohmann::json::array();
        for (const auto& r : p.rules) rules.push_back({{"rule_id", r.ruleId}, {"description", r.description}, {"blocking", r.blocking}});
        return {{"pack_id", p.packId}, {"type", typeStr(p.type)}, {"name", p.name},
                {"rules", rules}, {"allow_waivers", p.allowWaivers}};
    }
};
