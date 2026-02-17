#pragma once

// Step 474: Skeleton Generation from Requirements
// Produces a multi-module annotated skeleton project specification from
// requirements, module graph, and selected technology stack.

#include "ArchitectTechStackSelector.h"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

struct SkeletonAnnotation {
    std::string key;   // e.g. @Intent
    std::string value; // e.g. "user authentication"
};

struct SkeletonFunctionSpec {
    std::string name;
    std::string signature;
    std::vector<SkeletonAnnotation> annotations;
};

struct SkeletonModuleSpec {
    std::string moduleName;
    std::string language;
    std::string framework;
    std::string database;
    std::vector<std::string> dependencies;
    std::vector<SkeletonFunctionSpec> functions;
};

struct SkeletonProjectSpec {
    std::vector<SkeletonModuleSpec> modules;

    bool hasModule(const std::string& moduleName) const {
        for (const auto& m : modules) if (m.moduleName == moduleName) return true;
        return false;
    }

    SkeletonModuleSpec getModule(const std::string& moduleName) const {
        for (const auto& m : modules) if (m.moduleName == moduleName) return m;
        return {};
    }
};

class ArchitectSkeletonGenerator {
public:
    static SkeletonProjectSpec generate(const StructuredRequirements& reqs,
                                        const ModuleGraph& graph,
                                        const TechStackDecision& stack) {
        SkeletonProjectSpec out;
        for (const auto& choice : stack.choices) {
            SkeletonModuleSpec mod;
            mod.moduleName = choice.moduleName;
            mod.language = choice.language;
            mod.framework = choice.framework;
            mod.database = choice.database;
            mod.dependencies = collectDependencies(graph, choice.moduleName);
            mod.functions = generateFunctions(reqs, graph, choice.moduleName);
            out.modules.push_back(std::move(mod));
        }

        // Ensure all non-cross-cutting modules are present even if stack choice was sparse.
        for (const auto& n : graph.nodes) {
            if (n.crossCutting) continue;
            if (!out.hasModule(n.name)) {
                SkeletonModuleSpec mod;
                mod.moduleName = n.name;
                mod.language = "python";
                mod.dependencies = collectDependencies(graph, n.name);
                mod.functions = generateFunctions(reqs, graph, n.name);
                out.modules.push_back(std::move(mod));
            }
        }
        return out;
    }

private:
    static std::vector<std::string> collectDependencies(const ModuleGraph& graph,
                                                         const std::string& moduleName) {
        std::set<std::string> deps;
        for (const auto& e : graph.edges) {
            if (e.from == moduleName) deps.insert(e.to);
        }
        return std::vector<std::string>(deps.begin(), deps.end());
    }

    static std::vector<SkeletonFunctionSpec> generateFunctions(const StructuredRequirements& reqs,
                                                               const ModuleGraph& graph,
                                                               const std::string& moduleName) {
        std::vector<SkeletonFunctionSpec> out;

        // Seed function names per module role.
        if (moduleName == "api") {
            out.push_back(makeFn("handleRequest", "Response handleRequest(Request req)", reqs, graph, moduleName));
            out.push_back(makeFn("validateRequest", "bool validateRequest(Request req)", reqs, graph, moduleName));
        } else if (moduleName == "auth") {
            out.push_back(makeFn("authenticateUser", "AuthResult authenticateUser(Credentials creds)", reqs, graph, moduleName));
            out.push_back(makeFn("authorizeAction", "bool authorizeAction(User user, Action action)", reqs, graph, moduleName));
        } else if (moduleName == "ui") {
            out.push_back(makeFn("renderView", "View renderView(State state)", reqs, graph, moduleName));
            out.push_back(makeFn("handleInteraction", "State handleInteraction(Event event)", reqs, graph, moduleName));
        } else if (moduleName == "data") {
            out.push_back(makeFn("saveEntity", "bool saveEntity(Entity entity)", reqs, graph, moduleName));
            out.push_back(makeFn("loadEntity", "Entity loadEntity(Id id)", reqs, graph, moduleName));
        } else if (moduleName == "search") {
            out.push_back(makeFn("search", "Results search(Query query)", reqs, graph, moduleName));
        } else if (moduleName == "reporting") {
            out.push_back(makeFn("generateReport", "Report generateReport(Filter filter)", reqs, graph, moduleName));
        } else if (moduleName == "notifications") {
            out.push_back(makeFn("sendNotification", "bool sendNotification(Message msg)", reqs, graph, moduleName));
        } else if (moduleName == "integrations") {
            out.push_back(makeFn("callExternalService", "ExternalResult callExternalService(Payload payload)", reqs, graph, moduleName));
        } else {
            out.push_back(makeFn("execute", "void execute()", reqs, graph, moduleName));
        }

        return out;
    }

    static SkeletonFunctionSpec makeFn(const std::string& name,
                                       const std::string& sig,
                                       const StructuredRequirements& reqs,
                                       const ModuleGraph& graph,
                                       const std::string& moduleName) {
        SkeletonFunctionSpec fn;
        fn.name = name;
        fn.signature = sig;

        // Intent annotation from module + functional requirements.
        fn.annotations.push_back({"@Intent", inferIntent(reqs, moduleName, name)});

        // Routing annotations
        fn.annotations.push_back({"@Complexity", inferComplexity(graph, moduleName)});
        fn.annotations.push_back({"@ContextWidth", inferContextWidth(moduleName)});
        fn.annotations.push_back({"@Automatability", inferAutomatability(moduleName, name)});

        // Contract annotation from functional requirements
        fn.annotations.push_back({"@Contract", inferContract(reqs, moduleName, name)});

        return fn;
    }

    static std::string inferIntent(const StructuredRequirements& reqs,
                                   const std::string& moduleName,
                                   const std::string& fnName) {
        std::string seed = moduleName + ":" + fnName;
        if (!reqs.functionalRequirements.empty()) {
            seed += " " + reqs.functionalRequirements[0].text;
        }
        return seed;
    }

    static std::string inferComplexity(const ModuleGraph& graph, const std::string& moduleName) {
        for (const auto& n : graph.nodes) {
            if (n.name == moduleName) {
                if (n.complexity >= 8) return "high";
                if (n.complexity >= 4) return "medium";
                return "low";
            }
        }
        return "medium";
    }

    static std::string inferContextWidth(const std::string& moduleName) {
        if (moduleName == "api" || moduleName == "core" || moduleName == "auth") return "project";
        if (moduleName == "integrations" || moduleName == "reporting") return "module";
        return "file";
    }

    static std::string inferAutomatability(const std::string& moduleName,
                                           const std::string& fnName) {
        (void)fnName;
        if (moduleName == "security" || moduleName == "auth") return "human";
        if (moduleName == "api" || moduleName == "data") return "llm";
        return "deterministic";
    }

    static std::string inferContract(const StructuredRequirements& reqs,
                                     const std::string& moduleName,
                                     const std::string& fnName) {
        std::string contract = "pre: valid input; post: deterministic output";
        if (moduleName == "auth") {
            contract = "pre: credentials provided; post: auth decision emitted";
        } else if (moduleName == "data") {
            contract = "pre: entity schema valid; post: persistence consistency";
        } else if (moduleName == "api" && fnName.find("validate") != std::string::npos) {
            contract = "pre: request shape provided; post: validation status";
        }
        if (!reqs.nonFunctionalRequirements.empty() &&
            reqs.nonFunctionalRequirements[0].text.find("secure") != std::string::npos) {
            contract += "; sec: enforce security constraints";
        }
        return contract;
    }
};
