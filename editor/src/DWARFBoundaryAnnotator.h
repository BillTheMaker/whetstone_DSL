#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace whetstone {

class DWARFBoundaryAnnotator {
public:
    // Produce one JSON annotation object per ABIBoundaryNode.
    static nlohmann::json annotate(const std::vector<ABIBoundaryNode>& nodes) {
        nlohmann::json result = nlohmann::json::array();
        for (const auto& node : nodes) {
            nlohmann::json ann;
            ann["astNodeId"]         = makeAstNodeId(node);
            ann["name"]              = node.name;
            ann["kind"]              = node.kind;
            ann["fromComponent"]     = node.fromComponent;
            ann["toComponent"]       = node.toComponent;
            ann["fromLanguage"]      = node.fromLanguage;
            ann["toLanguage"]        = node.toLanguage;
            ann["dwarfTag"]          = dwarfTag(node.kind);
            ann["crossLangBoundary"] = true;
            result.push_back(std::move(ann));
        }
        return result;
    }

private:
    // Deterministic: "<fromComponent>:<toComponent>:<name>:<kind>"
    static std::string makeAstNodeId(const ABIBoundaryNode& node) {
        return node.fromComponent + ":" + node.toComponent + ":" + node.name + ":" + node.kind;
    }

    static std::string dwarfTag(const std::string& kind) {
        if (kind == "function") return "DW_TAG_subprogram";
        if (kind == "struct")   return "DW_TAG_structure_type";
        return "DW_TAG_variable";
    }
};

} // namespace whetstone
