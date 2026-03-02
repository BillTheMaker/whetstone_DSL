#pragma once
#include "PolyglotProjectSpec.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace whetstone {

struct ABIBoundaryNode {
    std::string name;           // exported symbol name
    std::string kind;           // "function" | "struct"
    std::string fromComponent;  // component that exports this node
    std::string toComponent;    // component that imports it
    std::string fromLanguage;   // assignedLanguage of fromComponent
    std::string toLanguage;     // assignedLanguage of toComponent
    nlohmann::json signature;   // raw AST node JSON (pass-through)
};

class ABIBoundaryExtractor {
public:
    // Extract boundary nodes from all interfaces in the spec.
    // ast format: {"components": {"comp-name": {"nodes": [{name, kind, exported, ...}]}}}
    // For each interface (fromComponent → toComponent), collect exported nodes from
    // the from-side component. Non-exported nodes and missing components are skipped.
    static std::vector<ABIBoundaryNode> extract(
        const PolyglotProjectSpec& spec,
        const nlohmann::json& ast)
    {
        std::vector<ABIBoundaryNode> result;

        if (!ast.contains("components") || !ast["components"].is_object())
            return result;

        const auto& components = ast["components"];

        for (const auto& iface : spec.interfaces) {
            const std::string& from = iface.fromComponent;
            const std::string& to   = iface.toComponent;

            // The provider (toComponent) exports the symbols the caller (fromComponent) needs.
            if (!components.contains(to)) continue;
            const auto& toEntry = components[to];
            if (!toEntry.contains("nodes") || !toEntry["nodes"].is_array()) continue;

            std::string fromLang = findLanguage(spec, from);
            std::string toLang   = findLanguage(spec, to);

            for (const auto& node : toEntry["nodes"]) {
                if (!node.contains("exported")) continue;
                if (!node["exported"].get<bool>()) continue;

                ABIBoundaryNode bn;
                bn.name          = node.contains("name") ? node["name"].get<std::string>() : "";
                bn.kind          = node.contains("kind") ? node["kind"].get<std::string>() : "function";
                bn.fromComponent = from;
                bn.toComponent   = to;
                bn.fromLanguage  = fromLang;
                bn.toLanguage    = toLang;
                bn.signature     = node;

                if (!bn.name.empty())
                    result.push_back(std::move(bn));
            }
        }

        return result;
    }

private:
    static std::string findLanguage(const PolyglotProjectSpec& spec,
                                    const std::string& componentName)
    {
        for (const auto& section : spec.sections) {
            if (section.componentName == componentName)
                return section.assignedLanguage;
        }
        return {};
    }
};

} // namespace whetstone
