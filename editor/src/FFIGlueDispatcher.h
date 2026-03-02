#pragma once
#include "ABIBoundaryExtractor.h"
#include "CHeaderEmitter.h"
#include "RustPythonBindingEmitter.h"
#include "GoCppBindingEmitter.h"
#include "RustGoBindingEmitter.h"
#include "CppJSBindingEmitter.h"
#include "DWARFBoundaryAnnotator.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <string>
#include <vector>

namespace whetstone {

class FFIGlueDispatcher {
public:
    // Dispatch a single node to the correct emitter.
    // Returns: {rustCode, secondaryCode, emitter}
    static nlohmann::json dispatch(const ABIBoundaryNode& node) {
        std::string from = lower(node.fromLanguage);
        std::string to   = lower(node.toLanguage);

        if (from == "python" && to == "rust") {
            auto b = RustPythonBindingEmitter::emit(node);
            return {{"rustCode", b.rustCode}, {"secondaryCode", b.pythonCode},
                    {"emitter", "RustPython"}};
        }
        if (from == "go" && to == "c++") {
            auto b = GoCppBindingEmitter::emit(node);
            return {{"rustCode", b.cppCode}, {"secondaryCode", b.goCode},
                    {"emitter", "GoCpp"}};
        }
        if (from == "go" && to == "rust") {
            auto b = RustGoBindingEmitter::emit(node);
            return {{"rustCode", b.rustCode}, {"secondaryCode", b.goCode},
                    {"emitter", "RustGo"}};
        }
        if ((from == "typescript" || from == "javascript") && to == "c++") {
            auto b = CppJSBindingEmitter::emit(node);
            return {{"rustCode", b.cppCode}, {"secondaryCode", b.jsCode},
                    {"emitter", "CppJS"}};
        }
        // fallback
        return {{"rustCode", ""}, {"secondaryCode", ""}, {"emitter", "unknown"}};
    }

    // Orchestrate all nodes.
    // Returns: {bindings: [...], c_header: "...", dwarf_annotations: [...]}
    static nlohmann::json generateAll(const std::vector<ABIBoundaryNode>& nodes,
                                      const std::string& projectName)
    {
        nlohmann::json bindings = nlohmann::json::array();
        for (const auto& node : nodes) {
            bindings.push_back(dispatch(node));
        }

        std::string header = CHeaderEmitter::emit(nodes, projectName);
        nlohmann::json dwarf = DWARFBoundaryAnnotator::annotate(nodes);

        return {{"bindings", bindings},
                {"c_header", header},
                {"dwarf_annotations", dwarf}};
    }

private:
    static std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    }
};

} // namespace whetstone
