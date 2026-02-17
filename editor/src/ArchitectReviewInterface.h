#pragma once

// Step 475: Architect Review Interface
// Structured review state and mutation operations over proposed stack/skeleton.

#include "ArchitectSkeletonGenerator.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

struct ModuleReviewSummary {
    std::string moduleName;
    std::string language;
    int functionCount = 0;
    int dependencyCount = 0;
    std::vector<std::string> keyAnnotations;
};

struct ArchitectReviewSnapshot {
    std::vector<ModuleReviewSummary> modules;
    std::vector<ModuleEdge> dependencyGraph;
    std::vector<std::string> stackRationales;
};

struct ArchitectReviewState {
    SkeletonProjectSpec skeleton;
    TechStackDecision stack;
    ModuleGraph moduleGraph;
    bool approved = false;
    std::vector<std::string> changeLog;
};

class ArchitectReviewInterface {
public:
    static ArchitectReviewState initialize(const SkeletonProjectSpec& skeleton,
                                           const TechStackDecision& stack,
                                           const ModuleGraph& graph) {
        ArchitectReviewState st;
        st.skeleton = skeleton;
        st.stack = stack;
        st.moduleGraph = graph;
        st.approved = false;
        return st;
    }

    static ArchitectReviewSnapshot snapshot(const ArchitectReviewState& st) {
        ArchitectReviewSnapshot snap;
        snap.dependencyGraph = st.moduleGraph.edges;

        for (const auto& m : st.skeleton.modules) {
            ModuleReviewSummary s;
            s.moduleName = m.moduleName;
            s.language = m.language;
            s.functionCount = static_cast<int>(m.functions.size());
            s.dependencyCount = static_cast<int>(m.dependencies.size());
            s.keyAnnotations = collectKeyAnnotations(m);
            snap.modules.push_back(std::move(s));
        }

        for (const auto& c : st.stack.choices) {
            snap.stackRationales.push_back(c.moduleName + ": " + c.rationale);
        }
        return snap;
    }

    static bool changeModuleLanguage(ArchitectReviewState& st,
                                     const std::string& moduleName,
                                     const std::string& newLanguage) {
        bool changed = false;
        for (auto& m : st.skeleton.modules) {
            if (m.moduleName == moduleName) {
                m.language = newLanguage;
                changed = true;
            }
        }
        for (auto& c : st.stack.choices) {
            if (c.moduleName == moduleName) {
                c.language = newLanguage;
                changed = true;
            }
        }
        if (changed) st.changeLog.push_back("language:" + moduleName + "->" + newLanguage);
        return changed;
    }

    static bool mergeModules(ArchitectReviewState& st,
                             const std::string& primary,
                             const std::string& secondary) {
        auto pIdx = indexOf(st.skeleton.modules, primary);
        auto sIdx = indexOf(st.skeleton.modules, secondary);
        if (pIdx < 0 || sIdx < 0 || pIdx == sIdx) return false;

        auto& p = st.skeleton.modules[static_cast<size_t>(pIdx)];
        auto& s = st.skeleton.modules[static_cast<size_t>(sIdx)];

        p.functions.insert(p.functions.end(), s.functions.begin(), s.functions.end());
        p.dependencies.insert(p.dependencies.end(), s.dependencies.begin(), s.dependencies.end());
        dedupe(p.dependencies);

        st.skeleton.modules.erase(st.skeleton.modules.begin() + sIdx);
        removeStackChoice(st.stack, secondary);
        remapGraphAfterMerge(st.moduleGraph, primary, secondary);
        st.changeLog.push_back("merge:" + primary + "<-" + secondary);
        return true;
    }

    static bool addModule(ArchitectReviewState& st,
                          const std::string& moduleName,
                          const std::string& language,
                          const std::vector<std::string>& responsibilities = {}) {
        if (st.skeleton.hasModule(moduleName)) return false;

        SkeletonModuleSpec m;
        m.moduleName = moduleName;
        m.language = language;
        m.functions.push_back(defaultFunctionFor(moduleName));
        st.skeleton.modules.push_back(m);

        st.stack.choices.push_back({moduleName, language, "", "", "Architect-added module", 0.70f});
        st.moduleGraph.nodes.push_back({moduleName, responsibilities, 4, false});
        st.changeLog.push_back("add_module:" + moduleName);
        return true;
    }

