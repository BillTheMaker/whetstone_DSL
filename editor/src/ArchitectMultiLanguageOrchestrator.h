#pragma once

// Step 480: Multi-Language Project Orchestration
// Creates one project workflow with build-ordering and cross-language links.

#include "ArchitectBuildAwareness.h"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

struct CrossLanguageLink {
    std::string fromModule;
    std::string toModule;
    std::string fromLanguage;
    std::string toLanguage;
    std::string linkAnnotation; // @Link(...)
    std::string shimAnnotation; // @Shim(...)
    bool ffiBoundary = false;
    bool reviewRequired = false;
};

struct OrchestrationTask {
    std::string taskId;
    std::string moduleName;
    std::string language;
    std::vector<std::string> dependencies;
    int buildOrder = 0;
    bool reviewRequired = false;
    std::vector<std::string> annotations;
    std::vector<std::string> buildHints;
};

struct MultiLanguageWorkflowPlan {
    std::vector<OrchestrationTask> tasks;
    std::vector<CrossLanguageLink> links;
    std::vector<std::string> notes;

    bool hasTask(const std::string& moduleName) const {
        for (const auto& t : tasks) if (t.moduleName == moduleName) return true;
        return false;
    }

    OrchestrationTask getTask(const std::string& moduleName) const {
        for (const auto& t : tasks) if (t.moduleName == moduleName) return t;
        return {};
    }
};

class ArchitectMultiLanguageOrchestrator {
public:
    static MultiLanguageWorkflowPlan createWorkflow(
        const SkeletonProjectSpec& skeleton,
        const BuildAwarenessReport& awareness) {
        MultiLanguageWorkflowPlan out;
        const auto order = topoOrder(skeleton, out.notes);

        std::map<std::string, SkeletonModuleSpec> byName;
        for (const auto& m : skeleton.modules) byName[m.moduleName] = m;

        std::set<std::string> reviewModules;
        for (const auto& m : skeleton.modules) {
            for (const auto& dep : m.dependencies) {
                if (!byName.count(dep)) continue;
                const auto& to = byName[dep];
                if (lower(m.language) == lower(to.language)) continue;

                CrossLanguageLink link;
                link.fromModule = m.moduleName;
                link.toModule = dep;
                link.fromLanguage = lower(m.language);
                link.toLanguage = lower(to.language);
                link.linkAnnotation = "@Link(" + m.moduleName + "->" + dep + ")";
                link.shimAnnotation = "@Shim(" + link.fromLanguage + "_to_" + link.toLanguage + ")";
                link.ffiBoundary = true;
                link.reviewRequired = true;
                out.links.push_back(std::move(link));

                reviewModules.insert(m.moduleName);
                reviewModules.insert(dep);
            }
        }

        for (size_t i = 0; i < order.size(); ++i) {
            const auto& name = order[i];
            if (!byName.count(name)) continue;
            const auto& m = byName[name];

            OrchestrationTask task;
            task.taskId = "orchestrate-" + std::to_string(i + 1);
            task.moduleName = name;
            task.language = lower(m.language);
            task.dependencies = m.dependencies;
            task.buildOrder = (int)i + 1;
            task.reviewRequired = reviewModules.count(name) > 0;
            task.annotations.push_back("@Intent(orchestrate module " + name + ")");
            task.annotations.push_back("@BuildOrder(" + std::to_string(task.buildOrder) + ")");
            if (task.reviewRequired) task.annotations.push_back("@Review(ffi-boundary)");

            auto hint = awareness.getModule(name);
            if (!hint.manifestSnippet.empty()) task.buildHints.push_back(hint.manifestSnippet);
            for (const auto& imp : hint.importHints) task.buildHints.push_back(imp);

            out.tasks.push_back(std::move(task));
        }

        out.notes.push_back("Workflow spans all modules as a single project plan");
        out.notes.push_back("Cross-language links emit @Link/@Shim and require human review");
        return out;
    }

private:
    static std::string lower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static std::vector<std::string> topoOrder(const SkeletonProjectSpec& skeleton,
                                              std::vector<std::string>& notes) {
        std::map<std::string, int> indeg;
        std::map<std::string, std::vector<std::string>> adj;
        std::set<std::string> nodes;

        for (const auto& m : skeleton.modules) {
            nodes.insert(m.moduleName);
            if (!indeg.count(m.moduleName)) indeg[m.moduleName] = 0;
        }

        for (const auto& m : skeleton.modules) {
            for (const auto& dep : m.dependencies) {
                if (!nodes.count(dep)) continue;
                adj[dep].push_back(m.moduleName);
                indeg[m.moduleName]++;
            }
        }

        std::vector<std::string> ready;
        for (const auto& n : nodes) {
            if (indeg[n] == 0) ready.push_back(n);
        }
        std::sort(ready.begin(), ready.end());

        std::vector<std::string> ordered;
        while (!ready.empty()) {
            const std::string current = ready.front();
            ready.erase(ready.begin());
            ordered.push_back(current);
            for (const auto& next : adj[current]) {
                indeg[next]--;
                if (indeg[next] == 0) {
                    ready.push_back(next);
                    std::sort(ready.begin(), ready.end());
                }
            }
        }

        if (ordered.size() < nodes.size()) {
            std::vector<std::string> remainder;
            for (const auto& n : nodes) {
                if (std::find(ordered.begin(), ordered.end(), n) == ordered.end()) {
                    remainder.push_back(n);
                }
            }
            std::sort(remainder.begin(), remainder.end());
            ordered.insert(ordered.end(), remainder.begin(), remainder.end());
            notes.push_back("Dependency cycle detected: fallback lexical ordering applied");
        }
        return ordered;
    }
};
