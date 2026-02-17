#pragma once

// Step 473: Technology Stack Selector
// Chooses per-module language/framework/database suggestions from requirements
// and module graph, with optional preference overrides and rationale.

#include "ArchitectModuleDecomposer.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

struct TechChoice {
    std::string moduleName;
    std::string language;
    std::string framework;
    std::string database;
    std::string rationale;
    float confidence = 0.0f;
};

struct TechStackDecision {
    std::vector<TechChoice> choices;
    std::map<std::string, std::string> preferencesApplied;

    bool hasChoiceFor(const std::string& moduleName) const {
        for (const auto& c : choices) if (c.moduleName == moduleName) return true;
        return false;
    }

    TechChoice getChoice(const std::string& moduleName) const {
        for (const auto& c : choices) if (c.moduleName == moduleName) return c;
        return {};
    }
};

class ArchitectTechStackSelector {
public:
    static TechStackDecision select(const StructuredRequirements& reqs,
                                    const ModuleGraph& graph,
                                    const std::map<std::string, std::string>& preferences = {}) {
        TechStackDecision out;
        out.preferencesApplied = preferences;

        std::string preferredBackend = pref(preferences, "backend_language");
        std::string preferredDb = pref(preferences, "database");
        std::string preferredFrontend = pref(preferences, "frontend_language");

        for (const auto& mod : graph.nodes) {
            if (mod.crossCutting) continue;
            TechChoice c;
            c.moduleName = mod.name;

            if (mod.name == "ui") {
                c.language = preferredFrontend.empty() ? "typescript" : preferredFrontend;
                c.framework = pickFrontendFramework(reqs);
                c.database = "";
                c.rationale = "UI module mapped to frontend language/framework";
                c.confidence = 0.92f;
            } else if (mod.name == "data") {
                c.language = preferredBackend.empty() ? pickBackendLanguage(reqs, mod) : preferredBackend;
                c.framework = "";
                c.database = preferredDb.empty() ? pickDatabase(reqs) : preferredDb;
                c.rationale = "Data module mapped to persistence stack";
                c.confidence = 0.90f;
            } else if (mod.name == "api" || mod.name == "auth" || mod.name == "search" ||
                       mod.name == "reporting" || mod.name == "notifications" || mod.name == "core") {
                c.language = preferredBackend.empty() ? pickBackendLanguage(reqs, mod) : preferredBackend;
                c.framework = pickBackendFramework(c.language);
                c.database = (mod.name == "api" || mod.name == "auth" || mod.name == "core")
                    ? (preferredDb.empty() ? pickDatabase(reqs) : preferredDb)
                    : "";
                c.rationale = "Service/backend module mapped to backend stack";
                c.confidence = 0.88f;
            } else if (mod.name == "integrations") {
                c.language = preferredBackend.empty() ? "python" : preferredBackend;
                c.framework = "http-client";
                c.database = "";
                c.rationale = "Integration module favors high-velocity adapter language";
                c.confidence = 0.85f;
            } else {
                c.language = preferredBackend.empty() ? "python" : preferredBackend;
                c.framework = "";
                c.database = "";
                c.rationale = "Fallback language selection";
                c.confidence = 0.75f;
            }

            out.choices.push_back(std::move(c));
        }

        // Ensure we always return at least one choice for minimal graphs.
        if (out.choices.empty()) {
            out.choices.push_back({"core", "python", "", pickDatabase(reqs),
                                   "Fallback stack decision for empty graph", 0.70f});
        }

        return out;
    }

private:
    static std::string pref(const std::map<std::string, std::string>& p, const std::string& key) {
        auto it = p.find(key);
        return it == p.end() ? "" : it->second;
    }

    static bool hasKeyword(const std::vector<ExtractedItem>& items, const std::string& keyword) {
        auto k = toLower(keyword);
        for (const auto& it : items) {
            if (toLower(it.text).find(k) != std::string::npos) return true;
        }
        return false;
    }

    static std::string pickBackendLanguage(const StructuredRequirements& reqs, const ModuleNode& mod) {
        bool perf = hasKeyword(reqs.nonFunctionalRequirements, "performance") ||
                    hasKeyword(reqs.nonFunctionalRequirements, "latency") ||
                    hasKeyword(reqs.nonFunctionalRequirements, "throughput");
        bool secure = hasKeyword(reqs.nonFunctionalRequirements, "secure") ||
                      hasKeyword(reqs.nonFunctionalRequirements, "security");
        if (mod.name == "search" && perf) return "rust";
        if ((mod.name == "api" || mod.name == "auth" || mod.name == "core") && (perf || secure))
            return "rust";
        if (hasKeyword(reqs.platformConstraints, "embedded")) return "c";
        return "python";
    }

    static std::string pickBackendFramework(const std::string& language) {
        if (language == "rust") return "axum";
        if (language == "python") return "fastapi";
        if (language == "go") return "gin";
        if (language == "java") return "spring";
        return "";
    }

    static std::string pickFrontendFramework(const StructuredRequirements& reqs) {
        (void)reqs;
        return "react";
    }

    static std::string pickDatabase(const StructuredRequirements& reqs) {
        if (hasKeyword(reqs.integrationRequirements, "postgresql") ||
            hasKeyword(reqs.integrationRequirements, "postgres")) return "postgresql";
        if (hasKeyword(reqs.integrationRequirements, "mysql")) return "mysql";
        if (hasKeyword(reqs.integrationRequirements, "sqlite")) return "sqlite";
        if (hasKeyword(reqs.integrationRequirements, "redis")) return "redis";
        return "postgresql";
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }
};
