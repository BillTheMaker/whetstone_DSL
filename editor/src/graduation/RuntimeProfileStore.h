#pragma once
// Step 896: Runtime profile store — active runtime profile per pair.
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

struct RuntimeProfile {
    std::string pairId;
    std::string runtimeId;
    std::string version;
};

class RuntimeProfileStore {
    std::unordered_map<std::string, RuntimeProfile> profiles_;
public:
    void set(const RuntimeProfile& p) { profiles_[p.pairId] = p; }

    bool has(const std::string& pairId) const {
        return profiles_.count(pairId) > 0;
    }

    const RuntimeProfile* get(const std::string& pairId) const {
        auto it = profiles_.find(pairId);
        return (it != profiles_.end()) ? &it->second : nullptr;
    }

    size_t count() const { return profiles_.size(); }

    std::vector<std::string> list() const {
        std::vector<std::string> v;
        for (const auto& kv : profiles_) v.push_back(kv.first);
        return v;
    }

    void clear() { profiles_.clear(); }

    static nlohmann::json toJson(const RuntimeProfile& p) {
        return {{"pair_id", p.pairId}, {"runtime_id", p.runtimeId}, {"version", p.version}};
    }
};
