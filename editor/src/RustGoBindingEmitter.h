#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace whetstone {

struct RustGoBinding {
    std::string rustCode;  // Rust source snippet
    std::string goCode;    // Go source snippet
};

class RustGoBindingEmitter {
public:
    static RustGoBinding emit(const ABIBoundaryNode& node) {
        if (node.kind == "struct") return emitStruct(node);
        return emitFunction(node);
    }

private:
    static RustGoBinding emitFunction(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::string retRaw;
        if (sig.contains("returnType") && sig["returnType"].is_string())
            retRaw = sig["returnType"].get<std::string>();
        bool voidRet = retRaw.empty() || retRaw == "void";

        std::vector<std::pair<std::string,std::string>> params;
        if (sig.contains("params") && sig["params"].is_array()) {
            for (const auto& p : sig["params"]) {
                std::string pn = p.contains("name") ? p["name"].get<std::string>() : "arg";
                std::string pt = p.contains("type") ? p["type"].get<std::string>() : "i32";
                params.push_back({pn, pt});
            }
        }

        std::string libName = node.toComponent;
        for (char& c : libName) if (c == '-') c = '_';

        // --- Rust side ---
        std::ostringstream rust;
        rust << "// Rust FFI export: " << node.fromComponent << " -> " << node.toComponent << "\n";
        rust << "use std::os::raw::c_void;\n";
        rust << "#[no_mangle]\n";
        rust << "pub extern \"C\" fn " << node.name << "(";

        bool first = true;
        for (auto& [pn, pt] : params) {
            if (!first) rust << ", ";
            first = false;
            rust << pn << ": " << toRustType(pt);
        }

        if (!voidRet) rust << ") -> " << toRustType(retRaw) << " {\n";
        else          rust << ") {\n";
        rust << "    todo!()\n}\n";

        // --- Go side ---
        std::ostringstream go;
        go << "// Go CGo caller: " << node.fromComponent << " -> " << node.toComponent << "\n";
        go << "/*\n";
        go << "#cgo LDFLAGS: -L. -l" << libName << "\n";
        go << "#include \"" << libName << "_abi.h\"\n";
        go << "*/\n";
        go << "import \"C\"\n";
        go << "import \"unsafe\"\n\n";

        std::string goName = toPascalCase(node.name);
        std::string goRet  = voidRet ? "" : toGoType(retRaw);

        go << "func " << goName << "(";
        bool firstGo = true;
        std::vector<std::pair<std::string,std::string>> goParams;
        for (auto& [pn, pt] : params) {
            if (!firstGo) go << ", ";
            firstGo = false;
            go << pn << " " << toGoType(pt);
            goParams.push_back({pn, pt});
        }
        if (!goRet.empty()) go << ") " << goRet << " {\n";
        else                go << ") {\n";

        if (!voidRet) go << "    return " << goRet << "(C." << node.name << "(";
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
        go << "var _ = unsafe.Pointer(nil)\n";

        return {rust.str(), go.str()};
    }

    static RustGoBinding emitStruct(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::string libName = node.toComponent;
        for (char& c : libName) if (c == '-') c = '_';

        // --- Rust side ---
        std::ostringstream rust;
        rust << "// Rust FFI struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        rust << "#[repr(C)]\n";
        rust << "pub struct " << node.name << " {\n";
        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& field : sig["fields"]) {
                std::string fn = field.contains("name") ? field["name"].get<std::string>() : "field";
                std::string ft = field.contains("type") ? field["type"].get<std::string>() : "i32";
                rust << "    pub " << fn << ": " << toRustType(ft) << ",\n";
            }
        }
        rust << "}\n";

        // --- Go side ---
        std::ostringstream go;
        go << "// Go CGo struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        go << "/*\n#include \"" << libName << "_abi.h\"\n*/\n";
        go << "import \"C\"\n\n";
        go << "type " << node.name << " = C.struct_" << node.name << "\n";

        return {rust.str(), go.str()};
    }

    static std::string toRustType(const std::string& t) {
        if (t == "void")   return "()";
        if (t == "i32" || t == "int32_t" || t == "int") return "i32";
        if (t == "u32" || t == "uint32_t") return "u32";
        if (t == "i64" || t == "int64_t")  return "i64";
        if (t == "u64" || t == "uint64_t") return "u64";
        if (t == "usize" || t == "size_t") return "usize";
        if (t == "f32" || t == "float")    return "f32";
        if (t == "f64" || t == "double")   return "f64";
        if (t.find('*') != std::string::npos) return "*mut c_void";
        return "c_void";
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

    static std::string toPascalCase(const std::string& name) {
        std::string result;
        bool cap = true;
        for (char c : name) {
            if (c == '_') { cap = true; }
            else if (cap) { result += static_cast<char>(std::toupper(static_cast<unsigned char>(c))); cap = false; }
            else { result += c; }
        }
        return result;
    }
};

} // namespace whetstone
