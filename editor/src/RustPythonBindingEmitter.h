#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace whetstone {

struct RustPythonBinding {
    std::string rustCode;    // Rust source snippet
    std::string pythonCode;  // Python source snippet
};

class RustPythonBindingEmitter {
public:
    static RustPythonBinding emit(const ABIBoundaryNode& node) {
        if (node.kind == "struct") return emitStruct(node);
        return emitFunction(node);
    }

private:
    static RustPythonBinding emitFunction(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::string retTypeRaw;
        if (sig.contains("returnType") && sig["returnType"].is_string())
            retTypeRaw = sig["returnType"].get<std::string>();

        // --- Rust ---
        std::ostringstream rust;
        rust << "// FFI binding: " << node.fromComponent << " -> " << node.toComponent << "\n";
        rust << "mod ffi {\n";
        rust << "    use std::os::raw::c_void;\n";
        rust << "    extern \"C\" {\n";
        rust << "        pub fn " << node.name << "(";

        bool hasParams = false;
        if (sig.contains("params") && sig["params"].is_array()) {
            bool first = true;
            for (const auto& param : sig["params"]) {
                if (!first) rust << ", ";
                first = false;
                std::string pname = param.contains("name") ? param["name"].get<std::string>() : "arg";
                std::string ptype = param.contains("type") ? param["type"].get<std::string>() : "i32";
                rust << pname << ": " << toRustType(ptype);
                hasParams = true;
            }
        }
        if (!hasParams) rust << "";  // empty params OK in Rust extern

        std::string rustRet = retTypeRaw.empty() || retTypeRaw == "void" ? "" : " -> " + toRustType(retTypeRaw);
        rust << ")" << rustRet << ";\n";
        rust << "    }\n";
        rust << "}\n";

        // --- Python ---
        std::ostringstream py;
        py << "import ctypes\n";
        py << "# lib = ctypes.CDLL(\"./lib" << node.toComponent << ".so\")\n";
        py << "lib." << node.name << ".argtypes = [";

        bool firstPy = true;
        if (sig.contains("params") && sig["params"].is_array()) {
            for (const auto& param : sig["params"]) {
                if (!firstPy) py << ", ";
                firstPy = false;
                std::string ptype = param.contains("type") ? param["type"].get<std::string>() : "i32";
                py << toCType(ptype);
            }
        }
        py << "]\n";

        std::string pyRet = (retTypeRaw.empty() || retTypeRaw == "void") ? "None" : toCType(retTypeRaw);
        py << "lib." << node.name << ".restype = " << pyRet << "\n";

        return {rust.str(), py.str()};
    }

    static RustPythonBinding emitStruct(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;

        // --- Rust ---
        std::ostringstream rust;
        rust << "// FFI struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        rust << "#[repr(C)]\n";
        rust << "pub struct " << node.name << " {\n";

        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& field : sig["fields"]) {
                std::string fname = field.contains("name") ? field["name"].get<std::string>() : "field";
                std::string ftype = field.contains("type") ? field["type"].get<std::string>() : "i32";
                rust << "    pub " << fname << ": " << toRustType(ftype) << ",\n";
            }
        }
        rust << "}\n";

        // --- Python ---
        std::ostringstream py;
        py << "import ctypes\n";
        py << "class " << node.name << "(ctypes.Structure):\n";
        py << "    _fields_ = [\n";

        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& field : sig["fields"]) {
                std::string fname = field.contains("name") ? field["name"].get<std::string>() : "field";
                std::string ftype = field.contains("type") ? field["type"].get<std::string>() : "i32";
                py << "        (\"" << fname << "\", " << toCType(ftype) << "),\n";
            }
        }
        py << "    ]\n";

        return {rust.str(), py.str()};
    }

    static std::string toRustType(const std::string& t) {
        if (t == "void")    return "()";
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

    static std::string toCType(const std::string& t) {
        if (t == "void")   return "None";
        if (t == "i32" || t == "int32_t" || t == "int") return "ctypes.c_int32";
        if (t == "u32" || t == "uint32_t") return "ctypes.c_uint32";
        if (t == "i64" || t == "int64_t")  return "ctypes.c_int64";
        if (t == "u64" || t == "uint64_t") return "ctypes.c_uint64";
        if (t == "usize" || t == "size_t") return "ctypes.c_size_t";
        if (t == "f32" || t == "float")    return "ctypes.c_float";
        if (t == "f64" || t == "double")   return "ctypes.c_double";
        if (t.find('*') != std::string::npos) return "ctypes.c_void_p";
        return "ctypes.c_void_p";
    }
};

} // namespace whetstone
