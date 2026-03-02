#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace whetstone {

class CHeaderEmitter {
public:
    // Emit a complete C ABI header for the given boundary nodes.
    // headerName is used to form the include guard (e.g., "sort_array" → SORT_ARRAY_ABI_H).
    // Returns the full header as a std::string ending with '\n'.
    static std::string emit(const std::vector<ABIBoundaryNode>& nodes,
                             const std::string& headerName)
    {
        std::string guard = toUpperGuard(headerName);
        std::ostringstream out;

        out << "#ifndef " << guard << "\n";
        out << "#define " << guard << "\n\n";
        out << "#include <stdint.h>\n";
        out << "#include <stddef.h>\n\n";
        out << "#ifdef __cplusplus\n";
        out << "extern \"C\" {\n";
        out << "#endif\n\n";

        for (const auto& node : nodes) {
            out << "/* boundary: " << node.fromComponent
                << " -> " << node.toComponent << " */\n";
            if (node.kind == "struct") {
                out << emitStruct(node);
            } else {
                out << emitFunction(node);
            }
            out << "\n";
        }

        out << "#ifdef __cplusplus\n";
        out << "}\n";
        out << "#endif\n\n";
        out << "#endif /* " << guard << " */\n";

        return out.str();
    }

private:
    static std::string emitFunction(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;

        std::string retType = "void";
        if (sig.contains("returnType") && sig["returnType"].is_string())
            retType = sig["returnType"].get<std::string>();

        std::ostringstream out;
        out << retType << " " << node.name << "(";

        bool hasParams = false;
        if (sig.contains("params") && sig["params"].is_array() && !sig["params"].empty()) {
            bool first = true;
            for (const auto& param : sig["params"]) {
                if (!first) out << ", ";
                first = false;
                std::string ptype = param.contains("type") ? param["type"].get<std::string>() : "void*";
                std::string pname = param.contains("name") ? param["name"].get<std::string>() : "";
                out << ptype;
                if (!pname.empty()) out << " " << pname;
                hasParams = true;
            }
        }
        if (!hasParams) out << "void";

        out << ");";
        return out.str();
    }

    static std::string emitStruct(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::ostringstream out;

        out << "typedef struct {\n";

        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& field : sig["fields"]) {
                std::string ftype = field.contains("type") ? field["type"].get<std::string>() : "void*";
                std::string fname = field.contains("name") ? field["name"].get<std::string>() : "field";
                out << "    " << ftype << " " << fname << ";\n";
            }
        }

        out << "} " << node.name << ";";
        return out.str();
    }

    static std::string toUpperGuard(const std::string& name) {
        std::string result;
        result.reserve(name.size() + 8);
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)))
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            else
                result += '_';
        }
        result += "_ABI_H";
        return result;
    }
};

} // namespace whetstone
