#pragma once
// Step 693: IR -> language adapter contract and registry.

#include <map>
#include <string>
#include <vector>

#include "LanguageToIRAdapter.h"

class IRToLanguageAdapter {
public:
    virtual ~IRToLanguageAdapter() = default;
    virtual std::string targetLanguage() const = 0;
    virtual std::string version() const = 0;
    virtual AdapterResult raise(const SemanticCoreIR& ir,
                                std::string* outCode) const = 0;
};

class IRToLanguageAdapterRegistry {
public:
    void registerAdapter(const IRToLanguageAdapter* adapter) {
        if (!adapter) return;
        raising_[adapter->targetLanguage()].push_back(adapter);
    }

    const IRToLanguageAdapter* select(const std::string& language,
                                      const std::string& version = "") const {
        auto it = raising_.find(language);
        if (it == raising_.end() || it->second.empty()) return nullptr;
        if (!version.empty()) {
            for (auto* a : it->second) if (a->version() == version) return a;
        }
        return it->second.front(); // deterministic fallback
    }

private:
    std::map<std::string, std::vector<const IRToLanguageAdapter*>> raising_;
};
