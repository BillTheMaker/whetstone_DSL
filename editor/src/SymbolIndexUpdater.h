#pragma once
#include "CrossLanguageSymbolTable.h"
#include "ABIBoundaryExtractor.h"
#include <set>
#include <string>
#include <vector>

namespace whetstone {

struct UpdateResult {
    int added     = 0;
    int removed   = 0;
    int unchanged = 0;
};

class SymbolIndexUpdater {
public:
    // Diff oldNodes vs newNodes; update table in place.
    // Node identity key: toComponent/name.fromLanguage:toLanguage
    static UpdateResult update(CrossLanguageSymbolTable& table,
                               const std::vector<ABIBoundaryNode>& oldNodes,
                               const std::vector<ABIBoundaryNode>& newNodes)
    {
        std::set<std::string> oldKeys, newKeys;
        for (auto& n : oldNodes) oldKeys.insert(nodeKey(n));
        for (auto& n : newNodes) newKeys.insert(nodeKey(n));

        UpdateResult result;
        for (auto& n : newNodes) {
            if (oldKeys.count(nodeKey(n))) ++result.unchanged;
            else                           ++result.added;
        }
        for (auto& n : oldNodes) {
            if (!newKeys.count(nodeKey(n))) ++result.removed;
        }

        // Rebuild table from newNodes (clean remove-and-reinsert)
        table = CrossLanguageSymbolTable{};
        for (auto& n : newNodes) table.insert(n);

        return result;
    }

private:
    static std::string nodeKey(const ABIBoundaryNode& n) {
        return n.toComponent + "/" + n.name + "." + n.fromLanguage + ":" + n.toLanguage;
    }
};

} // namespace whetstone
