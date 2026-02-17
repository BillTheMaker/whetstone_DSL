#pragma once
// Step 545: Context Bundle Minimizer

#include <algorithm>
#include <set>
#include <string>
#include <vector>

struct ContextBundleInput {
    std::string taskitemId;
    std::vector<std::string> contractTargets;
    std::vector<std::string> contractSymbols;
    std::vector<std::string> contractOps;
    std::vector<std::string> fileContext;
    std::vector<std::string> projectContext;
    std::vector<std::string> dependencyContext;
    bool narrowOperation = true;
};

struct ContextBundleOutput {
    std::vector<std::string> minimalContext;
    std::vector<std::string> removedContext;
};

class ContextBundleMinimizer {
public:
    static ContextBundleOutput minimize(const ContextBundleInput& input) {
        ContextBundleOutput out;

        auto mustKeep = seedMustKeep(input);
        appendFiltered(input.fileContext, mustKeep, out.minimalContext, out.removedContext);

        if (!input.narrowOperation) {
            appendFiltered(input.projectContext, mustKeep, out.minimalContext, out.removedContext);
            appendFiltered(input.dependencyContext, mustKeep, out.minimalContext, out.removedContext);
        } else {
            appendFiltered(input.projectContext, mustKeep, out.minimalContext, out.removedContext,
                           /*strict=*/true);
            appendFiltered(input.dependencyContext, mustKeep, out.minimalContext, out.removedContext,
                           /*strict=*/true);
        }

        dedupe(out.minimalContext);
        dedupe(out.removedContext);
        return out;
    }

private:
    static std::set<std::string> seedMustKeep(const ContextBundleInput& input) {
        std::set<std::string> keys;
        for (const auto& t : input.contractTargets) keys.insert(t);
        for (const auto& s : input.contractSymbols) keys.insert(s);
        for (const auto& o : input.contractOps) keys.insert(o);
        return keys;
    }

    static void appendFiltered(const std::vector<std::string>& context,
                               const std::set<std::string>& keys,
                               std::vector<std::string>& keep,
                               std::vector<std::string>& drop,
                               bool strict = false) {
        for (const auto& item : context) {
            bool relevant = containsAny(item, keys);
            if (relevant || (!strict && looksStructural(item))) {
                keep.push_back(item);
            } else {
                drop.push_back(item);
            }
        }
    }

    static bool containsAny(const std::string& item,
                            const std::set<std::string>& keys) {
        for (const auto& k : keys) {
            if (!k.empty() && item.find(k) != std::string::npos) return true;
        }
        return false;
    }

    static bool looksStructural(const std::string& item) {
        return item.find("file:") == 0 || item.find("ast:") == 0 || item.find("diag:") == 0;
    }

    static void dedupe(std::vector<std::string>& items) {
        std::sort(items.begin(), items.end());
        items.erase(std::unique(items.begin(), items.end()), items.end());
    }
};
