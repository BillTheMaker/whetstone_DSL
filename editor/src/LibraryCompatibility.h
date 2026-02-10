#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class LibraryCompatibility {
public:
    void addMapping(const std::string& fromLibrary,
                    const std::string& fromLang,
                    const std::string& toLibrary,
                    const std::string& toLang) {
        mappings_[key(fromLibrary, fromLang)].push_back({toLibrary, toLang});
    }

    std::vector<std::pair<std::string, std::string>> getMappings(
        const std::string& fromLibrary,
        const std::string& fromLang) const {
        auto it = mappings_.find(key(fromLibrary, fromLang));
        if (it == mappings_.end()) return {};
        return it->second;
    }

    void loadDefaultMappings() {
        addMapping("numpy", "python", "Eigen", "cpp");
        addMapping("numpy", "python", "ndarray", "rust");
        addMapping("pandas", "python", "dataframe", "rust");
        addMapping("pandas", "python", "DataFrame", "cpp");
        addMapping("requests", "python", "reqwest", "rust");
        addMapping("requests", "python", "cpr", "cpp");
    }

private:
    static std::string key(const std::string& lib, const std::string& lang) {
        return lib + "|" + lang;
    }

    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> mappings_;
};
