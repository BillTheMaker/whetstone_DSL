#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <vector>

namespace whetstone {

class SCIPEmitter {
public:
    // Emit a SCIP-format index for the given boundary nodes.
    // Returns: {"schemaVersion":"0.1", "documents":[{language, uri, symbols:[...]}, ...]}
    // Each node appears in both its fromLanguage and toLanguage documents.
    static nlohmann::json emit(const std::vector<ABIBoundaryNode>& nodes) {
        // language → document stub
        struct DocStub {
            std::string uri;
            nlohmann::json symbols = nlohmann::json::array();
        };
        std::map<std::string, DocStub> docs;

        for (const auto& node : nodes) {
            // Provider side (toLanguage document)
            auto& toDoc = docs[node.toLanguage];
            if (toDoc.uri.empty()) toDoc.uri = "scip://" + node.toComponent;
            toDoc.symbols.push_back(makeSymbol(node, "provider"));

            // Caller side (fromLanguage document)
            auto& fromDoc = docs[node.fromLanguage];
            if (fromDoc.uri.empty()) fromDoc.uri = "scip://" + node.fromComponent;
            fromDoc.symbols.push_back(makeSymbol(node, "caller"));
        }

        nlohmann::json docArray = nlohmann::json::array();
        for (auto& [lang, stub] : docs) {
            docArray.push_back({
                {"language", lang},
                {"uri",      stub.uri},
                {"symbols",  stub.symbols}
            });
        }

        return {{"schemaVersion", "0.1"}, {"documents", docArray}};
    }

private:
    static nlohmann::json makeSymbol(const ABIBoundaryNode& node,
                                     const std::string& role) {
        return {
            {"name",          node.name},
            {"kind",          node.kind},
            {"fromComponent", node.fromComponent},
            {"toComponent",   node.toComponent},
            {"role",          role},
            {"scipSymbol",    node.toComponent + "/" + node.name + "."}
        };
    }
};

} // namespace whetstone