    static bool adjustRouting(ArchitectReviewState& st,
                              const std::string& moduleName,
                              const std::string& functionName,
                              const std::string& automatability,
                              const std::string& contextWidth,
                              const std::string& complexity) {
        for (auto& m : st.skeleton.modules) {
            if (m.moduleName != moduleName) continue;
            for (auto& f : m.functions) {
                if (f.name != functionName) continue;
                setAnnotation(f.annotations, "@Automatability", automatability);
                setAnnotation(f.annotations, "@ContextWidth", contextWidth);
                setAnnotation(f.annotations, "@Complexity", complexity);
                st.changeLog.push_back("routing:" + moduleName + "." + functionName);
                return true;
            }
        }
        return false;
    }

    static void approve(ArchitectReviewState& st) {
        st.approved = true;
        st.changeLog.push_back("approved");
    }

private:
    static std::vector<std::string> collectKeyAnnotations(const SkeletonModuleSpec& m) {
        std::set<std::string> keys;
        for (const auto& f : m.functions) {
            for (const auto& a : f.annotations) {
                if (a.key == "@Intent" || a.key == "@Complexity" || a.key == "@Automatability" ||
                    a.key == "@ContextWidth" || a.key == "@Contract") {
                    keys.insert(a.key);
                }
            }
        }
        return std::vector<std::string>(keys.begin(), keys.end());
    }

    static int indexOf(const std::vector<SkeletonModuleSpec>& mods, const std::string& name) {
        for (size_t i = 0; i < mods.size(); ++i) if (mods[i].moduleName == name) return static_cast<int>(i);
        return -1;
    }

    static void dedupe(std::vector<std::string>& values) {
        std::set<std::string> uniq(values.begin(), values.end());
        values.assign(uniq.begin(), uniq.end());
    }

    static void removeStackChoice(TechStackDecision& stack, const std::string& moduleName) {
        stack.choices.erase(
            std::remove_if(stack.choices.begin(), stack.choices.end(),
                           [&](const TechChoice& c){ return c.moduleName == moduleName; }),
            stack.choices.end());
    }

    static void remapGraphAfterMerge(ModuleGraph& g,
                                     const std::string& primary,
                                     const std::string& secondary) {
        for (auto& e : g.edges) {
            if (e.from == secondary) e.from = primary;
            if (e.to == secondary) e.to = primary;
        }
        g.nodes.erase(std::remove_if(g.nodes.begin(), g.nodes.end(),
                                     [&](const ModuleNode& n){ return n.name == secondary; }),
                      g.nodes.end());
        // Remove self-loop duplicates introduced by merge remapping.
        std::set<std::string> seen;
        std::vector<ModuleEdge> out;
        for (const auto& e : g.edges) {
            if (e.from == e.to) continue;
            std::string k = e.from + "->" + e.to + ":" + e.reason;
            if (!seen.insert(k).second) continue;
            out.push_back(e);
        }
        g.edges.swap(out);
    }

    static SkeletonFunctionSpec defaultFunctionFor(const std::string& moduleName) {
        SkeletonFunctionSpec f;
        f.name = "execute";
        f.signature = "void execute()";
        f.annotations = {
            {"@Intent", moduleName + ":execute"},
            {"@Complexity", "medium"},
            {"@ContextWidth", "module"},
            {"@Automatability", "llm"},
            {"@Contract", "pre: valid input; post: deterministic output"}
        };
        return f;
    }

    static void setAnnotation(std::vector<SkeletonAnnotation>& anns,
                              const std::string& key,
                              const std::string& value) {
        for (auto& a : anns) {
            if (a.key == key) {
                a.value = value;
                return;
            }
        }
        anns.push_back({key, value});
    }
};
