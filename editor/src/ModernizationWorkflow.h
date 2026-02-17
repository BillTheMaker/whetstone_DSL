#pragma once

// Step 441: Modernization Workflow Generation — converts modernization suggestions
// into an ordered workflow with routing (deterministic/LLM/human) and dependencies.

#include "ModernizationSuggester.h"

#include <string>
#include <vector>

enum class WorkItemRouting { Deterministic, LLM, Human };

struct ModernizeWorkItem {
    std::string id;
    std::string title;
    std::string fromPattern;
    std::string toReplacement;
    std::string annotation;
    WorkItemRouting routing = WorkItemRouting::Deterministic;
    ModernizeEffort effort = ModernizeEffort::QuickWin;
    int priority = 0;         // lower = execute first
    std::vector<std::string> dependsOn;
};

struct ModernizationSkeleton {
    std::string originalSource;
    std::string skeletonSource;  // modernized structure with placeholders
    std::string language;
    std::vector<std::string> annotations;
};

struct ModernizationWorkflow {
    std::string filePath;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<ModernizeWorkItem> items;
    ModernizationSkeleton skeleton;

    int countByRouting(WorkItemRouting r) const {
        int n = 0;
        for (const auto& item : items)
            if (item.routing == r) ++n;
        return n;
    }

    bool hasItemFor(const std::string& from) const {
        for (const auto& item : items)
            if (item.fromPattern == from) return true;
        return false;
    }

    std::vector<ModernizeWorkItem> itemsInOrder() const {
        auto sorted = items;
        std::sort(sorted.begin(), sorted.end(),
                  [](const ModernizeWorkItem& a, const ModernizeWorkItem& b) {
                      return a.priority < b.priority;
                  });
        return sorted;
    }

    ModernizeWorkItem itemById(const std::string& id) const {
        for (const auto& item : items)
            if (item.id == id) return item;
        return {};
    }
};

class ModernizationWorkflowGenerator {
public:
    static ModernizationWorkflow generate(const std::string& source,
                                          const ModernizationReport& report) {
        ModernizationWorkflow wf;
        wf.filePath = report.filePath;
        wf.sourceLanguage = report.sourceLanguage;
        wf.targetLanguage = report.targetLanguage;

        // Convert suggestions to work items with routing and priority
        int idx = 0;
        for (const auto& s : report.suggestions) {
            ModernizeWorkItem item;
            item.id = "mod-" + std::to_string(idx++);
            item.title = s.fromPattern + " -> " + s.toReplacement;
            item.fromPattern = s.fromPattern;
            item.toReplacement = s.toReplacement;
            item.annotation = s.annotation;
            item.effort = s.effort;
            item.routing = routeByEffort(s);
            item.priority = priorityForEffort(s.effort);
            wf.items.push_back(std::move(item));
        }

        // Set dependencies: deep refactors depend on quick wins completing first
        setDependencies(wf);

        // Generate skeleton
        wf.skeleton = generateSkeleton(source, report);

        return wf;
    }

private:
    static WorkItemRouting routeByEffort(const ModernizeSuggestion& s) {
        if (s.effort == ModernizeEffort::QuickWin)
            return WorkItemRouting::Deterministic;
        if (s.effort == ModernizeEffort::DeepRefactor)
            return WorkItemRouting::Human;
        // Moderate: depends on risk
        if (s.risk == "high")
            return WorkItemRouting::Human;
        return WorkItemRouting::LLM;
    }

    static int priorityForEffort(ModernizeEffort e) {
        switch (e) {
            case ModernizeEffort::QuickWin: return 0;
            case ModernizeEffort::Moderate: return 1;
            case ModernizeEffort::DeepRefactor: return 2;
        }
        return 1;
    }

    static void setDependencies(ModernizationWorkflow& wf) {
        // Collect quick-win IDs
        std::vector<std::string> quickWinIds;
        for (const auto& item : wf.items)
            if (item.effort == ModernizeEffort::QuickWin)
                quickWinIds.push_back(item.id);

        // Deep refactors depend on all quick wins
        for (auto& item : wf.items) {
            if (item.effort == ModernizeEffort::DeepRefactor) {
                item.dependsOn = quickWinIds;
            }
        }
    }

    static ModernizationSkeleton generateSkeleton(const std::string& source,
                                                   const ModernizationReport& report) {
        ModernizationSkeleton sk;
        sk.originalSource = source;
        sk.language = report.targetLanguage;

        // Collect annotations
        for (const auto& s : report.suggestions)
            sk.annotations.push_back(s.annotation);

        // Build skeleton: replace known patterns with placeholders
        std::string skel = source;
        for (const auto& s : report.suggestions) {
            if (s.fromPattern == "malloc/free") {
                replaceAll(skel, "malloc(", "/* @Modernize: smart_ptr */ malloc(");
                replaceAll(skel, "free(", "/* @Modernize: remove_free */ free(");
            }
            if (s.fromPattern == "gets") {
                replaceAll(skel, "gets(", "/* @Modernize: fgets */ gets(");
            }
            if (s.fromPattern == "sprintf") {
                replaceAll(skel, "sprintf(", "/* @Modernize: snprintf */ sprintf(");
            }
            if (s.fromPattern == "strcpy") {
                replaceAll(skel, "strcpy(", "/* @Modernize: strncpy */ strcpy(");
            }
            if (s.fromPattern == "goto") {
                replaceAll(skel, "goto ", "/* @Modernize: structured_control */ goto ");
            }
        }
        sk.skeletonSource = skel;
        return sk;
    }

    static void replaceAll(std::string& str, const std::string& from,
                            const std::string& to) {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::string::npos) {
            str.replace(pos, from.size(), to);
            pos += to.size();
        }
    }
};
