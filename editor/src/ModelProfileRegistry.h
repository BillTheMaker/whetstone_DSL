#pragma once

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct ModelProfile {
    std::string name;
    int contextWindow = 0;
    double costPer1kInput = 0.0;
    double costPer1kOutput = 0.0;
    std::string speedClass;
    std::vector<std::string> capabilities;

    json toJson() const {
        return {
            {"name", name},
            {"contextWindow", contextWindow},
            {"costPer1kInput", costPer1kInput},
            {"costPer1kOutput", costPer1kOutput},
            {"speedClass", speedClass},
            {"capabilities", capabilities}
        };
    }
};

class ModelProfileRegistry {
public:
    ModelProfileRegistry() {
        registerDefaults();
    }

    void registerProfile(const ModelProfile& profile) {
        if (profile.name.empty()) return;
        profiles_[profile.name] = profile;
    }

    bool hasProfile(const std::string& name) const {
        return profiles_.find(name) != profiles_.end();
    }

    ModelProfile getProfile(const std::string& name) const {
        auto it = profiles_.find(name);
        if (it != profiles_.end()) return it->second;
        return {};
    }

    std::vector<std::string> listProfileNames() const {
        std::vector<std::string> names;
        for (const auto& [k, _] : profiles_) names.push_back(k);
        std::sort(names.begin(), names.end());
        return names;
    }

    std::string resolveProfileForWorker(const std::string& workerType) const {
        std::string lower = toLower(workerType);
        auto it = workerToProfile_.find(lower);
        if (it != workerToProfile_.end()) return it->second;
        return "";
    }

    ModelProfile resolveForWorker(const std::string& workerType) const {
        std::string name = resolveProfileForWorker(workerType);
        if (name.empty()) return {};
        return getProfile(name);
    }

    void setWorkerProfileMapping(const std::string& workerType,
                                 const std::string& profileName) {
        std::string w = toLower(workerType);
        if (w.empty() || profileName.empty()) return;
        if (!hasProfile(profileName)) return;
        workerToProfile_[w] = profileName;
    }

    bool loadOverridesFromFile(const std::string& configPath,
                               std::string& error) {
        std::ifstream in(configPath);
        if (!in.is_open()) {
            error = "Cannot open config: " + configPath;
            return false;
        }
        json cfg;
        try {
            in >> cfg;
        } catch (...) {
            error = "Invalid JSON in config";
            return false;
        }
        if (!cfg.is_object()) {
            error = "Config must be object";
            return false;
        }
        applyOverrides(cfg);
        return true;
    }

    void applyOverrides(const json& cfg) {
        if (cfg.contains("profiles") && cfg["profiles"].is_array()) {
            for (const auto& p : cfg["profiles"]) {
                std::string name = p.value("name", "");
                if (name.empty()) continue;
                ModelProfile prof = getProfile(name);
                if (prof.name.empty()) prof.name = name;
                prof.contextWindow = p.value("contextWindow", prof.contextWindow);
                prof.costPer1kInput = p.value("costPer1kInput", prof.costPer1kInput);
                prof.costPer1kOutput = p.value("costPer1kOutput", prof.costPer1kOutput);
                prof.speedClass = p.value("speedClass", prof.speedClass);
                if (p.contains("capabilities") && p["capabilities"].is_array()) {
                    prof.capabilities.clear();
                    for (const auto& c : p["capabilities"]) {
                        if (c.is_string()) prof.capabilities.push_back(c.get<std::string>());
                    }
                }
                registerProfile(prof);
            }
        }

        if (cfg.contains("workerMapping") && cfg["workerMapping"].is_object()) {
            for (auto it = cfg["workerMapping"].begin();
                 it != cfg["workerMapping"].end(); ++it) {
                if (!it.value().is_string()) continue;
                setWorkerProfileMapping(it.key(), it.value().get<std::string>());
            }
        }
    }

    json toJson() const {
        json profiles = json::array();
        for (const auto& name : listProfileNames()) {
            profiles.push_back(getProfile(name).toJson());
        }
        json mapping = json::object();
        for (const auto& [worker, profile] : workerToProfile_) {
            mapping[worker] = profile;
        }
        return {{"profiles", profiles}, {"workerMapping", mapping}};
    }

private:
    std::map<std::string, ModelProfile> profiles_;
    std::map<std::string, std::string> workerToProfile_;

    static std::string toLower(const std::string& in) {
        std::string out = in;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return out;
    }

    void registerDefaults() {
        registerProfile({"claude-opus", 200000, 0.015, 0.075, "slow",
                         {"reasoning", "architecture", "long-context"}});
        registerProfile({"claude-sonnet", 200000, 0.003, 0.015, "medium",
                         {"coding", "reasoning", "long-context"}});
        registerProfile({"claude-haiku", 200000, 0.0008, 0.004, "fast",
                         {"coding", "quick-edits"}});
        registerProfile({"local-slm", 8192, 0.0, 0.0, "fast",
                         {"offline", "low-latency"}});

        workerToProfile_["llm"] = "claude-sonnet";
        workerToProfile_["slm"] = "claude-haiku";
        workerToProfile_["deterministic"] = "local-slm";
        workerToProfile_["template"] = "local-slm";
    }
};
