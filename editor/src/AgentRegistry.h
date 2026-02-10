#pragma once
// Step 159: Agent registry for marketplace

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct AgentDescriptor {
    std::string name;
    std::string description;
    std::string endpoint;
    std::vector<std::string> capabilities;
    std::vector<std::string> requiredPermissions;
    bool installed = false;
};

class AgentRegistry {
public:
    const std::vector<AgentDescriptor>& list() const { return agents_; }

    AgentDescriptor* find(const std::string& name) {
        for (auto& a : agents_) {
            if (a.name == name) return &a;
        }
        return nullptr;
    }

    void upsert(const AgentDescriptor& agent) {
        if (auto* existing = find(agent.name)) {
            *existing = agent;
        } else {
            agents_.push_back(agent);
        }
    }

    void remove(const std::string& name) {
        agents_.erase(std::remove_if(agents_.begin(), agents_.end(),
                                     [&](const AgentDescriptor& a) {
                                         return a.name == name;
                                     }),
                      agents_.end());
    }

    void clear() { agents_.clear(); }

    void setRegistryUrl(const std::string& url) { registryUrl_ = url; }
    const std::string& getRegistryUrl() const { return registryUrl_; }

    bool loadFromFile(const std::string& path, std::string& error) {
        std::ifstream in(path);
        if (!in.is_open()) {
            error = "Could not open file: " + path;
            return false;
        }
        std::ostringstream ss;
        ss << in.rdbuf();
        return loadFromJsonString(ss.str(), error);
    }

    bool saveToFile(const std::string& path, std::string& error) const {
        std::ofstream out(path);
        if (!out.is_open()) {
            error = "Could not write file: " + path;
            return false;
        }
        out << toJson().dump(2);
        return true;
    }

    bool loadFromJsonString(const std::string& text, std::string& error) {
        try {
            json j = json::parse(text);
            return loadFromJson(j, error);
        } catch (const std::exception& e) {
            error = std::string("JSON parse error: ") + e.what();
            return false;
        }
    }

    bool loadFromJson(const json& j, std::string& error) {
        if (!j.is_object()) {
            error = "Invalid registry format";
            return false;
        }
        registryUrl_ = j.value("registryUrl", registryUrl_);
        if (!j.contains("agents") || !j["agents"].is_array()) {
            error = "Missing agents list";
            return false;
        }
        agents_.clear();
        for (const auto& item : j["agents"]) {
            AgentDescriptor desc;
            desc.name = item.value("name", "");
            desc.description = item.value("description", "");
            desc.endpoint = item.value("endpoint", "");
            desc.installed = item.value("installed", false);
            if (item.contains("capabilities")) {
                for (const auto& cap : item["capabilities"]) {
                    desc.capabilities.push_back(cap.get<std::string>());
                }
            }
            if (item.contains("requiredPermissions")) {
                for (const auto& perm : item["requiredPermissions"]) {
                    desc.requiredPermissions.push_back(perm.get<std::string>());
                }
            }
            if (!desc.name.empty()) agents_.push_back(std::move(desc));
        }
        return true;
    }

    json toJson() const {
        json out;
        out["registryUrl"] = registryUrl_;
        json arr = json::array();
        for (const auto& agent : agents_) {
            arr.push_back({
                {"name", agent.name},
                {"description", agent.description},
                {"endpoint", agent.endpoint},
                {"capabilities", agent.capabilities},
                {"requiredPermissions", agent.requiredPermissions},
                {"installed", agent.installed}
            });
        }
        out["agents"] = arr;
        return out;
    }

private:
    std::string registryUrl_;
    std::vector<AgentDescriptor> agents_;
};
