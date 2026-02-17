#pragma once

// Step 472: Module Decomposition Engine
// Produces a module graph from structured requirements with strategy-specific
// decomposition behavior (layered/monolith/microservice).

#include "ArchitectProblemParser.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

enum class DecompositionStrategy {
    Layered,
    Monolith,
    Microservice
};

struct ModuleNode {
    std::string name;
    std::vector<std::string> responsibilities;
    int complexity = 1; // 1-10 heuristic
    bool crossCutting = false;
};

struct ModuleEdge {
    std::string from;
    std::string to;
    std::string reason;
};

struct ModuleGraph {
    std::vector<ModuleNode> nodes;
    std::vector<ModuleEdge> edges;
    DecompositionStrategy strategy = DecompositionStrategy::Layered;

    bool hasModule(const std::string& name) const {
        for (const auto& n : nodes) if (n.name == name) return true;
        return false;
    }

    int dependencyCountFor(const std::string& moduleName) const {
        int count = 0;
        for (const auto& e : edges)
            if (e.from == moduleName || e.to == moduleName) ++count;
        return count;
    }
};

class ArchitectModuleDecomposer {
public:
    static ModuleGraph decompose(const StructuredRequirements& reqs,
                                 DecompositionStrategy strategy = DecompositionStrategy::Layered) {
        ModuleGraph g;
        g.strategy = strategy;

        // Core modules inferred from functional/integration requirements
        maybeAddAuth(reqs, g);
        maybeAddApi(reqs, g);
        maybeAddUi(reqs, g);
        maybeAddData(reqs, g);
        maybeAddSearch(reqs, g);
        maybeAddReporting(reqs, g);
        maybeAddNotifications(reqs, g);
        maybeAddIntegration(reqs, g);

        if (strategy == DecompositionStrategy::Monolith) {
            addModule(g, "core", {"application orchestration", "module coordination"}, 5);
        }

        if (g.nodes.empty()) {
            g.nodes.push_back({"core", {"core application behavior"}, 3, false});
        }

        addCrossCutting(reqs, g);
        addEdges(strategy, g);
        normalizeComplexity(g);
        return g;
    }

private:
    static void addModule(ModuleGraph& g,
                          const std::string& name,
                          const std::vector<std::string>& responsibilities,
                          int complexity,
                          bool crossCutting = false) {
        if (g.hasModule(name)) return;
        g.nodes.push_back({name, responsibilities, complexity, crossCutting});
    }

    static bool hasReqKeyword(const std::vector<ExtractedItem>& items, const std::string& key) {
        auto lk = toLower(key);
        for (const auto& it : items) {
            if (toLower(it.text).find(lk) != std::string::npos) return true;
        }
        return false;
    }

    static bool hasAny(const StructuredRequirements& r,
                       const std::vector<std::string>& keys) {
        for (const auto& k : keys) {
            if (hasReqKeyword(r.functionalRequirements, k)) return true;
            if (hasReqKeyword(r.integrationRequirements, k)) return true;
            if (hasReqKeyword(r.platformConstraints, k)) return true;
        }
        return false;
    }

    static void maybeAddAuth(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"auth", "login", "signup", "oauth", "jwt"}))
            addModule(g, "auth", {"identity", "session", "authorization"}, 6);
    }

    static void maybeAddApi(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"api", "rest", "endpoint", "crud"}))
            addModule(g, "api", {"http routing", "request validation", "response mapping"}, 6);
    }

    static void maybeAddUi(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"web", "mobile", "frontend", "dashboard", "ios", "android"}))
            addModule(g, "ui", {"presentation", "interaction", "client state"}, 5);
    }

    static void maybeAddData(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"postgres", "postgresql", "mysql", "sqlite", "redis", "data"}))
            addModule(g, "data", {"persistence", "queries", "transactions"}, 7);
    }

    static void maybeAddSearch(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"search"}))
            addModule(g, "search", {"indexing", "query parsing"}, 5);
    }

    static void maybeAddReporting(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"reporting"}))
            addModule(g, "reporting", {"aggregation", "export", "analytics"}, 5);
    }

    static void maybeAddNotifications(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"notification"}))
            addModule(g, "notifications", {"event fanout", "delivery"}, 4);
    }

    static void maybeAddIntegration(const StructuredRequirements& r, ModuleGraph& g) {
        if (hasAny(r, {"stripe", "salesforce", "ldap", "kafka", "s3"}))
            addModule(g, "integrations", {"external api clients", "adapters"}, 6);
    }

    static void addCrossCutting(const StructuredRequirements& r, ModuleGraph& g) {
        bool secure = hasReqKeyword(r.nonFunctionalRequirements, "secure") ||
                      hasReqKeyword(r.nonFunctionalRequirements, "security");
        bool scalable = hasReqKeyword(r.nonFunctionalRequirements, "scalable") ||
                        hasReqKeyword(r.nonFunctionalRequirements, "performance");

        addModule(g, "logging", {"structured logs", "trace context"}, 2, true);
        addModule(g, "config", {"env config", "feature flags"}, 2, true);
        addModule(g, "error_handling", {"error taxonomy", "mapping"}, 2, true);
        if (secure) addModule(g, "security", {"policy", "hardening"}, 4, true);
        if (scalable) addModule(g, "observability", {"metrics", "alerts"}, 3, true);
    }

    static void addEdges(DecompositionStrategy strategy, ModuleGraph& g) {
        auto edge = [&g](const std::string& a, const std::string& b, const std::string& reason) {
            if (!g.hasModule(a) || !g.hasModule(b)) return;
            g.edges.push_back({a, b, reason});
        };

        if (strategy == DecompositionStrategy::Layered) {
            edge("ui", "api", "ui consumes api");
            edge("api", "auth", "endpoint auth checks");
            edge("api", "data", "persistence access");
            edge("api", "search", "search endpoints");
            edge("api", "reporting", "report generation");
        } else if (strategy == DecompositionStrategy::Microservice) {
            edge("api", "auth", "auth service call");
            edge("api", "data", "data service call");
            edge("api", "integrations", "integration gateway");
            edge("notifications", "integrations", "delivery integrations");
        } else { // Monolith
            edge("core", "data", "monolith persistence");
            edge("core", "auth", "monolith auth");
            edge("core", "ui", "monolith ui");
        }

        // Cross-cutting edges
        std::vector<std::string> cross;
        for (const auto& n : g.nodes) if (n.crossCutting) cross.push_back(n.name);
        for (const auto& n : g.nodes) {
            if (n.crossCutting) continue;
            for (const auto& c : cross) edge(n.name, c, "cross-cutting concern");
        }
    }

    static void normalizeComplexity(ModuleGraph& g) {
        std::map<std::string, int> incoming;
        for (const auto& e : g.edges) incoming[e.to]++;
        for (auto& n : g.nodes) {
            int c = n.complexity + incoming[n.name];
            if (c < 1) c = 1;
            if (c > 10) c = 10;
            n.complexity = c;
        }
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }
};
