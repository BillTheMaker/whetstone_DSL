#pragma once
// Step 634: Job schema -> typed C++ structs generator

#include <nlohmann/json.hpp>

#include <cctype>
#include <string>
#include <vector>

using json = nlohmann::json;

struct SchemaToCppOutput {
    bool success = false;
    std::string headerCode;
    std::string cmakeInterfaceTarget;
    std::vector<std::string> errors;
};

class SchemaToCppGenerator {
public:
    static SchemaToCppOutput generate(const json& schema,
                                      const std::string& headerName,
                                      const std::string& targetName) {
        SchemaToCppOutput out;
        if (!schema.is_object()) {
            out.errors.push_back("schema_invalid");
            return out;
        }
        if (!schema.contains("title") ||
            !schema["title"].is_string() ||
            schema["title"].get<std::string>().empty()) {
            out.errors.push_back("schema_title_missing");
            return out;
        }
        const std::string structName = sanitizeIdentifier(
            schema["title"].get<std::string>());
        if (structName.empty()) {
            out.errors.push_back("schema_title_missing");
            return out;
        }

        std::string code;
        code += "#pragma once\n";
        code += "#include <nlohmann/json.hpp>\n";
        code += "#include <string>\n\n";
        code += "struct " + structName + " {\n";

        std::vector<std::pair<std::string, std::string>> fields;
        if (schema.contains("properties") && schema["properties"].is_object()) {
            for (auto it = schema["properties"].begin(); it != schema["properties"].end(); ++it) {
                std::string fieldName = sanitizeIdentifier(it.key());
                std::string cppType = mapType(it.value().value("type", "string"));
                if (fieldName.empty()) continue;
                fields.push_back({fieldName, cppType});
                code += "    " + cppType + " " + fieldName + ";\n";
            }
        }
        if (fields.empty()) {
            fields.push_back({"raw", "std::string"});
            code += "    std::string raw;\n";
        }
        code += "};\n\n";

        code += "inline void to_json(nlohmann::json& j, const " + structName + "& v) {\n";
        code += "    j = nlohmann::json::object();\n";
        for (const auto& field : fields) {
            code += "    j[\"" + field.first + "\"] = v." + field.first + ";\n";
        }
        code += "}\n\n";

        code += "inline void from_json(const nlohmann::json& j, " + structName + "& v) {\n";
        for (const auto& field : fields) {
            code += "    if (j.contains(\"" + field.first + "\")) j.at(\"" + field.first + "\").get_to(v." + field.first + ");\n";
        }
        code += "}\n\n";

        code += "inline bool validate" + structName + "(const " + structName + "& v) {\n";
        code += "    (void)v;\n";
        code += "    return true;\n";
        code += "}\n";

        out.success = true;
        out.headerCode = code;
        out.cmakeInterfaceTarget =
            "add_library(" + targetName + " INTERFACE)\n"
            "target_include_directories(" + targetName + " INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)\n"
            "target_sources(" + targetName + " INTERFACE include/" + headerName + ")\n";
        return out;
    }

    static std::string toolName() {
        return "whetstone_schema_to_cpp";
    }

private:
    static std::string mapType(const std::string& type) {
        if (type == "integer") return "int";
        if (type == "number") return "double";
        if (type == "boolean") return "bool";
        return "std::string";
    }

    static std::string sanitizeIdentifier(const std::string& raw) {
        std::string out;
        out.reserve(raw.size());
        for (char ch : raw) {
            if (std::isalnum(static_cast<unsigned char>(ch))) out.push_back(ch);
            else if (ch == '_' || ch == '-') out.push_back('_');
        }
        if (out.empty()) return out;
        if (std::isdigit(static_cast<unsigned char>(out.front()))) {
            out = "_" + out;
        }
        return out;
    }
};
