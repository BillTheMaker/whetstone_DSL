#pragma once
// Step 656: Apiary browser panel model

#include <string>
#include <vector>

struct ApiaryToolEntry {
    std::string name;
    std::string version;
    std::string language;
    std::vector<std::string> capabilities;
    std::string docSummary;
    std::string sourcePath;
};

class ApiaryBrowserPanelModel {
public:
    static std::vector<ApiaryToolEntry> filterByCapability(const std::vector<ApiaryToolEntry>& entries,
                                                           const std::string& capability) {
        if (capability.empty()) return entries;
        std::vector<ApiaryToolEntry> out;
        for (const auto& e : entries) {
            for (const auto& cap : e.capabilities) {
                if (cap == capability) {
                    out.push_back(e);
                    break;
                }
            }
        }
        return out;
    }

    static std::string title(const ApiaryToolEntry& entry) {
        return entry.name + " v" + entry.version;
    }
};
