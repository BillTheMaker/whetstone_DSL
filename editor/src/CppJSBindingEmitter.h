#pragma once
#include "ABIBoundaryExtractor.h"
#include <nlohmann/json.hpp>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace whetstone {

struct CppJSBinding {
    std::string cppCode;  // C++ N-API source snippet
    std::string jsCode;   // JavaScript source snippet
};

class CppJSBindingEmitter {
public:
    static CppJSBinding emit(const ABIBoundaryNode& node) {
        if (node.kind == "struct") return emitStruct(node);
        return emitFunction(node);
    }

private:
    static CppJSBinding emitFunction(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::string retRaw;
        if (sig.contains("returnType") && sig["returnType"].is_string())
            retRaw = sig["returnType"].get<std::string>();
        bool voidRet = retRaw.empty() || retRaw == "void";

        // Collect params
        std::vector<std::pair<std::string,std::string>> params; // name, type
        if (sig.contains("params") && sig["params"].is_array()) {
            for (const auto& p : sig["params"]) {
                std::string pn = p.contains("name") ? p["name"].get<std::string>() : "arg";
                std::string pt = p.contains("type") ? p["type"].get<std::string>() : "int";
                params.push_back({pn, pt});
            }
        }

        std::string camel = toCamelCase(node.name);
        std::string addonName = node.toComponent;
        // replace '-' with '_' for valid C identifiers
        for (char& c : addonName) if (c == '-') c = '_';

        // --- C++ side ---
        std::ostringstream cpp;
        cpp << "// N-API binding: " << node.fromComponent << " -> " << node.toComponent << "\n";
        cpp << "#include <node_api.h>\n";
        cpp << "napi_value js_" << node.name << "(napi_env env, napi_callback_info info) {\n";

        if (!params.empty()) {
            cpp << "    size_t argc = " << params.size() << ";\n";
            cpp << "    napi_value args[" << params.size() << "];\n";
            cpp << "    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);\n";
            for (int i = 0; i < (int)params.size(); ++i) {
                cpp << "    " << napiGetValue(params[i].second, params[i].first, i);
            }
        } else {
            cpp << "    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, nullptr);\n";
        }

        // Call native
        if (!voidRet) {
            cpp << "    " << toCppRetType(retRaw) << " result = " << node.name << "(";
        } else {
            cpp << "    " << node.name << "(";
        }
        for (int i = 0; i < (int)params.size(); ++i) {
            if (i > 0) cpp << ", ";
            cpp << params[i].first;
        }
        cpp << ");\n";

        // Return value
        cpp << "    napi_value ret;\n";
        if (!voidRet) {
            cpp << "    " << napiSetValue(retRaw, "result");
        } else {
            cpp << "    napi_get_undefined(env, &ret);\n";
        }
        cpp << "    return ret;\n}\n";

        // NAPI_MODULE_INIT
        cpp << "NAPI_MODULE_INIT() {\n";
        cpp << "    napi_value fn;\n";
        cpp << "    napi_create_function(env, \"" << camel
            << "\", NAPI_AUTO_LENGTH, js_" << node.name << ", nullptr, &fn);\n";
        cpp << "    napi_set_named_property(exports, \"" << camel << "\", fn);\n";
        cpp << "    return exports;\n}\n";

        // --- JS side ---
        std::ostringstream js;
        js << "// JS binding: " << node.fromComponent << " -> " << node.toComponent << "\n";
        js << "const addon = require('./" << addonName << "_addon');\n";
        js << "// " << camel << "(";
        for (int i = 0; i < (int)params.size(); ++i) {
            if (i > 0) js << ", ";
            js << params[i].first << ": " << toJSType(params[i].second);
        }
        js << "): " << (voidRet ? "void" : toJSType(retRaw)) << "\n";
        js << "const " << camel << " = addon." << camel << ";\n";
        js << "module.exports = { " << camel << " };\n";

        return {cpp.str(), js.str()};
    }

