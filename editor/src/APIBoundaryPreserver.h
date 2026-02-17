#pragma once

// Step 445: API Boundary Preservation — ensures migrated modules maintain
// their public API surface with FFI annotations and @Contract verification.

#include "MigrationPlanGenerator.h"

#include <string>
#include <vector>

struct FunctionSignature {
    std::string name;
    std::string returnType;
    std::vector<std::string> paramTypes;
    std::vector<std::string> paramNames;
    bool isPublic = true;
};

struct TypeMapping {
    std::string sourceName;
    std::string sourceLanguage;
    std::string targetName;
    std::string targetLanguage;
    std::string note;  // compatibility note
};

struct ContractAnnotation {
    std::string functionName;
    std::string precondition;
    std::string postcondition;
};

struct FFIBoundary {
    std::string functionName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::string annotation;  // e.g. @FFI(extern "C"), #[no_mangle], etc.
};

struct APIBoundaryReport {
    std::string moduleName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<FunctionSignature> publicAPI;
    std::vector<TypeMapping> typeMappings;
    std::vector<ContractAnnotation> contracts;
    std::vector<FFIBoundary> ffiBoundaries;
    bool apiPreserved = true;

    bool hasFunction(const std::string& name) const {
        for (const auto& f : publicAPI)
            if (f.name == name) return true;
        return false;
    }

    bool hasTypeMapping(const std::string& sourceName) const {
        for (const auto& t : typeMappings)
            if (t.sourceName == sourceName) return true;
        return false;
    }

    bool hasContract(const std::string& funcName) const {
        for (const auto& c : contracts)
            if (c.functionName == funcName) return true;
        return false;
    }

    bool hasFFI(const std::string& funcName) const {
        for (const auto& f : ffiBoundaries)
            if (f.functionName == funcName) return true;
        return false;
    }
};

class APIBoundaryPreserver {
public:
    static APIBoundaryReport analyze(const std::string& source,
                                     const MigrationUnit& unit) {
        APIBoundaryReport report;
        report.moduleName = unit.moduleName;
        report.sourceLanguage = unit.sourceLanguage;
        report.targetLanguage = unit.targetLanguage;

        // Extract public API from source
        report.publicAPI = extractPublicFunctions(source, unit.exports);

        // Generate type mappings
        report.typeMappings = generateTypeMappings(source, unit);

        // Generate contracts for public functions
        report.contracts = generateContracts(report.publicAPI, source);

        // Generate FFI boundaries for mixed-language operation
        report.ffiBoundaries = generateFFIBoundaries(report.publicAPI, unit);

        report.apiPreserved = true;
        return report;
    }

    static bool verifyPreservation(const APIBoundaryReport& original,
                                    const APIBoundaryReport& migrated) {
        // Every original public function must exist in migrated
        for (const auto& f : original.publicAPI) {
            if (!migrated.hasFunction(f.name)) return false;
        }
        return true;
    }

private:
    static std::vector<FunctionSignature> extractPublicFunctions(
        const std::string& source, const std::vector<std::string>& exports) {
        std::vector<FunctionSignature> sigs;
        for (const auto& name : exports) {
            FunctionSignature sig;
            sig.name = name;
            sig.isPublic = true;
            // Extract return type and params from source (heuristic)
            extractSignatureDetails(source, name, sig);
            sigs.push_back(std::move(sig));
        }
        return sigs;
    }

    static void extractSignatureDetails(const std::string& source,
                                         const std::string& name,
                                         FunctionSignature& sig) {
        // Find "type name(" pattern
        size_t namePos = source.find(name + "(");
        if (namePos == std::string::npos) {
            sig.returnType = "int";  // default for C
            return;
        }
        // Extract return type (word before function name)
        size_t start = namePos;
        while (start > 0 && source[start - 1] == ' ') --start;
        size_t typeEnd = start;
        while (start > 0 && (std::isalnum((unsigned char)source[start - 1]) ||
               source[start - 1] == '_' || source[start - 1] == '*')) --start;
        sig.returnType = source.substr(start, typeEnd - start);
        if (sig.returnType.empty()) sig.returnType = "int";

        // Extract parameter list
        size_t parenOpen = source.find('(', namePos);
        size_t parenClose = source.find(')', parenOpen);
        if (parenOpen != std::string::npos && parenClose != std::string::npos) {
            std::string params = source.substr(parenOpen + 1, parenClose - parenOpen - 1);
            parseParams(params, sig);
        }
    }

