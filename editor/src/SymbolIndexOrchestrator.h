#pragma once
#include "SCIPEmitter.h"
#include "CrossLanguageSymbolTable.h"
#include "SymbolIndexUpdater.h"
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace whetstone {

class SymbolIndexOrchestrator {
public:
    // Build index from scratch: insert all nodes into table, emit SCIP index.
    static nlohmann::json orchestrate(const std::vector<ABIBoundaryNode>& nodes,
                                      CrossLanguageSymbolTable& table)
    {
        for (const auto& node : nodes) table.insert(node);
        auto scipIndex = SCIPEmitter::emit(nodes);
        return {{"success",    true},
                {"node_count", (int)nodes.size()},
                {"scip_index", scipIndex}};
    }

    // Incremental update: delegate to SymbolIndexUpdater.
    static UpdateResult update(CrossLanguageSymbolTable& table,
                               const std::vector<ABIBoundaryNode>& oldNodes,
                               const std::vector<ABIBoundaryNode>& newNodes)
    {
        return SymbolIndexUpdater::update(table, oldNodes, newNodes);
    }
};

} // namespace whetstone
