#pragma once
// Step 651: Language generator plugin manifest

#include <string>
#include <vector>

struct PluginManifest {
    std::string pluginName;
    std::string sharedObject;
    std::string generatorClass;
    std::vector<std::string> languages;
};

class LanguageGeneratorPluginManifest {
public:
    static bool isValid(const PluginManifest& m) {
        return !m.pluginName.empty() && !m.sharedObject.empty() &&
               !m.generatorClass.empty() && !m.languages.empty();
    }

    static bool isPluginFile(const std::string& fileName) {
        return fileName.rfind("whetstone-plugin-", 0) == 0 &&
               fileName.size() > 3 &&
               fileName.substr(fileName.size() - 3) == ".so";
    }

    static std::string symbolName(const PluginManifest& m) {
        return "create_" + m.generatorClass;
    }
};
