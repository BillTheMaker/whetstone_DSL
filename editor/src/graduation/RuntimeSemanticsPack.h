#pragma once
// Step 889: Runtime semantics pack schema.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct RuntimeAssumption {
    std::string assumptionId;
    std::string category;    // "memory","threading","io","exception","lifecycle"
    std::string description;
    bool isBreaking = false;  // breaking if violated during migration
};

struct RuntimeSemanticsPack {
    std::string runtimeId;   // e.g. "cpython3", "jvm11", "dotnet6", "nodejs18"
    std::string version;
    std::vector<RuntimeAssumption> assumptions;
    bool loaded = false;
};

class RuntimeSemanticsPackSchema {
public:
    static RuntimeSemanticsPack make(const std::string& runtimeId,
                                      const std::string& version) {
        RuntimeSemanticsPack p;
        p.runtimeId = runtimeId;
        p.version = version;
        p.loaded = !runtimeId.empty();
        if (p.loaded) {
            p.assumptions.push_back({"A-1", "memory", "gc_managed", false});
            p.assumptions.push_back({"A-2", "threading", "gil_present", true});
        }
        return p;
    }

    static bool validate(const RuntimeSemanticsPack& p, std::string* error = nullptr) {
        if (p.runtimeId.empty()) { if (error) *error = "runtime_id_missing"; return false; }
        if (p.version.empty())   { if (error) *error = "version_missing"; return false; }
        return true;
    }

    static nlohmann::json toJson(const RuntimeSemanticsPack& p) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& a : p.assumptions)
            arr.push_back({{"id", a.assumptionId}, {"category", a.category},
                           {"description", a.description}, {"breaking", a.isBreaking}});
        return {{"runtime_id", p.runtimeId}, {"version", p.version},
                {"loaded", p.loaded}, {"assumptions", arr}};
    }
};
