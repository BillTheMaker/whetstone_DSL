#pragma once
// Step 890: Pack loader and version manager.
#include <string>
#include <vector>
#include <map>
#include "RuntimeSemanticsPack.h"

class RuntimePackLoader {
public:
    bool load(const RuntimeSemanticsPack& pack, std::string* error = nullptr) {
        if (!RuntimeSemanticsPackSchema::validate(pack, error)) return false;
        packs_[pack.runtimeId] = pack;
        return true;
    }

    bool isLoaded(const std::string& runtimeId) const {
        return packs_.count(runtimeId) > 0;
    }

    const RuntimeSemanticsPack* get(const std::string& runtimeId) const {
        auto it = packs_.find(runtimeId);
        return it != packs_.end() ? &it->second : nullptr;
    }

    int loadedCount() const { return static_cast<int>(packs_.size()); }

    std::vector<std::string> listLoaded() const {
        std::vector<std::string> ids;
        for (const auto& kv : packs_) ids.push_back(kv.first);
        return ids;
    }

    bool unload(const std::string& runtimeId) {
        return packs_.erase(runtimeId) > 0;
    }

private:
    std::map<std::string, RuntimeSemanticsPack> packs_;
};
