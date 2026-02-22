#pragma once
// Step 693: Language -> IR adapter contract and registry.

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "SemanticCoreIR.h"

struct AdapterResult {
    bool success = false;
    float confidence = 0.0f;
    bool reviewRequired = false;
    std::string error;
    std::vector<std::string> unsupportedFeatures;
    std::map<std::string, int> sourceLocationMap; // nodeId -> source line
};

class LanguageToIRAdapter {
public:
    virtual ~LanguageToIRAdapter() = default;
    virtual std::string language() const = 0;
    virtual std::string version() const = 0;
    virtual AdapterResult lower(const std::string& source,
                                SemanticCoreIR* out) const = 0;
};

class LanguageToIRAdapterRegistry {
public:
    void registerAdapter(const LanguageToIRAdapter* adapter) {
        if (!adapter) return;
        lowering_[adapter->language()].push_back(adapter);
    }

    const LanguageToIRAdapter* select(const std::string& language,
                                      const std::string& version = "") const {
        auto it = lowering_.find(language);
        if (it == lowering_.end() || it->second.empty()) return nullptr;
        if (!version.empty()) {
            for (auto* a : it->second) if (a->version() == version) return a;
        }
        return it->second.front(); // deterministic fallback: first registered
    }

private:
    std::map<std::string, std::vector<const LanguageToIRAdapter*>> lowering_;
};