    static CppJSBinding emitStruct(const ABIBoundaryNode& node) {
        const auto& sig = node.signature;
        std::vector<std::pair<std::string,std::string>> fields;
        if (sig.contains("fields") && sig["fields"].is_array()) {
            for (const auto& f : sig["fields"]) {
                std::string fn = f.contains("name") ? f["name"].get<std::string>() : "field";
                std::string ft = f.contains("type") ? f["type"].get<std::string>() : "int";
                fields.push_back({fn, ft});
            }
        }

        std::string addonName = node.toComponent;
        for (char& c : addonName) if (c == '-') c = '_';

        // --- C++ side ---
        std::ostringstream cpp;
        cpp << "// N-API struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        cpp << "#include <node_api.h>\n";
        cpp << "napi_value " << node.name << "_to_js(napi_env env, const " << node.name << "& s) {\n";
        cpp << "    napi_value obj;\n";
        cpp << "    napi_create_object(env, &obj);\n";
        for (auto& [fn, ft] : fields) {
            cpp << "    { napi_value v; " << napiSetFieldValue(ft, "s." + fn)
                << " napi_set_named_property(env, obj, \"" << fn << "\", v); }\n";
        }
        cpp << "    return obj;\n}\n";

        // --- JS side ---
        std::ostringstream js;
        js << "// JS struct: " << node.fromComponent << " -> " << node.toComponent << "\n";
        js << "// " << node.name << ": { ";
        for (int i = 0; i < (int)fields.size(); ++i) {
            if (i > 0) js << ", ";
            js << fields[i].first << ": " << toJSType(fields[i].second);
        }
        js << " }\n";
        js << "const addon = require('./" << addonName << "_addon');\n";
        js << "module.exports = { " << node.name << ": addon." << node.name << " };\n";

        return {cpp.str(), js.str()};
    }

    static std::string napiGetValue(const std::string& ctype,
                                    const std::string& varName, int idx) {
        std::string s = "    std::string idx" + std::to_string(idx) + " = std::to_string(" + std::to_string(idx) + ");\n";
        std::ostringstream o;
        if (ctype == "char*" || ctype == "char *") {
            o << "size_t " << varName << "_len; "
              << "napi_get_value_string_utf8(env, args[" << idx << "], nullptr, 0, &" << varName << "_len);\n";
            o << "    char* " << varName << " = nullptr; /* allocate " << varName << "_len+1 */\n";
        } else if (ctype == "double" || ctype == "f64") {
            o << "double " << varName << "; "
              << "napi_get_value_double(env, args[" << idx << "], &" << varName << ");\n";
        } else if (ctype == "float" || ctype == "f32") {
            o << "double " << varName << "_d; "
              << "napi_get_value_double(env, args[" << idx << "], &" << varName << "_d); "
              << "float " << varName << " = (float)" << varName << "_d;\n";
        } else {
            // default: int32
            o << "int32_t " << varName << "; "
              << "napi_get_value_int32(env, args[" << idx << "], &" << varName << ");\n";
        }
        return "    " + o.str();
    }

    static std::string napiSetValue(const std::string& ctype, const std::string& varName) {
        if (ctype == "double" || ctype == "f64")
            return "napi_create_double(env, " + varName + ", &ret);\n";
        return "napi_create_int32(env, (int32_t)" + varName + ", &ret);\n";
    }

    static std::string napiSetFieldValue(const std::string& ctype, const std::string& expr) {
        if (ctype == "double" || ctype == "f64")
            return "napi_create_double(env, " + expr + ", &v);";
        return "napi_create_int32(env, (int32_t)(" + expr + "), &v);";
    }

    static std::string toCppRetType(const std::string& t) {
        if (t == "double" || t == "f64") return "double";
        if (t == "float"  || t == "f32") return "float";
        return "int32_t";
    }

    static std::string toJSType(const std::string& t) {
        if (t == "double" || t == "f64" || t == "float" || t == "f32") return "number";
        if (t == "char*" || t == "char *" || t == "string") return "string";
        if (t == "void") return "void";
        return "number";
    }

    // process_request → processRequest
    static std::string toCamelCase(const std::string& name) {
        std::string result;
        bool capitalizeNext = false;
        bool first = true;
        for (char c : name) {
            if (c == '_') { capitalizeNext = true; }
            else if (first) { result += static_cast<char>(std::tolower(static_cast<unsigned char>(c))); first = false; capitalizeNext = false; }
            else if (capitalizeNext) { result += static_cast<char>(std::toupper(static_cast<unsigned char>(c))); capitalizeNext = false; }
            else { result += c; }
        }
        return result;
    }
};

} // namespace whetstone
