#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace whetstone {

struct GoCppBinding {
    std::string cppCode;  // C++ source snippet
    std::string goCode;   // Go source snippet
};

class GoCppBindingEmitter {
public:
    static GoCppBinding emit(const ABIBoundaryNode& node) {
        if (node.kind == "struct") return emitStruct(node);
        return emitFunction(node);
    }

private:
    static GoCppBinding emitFunction(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::string retRaw;
        if (sig.contains("returnType") && sig["returnType"].is_string())
            retRaw = sig["returnType"].get<std::string>();

        // --- C++ side ---
        std::ostringstream cpp;
        cpp << "// FFI: " << node.fromComponent << " -> " << node.toComponent << "\n";
        cpp << "extern \"C\" {\n";
        cpp << toCppType(retRaw) << " " << node.name << "(";

        bool hasParams = false;
        if (sig.contains("params") && sig["params"].is_array()) {
            bool first = true;
            for (const auto& param : sig["params"]) {
                if (!first) cpp << ", ";
                first = false;
                std::string pt = param.contains("type") ? param["type"].get<std::string>() : "void*";
                std::string pn = param.contains("name") ? param["name"].get<std::string>() : "arg";
                cpp << toCppType(pt) << " " << pn;
                hasParams = true;
            }
        }
        cpp << ");\n}\n";

        // --- Go side ---
        std::ostringstream go;
        go << "// FFI: " << node.fromComponent << " -> " << node.toComponent << "\n";
        go << "/*\n#include \"" << node.toComponent << "_abi.h\"\n*/\n";
        go << "import \"C\"\n";
        go << "import \"unsafe\"\n\n";

        bool voidRet = retRaw.empty() || retRaw == "void";
        std::string goRetType = voidRet ? "" : toGoType(retRaw);
        std::string goFuncName = toPascalCase(node.name);

        go << "func " << goFuncName << "(";
        bool firstGo = true;
        std::vector<std::pair<std::string,std::string>> goParams;
        if (sig.contains("params") && sig["params"].is_array()) {
            for (const auto& param : sig["params"]) {
                std::string pn = param.contains("name") ? param["name"].get<std::string>() : "arg";
                std::string pt = param.contains("type") ? param["type"].get<std::string>() : "void*";
                goParams.push_back({pn, pt});
                if (!firstGo) go << ", ";
                firstGo = false;
                go << pn << " " << toGoType(pt);
            }
        }
        if (!goRetType.empty()) go << ") " << goRetType << " {\n";
        else                    go << ") {\n";

        // body: call C function
        if (!voidRet) go << "    return " << goRetType << "(C." << node.name << "(";
        else          go << "    C." << node.name << "(";

        bool firstCall = true;
        for (auto& [pn, pt] : goParams) {
            if (!firstCall) go << ", ";
            firstCall = false;
            go << toCGoType(pt) << "(" << pn << ")";
        }
        if (!voidRet) go << "))\n";
        else          go << ")\n";

        go << "}\n";

        // suppress unused import warning
        go << "var _ = unsafe.Pointer(nil)\n";

        return {cpp.str(), go.str()};
    }

    static GoCppBinding emitStruct(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;

        // --- C++ side ---
        std::ostringstream cpp;
        cpp << "// FFI struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        cpp << "extern \"C\" {\n";
        cpp << "struct " << node.name << " {\n";
        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& field : sig["fields"]) {
                std::string fn = field.contains("name") ? field["name"].get<std::string>() : "field";
                std::string ft = field.contains("type") ? field["type"].get<std::string>() : "int";
                cpp << "    " << toCppType(ft) << " " << fn << ";\n";
            }
        }
        cpp << "};\n}\n";

        // --- Go side ---
        std::ostringstream go;
        go << "// FFI struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        go << "/*\n#include \"" << node.toComponent << "_abi.h\"\n*/\n";
        go << "import \"C\"\n\n";
        go << "type " << node.name << " = C.struct_" << node.name << "\n";

        return {cpp.str(), go.str()};
    }

    static std::string toCppType(const std::string& t) {
        if (t == "void")   return "void";
        if (t == "i32" || t == "int32_t" || t == "int") return "int";
        if (t == "u32" || t == "uint32_t") return "unsigned int";
        if (t == "i64" || t == "int64_t")  return "int64_t";
        if (t == "u64" || t == "uint64_t") return "uint64_t";
        if (t == "usize" || t == "size_t") return "size_t";
        if (t == "f32" || t == "float")    return "float";
        if (t == "f64" || t == "double")   return "double";
        if (t.find('*') != std::string::npos) return t;
        return "void*";
    }

    static std::string toGoType(const std::string& t) {
        if (t == "void")   return "";
        if (t == "i32" || t == "int32_t" || t == "int") return "int32";
        if (t == "u32" || t == "uint32_t") return "uint32";
        if (t == "i64" || t == "int64_t")  return "int64";
        if (t == "u64" || t == "uint64_t") return "uint64";
        if (t == "usize" || t == "size_t") return "uint";
        if (t == "f32" || t == "float")    return "float32";
        if (t == "f64" || t == "double")   return "float64";
        if (t.find('*') != std::string::npos) return "unsafe.Pointer";
        return "unsafe.Pointer";
    }

    static std::string toCGoType(const std::string& t) {
        if (t == "i32" || t == "int32_t" || t == "int") return "C.int";
        if (t == "u32" || t == "uint32_t") return "C.uint";
        if (t == "i64" || t == "int64_t")  return "C.longlong";
        if (t == "u64" || t == "uint64_t") return "C.ulonglong";
        if (t == "usize" || t == "size_t") return "C.size_t";
        if (t == "f32" || t == "float")    return "C.float";
        if (t == "f64" || t == "double")   return "C.double";
        if (t.find('*') != std::string::npos) return "unsafe.Pointer";
        return "unsafe.Pointer";
    }

    // "handle_request" → "HandleRequest"
    static std::string toPascalCase(const std::string& name) {
        std::string result;
        bool capitalizeNext = true;
        for (char c : name) {
            if (c == '_') {
                capitalizeNext = true;
            } else if (capitalizeNext) {
                result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                capitalizeNext = false;
            } else {
                result += c;
            }
        }
        return result;
    }
};

} // namespace whetstone