    static void parseParams(const std::string& params, FunctionSignature& sig) {
        if (params.empty() || params == "void") return;
        size_t pos = 0;
        while (pos < params.size()) {
            size_t comma = params.find(',', pos);
            if (comma == std::string::npos) comma = params.size();
            std::string param = trim(params.substr(pos, comma - pos));
            if (!param.empty()) {
                // Split "type name" — last word is name, rest is type
                size_t lastSpace = param.rfind(' ');
                if (lastSpace != std::string::npos) {
                    sig.paramTypes.push_back(trim(param.substr(0, lastSpace)));
                    sig.paramNames.push_back(trim(param.substr(lastSpace + 1)));
                } else {
                    sig.paramTypes.push_back(param);
                    sig.paramNames.push_back("arg" + std::to_string(sig.paramNames.size()));
                }
            }
            pos = comma + 1;
        }
    }

    static std::vector<TypeMapping> generateTypeMappings(const std::string& source,
                                                          const MigrationUnit& unit) {
        std::vector<TypeMapping> mappings;
        std::string tgt = unit.targetLanguage;

        // Common C → target type mappings
        auto addIf = [&](const std::string& pattern, const std::string& src,
                         const std::string& dst, const std::string& note) {
            if (source.find(pattern) != std::string::npos)
                mappings.push_back({src, unit.sourceLanguage, dst, tgt, note});
        };

        if (tgt == "rust") {
            addIf("int ", "int", "i32", "C int maps to Rust i32");
            addIf("char*", "char*", "&str / String", "C string to Rust string type");
            addIf("void*", "void*", "*mut c_void", "Opaque pointer in FFI");
            addIf("struct ", "struct", "struct", "C struct maps to Rust struct");
        } else if (tgt == "java") {
            addIf("int ", "int", "int", "Direct mapping");
            addIf("char*", "char*", "String", "C string to Java String");
            addIf("struct ", "struct", "class", "C struct maps to Java class");
        } else if (tgt == "python") {
            addIf("int ", "int", "int", "Direct mapping");
            addIf("char*", "char*", "str", "C string to Python str");
        }
        return mappings;
    }

    static std::vector<ContractAnnotation> generateContracts(
        const std::vector<FunctionSignature>& api, const std::string& source) {
        std::vector<ContractAnnotation> contracts;
        for (const auto& f : api) {
            ContractAnnotation c;
            c.functionName = f.name;
            // Generate pre/post conditions from signature
            if (!f.paramTypes.empty()) {
                // Pointer params get non-null precondition
                for (size_t i = 0; i < f.paramTypes.size(); ++i) {
                    if (f.paramTypes[i].find('*') != std::string::npos) {
                        c.precondition += f.paramNames[i] + " != NULL; ";
                    }
                }
            }
            if (f.returnType.find('*') != std::string::npos) {
                c.postcondition = "result may be NULL (caller must check)";
            } else if (f.returnType == "int") {
                c.postcondition = "returns int result";
            }
            if (!c.precondition.empty() || !c.postcondition.empty())
                contracts.push_back(std::move(c));
        }
        return contracts;
    }

    static std::vector<FFIBoundary> generateFFIBoundaries(
        const std::vector<FunctionSignature>& api, const MigrationUnit& unit) {
        std::vector<FFIBoundary> boundaries;
        for (const auto& f : api) {
            FFIBoundary ffi;
            ffi.functionName = f.name;
            ffi.sourceLanguage = unit.sourceLanguage;
            ffi.targetLanguage = unit.targetLanguage;
            if (unit.targetLanguage == "rust") {
                ffi.annotation = "@FFI(#[no_mangle] pub extern \"C\")";
            } else if (unit.targetLanguage == "java") {
                ffi.annotation = "@FFI(JNI native)";
            } else {
                ffi.annotation = "@FFI(extern)";
            }
            boundaries.push_back(std::move(ffi));
        }
        return boundaries;
    }

    static std::string trim(const std::string& s) {
        size_t start = 0, end = s.size();
        while (start < end && std::isspace((unsigned char)s[start])) ++start;
        while (end > start && std::isspace((unsigned char)s[end - 1])) --end;
        return s.substr(start, end - start);
    }
};
