#pragma once
// Step 400: Self-Hosting Test Harness
//
// Infrastructure to feed actual Whetstone .h files through the pipeline.
// Compare parsed AST against expected structure. Track coverage.
// Graceful degradation: unparseable sections become opaque blocks, not errors.

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Parser.h"
#include "ast/ClassDeclaration.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/CppAdvancedNodes.h"

// --- Coverage tracking ---

struct ConstructCoverage {
    std::string constructType;  // e.g. "class", "function", "struct", "enum"
    int expected = 0;
    int parsed = 0;
    int opaque = 0;  // unparseable sections that became opaque blocks

    double ratio() const {
        return expected > 0 ? static_cast<double>(parsed) / expected : 1.0;
    }
};

struct SelfHostResult {
    std::string filePath;
    bool parseSuccess = false;
    std::unique_ptr<Module> ast;
    std::vector<ConstructCoverage> coverage;
    std::vector<std::string> warnings;
    int totalExpected = 0;
    int totalParsed = 0;
    int totalOpaque = 0;

    double overallCoverage() const {
        return totalExpected > 0 ? static_cast<double>(totalParsed) / totalExpected : 1.0;
    }
};

// --- Expected structure declaration ---

struct ExpectedConstruct {
    std::string type;  // "class", "function", "struct", "namespace", "type_alias"
    std::string name;  // expected name (empty = don't check)
};

struct ExpectedStructure {
    std::string filePath;
    std::vector<ExpectedConstruct> constructs;
};

// --- Self-Hosting Harness ---

class SelfHostHarness {
public:

    // Read a file from disk and return its contents
    static std::string readFile(const std::string& path) {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return "";
        std::ostringstream ss;
        ss << ifs.rdbuf();
        return ss.str();
    }

    // Parse a Whetstone .h file through the C++ parser
    static SelfHostResult parseFile(const std::string& path) {
        SelfHostResult result;
        result.filePath = path;

        std::string source = readFile(path);
        if (source.empty()) {
            result.warnings.push_back("File not found or empty: " + path);
            return result;
        }

        result.ast = TreeSitterParser::parseCpp(source);
        result.parseSuccess = (result.ast != nullptr);

        if (!result.parseSuccess) {
            result.warnings.push_back("Parse returned null for: " + path);
        }

        return result;
    }

    // Parse source string directly (for testing without file I/O)
    static SelfHostResult parseSource(const std::string& source,
                                       const std::string& label = "<inline>") {
        SelfHostResult result;
        result.filePath = label;

        if (source.empty()) {
            result.warnings.push_back("Empty source provided");
            return result;
        }

        result.ast = TreeSitterParser::parseCpp(source);
        result.parseSuccess = (result.ast != nullptr);
        return result;
    }

    // Compare parsed AST against expected structure
    static void checkExpected(SelfHostResult& result,
                               const ExpectedStructure& expected) {
        if (!result.ast) {
            result.totalExpected = static_cast<int>(expected.constructs.size());
            result.totalOpaque = result.totalExpected;
            return;
        }

        // Group expected constructs by type
        std::map<std::string, std::vector<std::string>> byType;
        for (const auto& c : expected.constructs) {
            byType[c.type].push_back(c.name);
        }

        result.totalExpected = static_cast<int>(expected.constructs.size());

        for (const auto& [type, names] : byType) {
            ConstructCoverage cov;
            cov.constructType = type;
            cov.expected = static_cast<int>(names.size());

            if (type == "class" || type == "struct") {
                auto& classes = result.ast->getChildren("classes");
                for (const auto& name : names) {
                    bool found = false;
                    for (auto* c : classes) {
                        auto* cls = static_cast<ClassDeclaration*>(c);
                        if (name.empty() || cls->name == name) {
                            found = true;
                            break;
                        }
                    }
                    if (found) cov.parsed++;
                    else cov.opaque++;
                }
            }
            else if (type == "function") {
                auto& fns = result.ast->getChildren("functions");
                for (const auto& name : names) {
                    bool found = false;
                    for (auto* f : fns) {
                        auto* fn = static_cast<Function*>(f);
                        if (name.empty() || fn->name == name) {
                            found = true;
                            break;
                        }
                    }
                    if (found) cov.parsed++;
                    else cov.opaque++;
                }
            }
            else if (type == "namespace") {
                auto& stmts = result.ast->getChildren("statements");
                for (const auto& name : names) {
                    bool found = false;
                    for (auto* s : stmts) {
                        if (s->conceptType == "NamespaceDeclaration") {
                            auto* ns = static_cast<NamespaceDeclaration*>(s);
                            if (name.empty() || ns->name == name) {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) cov.parsed++;
                    else cov.opaque++;
                }
            }
            else if (type == "type_alias") {
                auto& stmts = result.ast->getChildren("statements");
                for (const auto& name : names) {
                    bool found = false;
                    for (auto* s : stmts) {
                        if (s->conceptType == "TypeAlias") {
                            auto* ta = static_cast<TypeAlias*>(s);
                            if (name.empty() || ta->aliasName == name) {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) cov.parsed++;
                    else cov.opaque++;
                }
            }
            else if (type == "enum") {
                auto& stmts = result.ast->getChildren("statements");
                for (const auto& name : names) {
                    bool found = false;
                    for (auto* s : stmts) {
                        if (s->conceptType == "EnumDeclaration") {
                            auto* en = static_cast<EnumDeclaration*>(s);
                            if (name.empty() || en->name == name) {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (found) cov.parsed++;
                    else cov.opaque++;
                }
            }
            else {
                // Unknown type — mark all as opaque
                cov.opaque = cov.expected;
                result.warnings.push_back("Unknown construct type: " + type);
            }

            result.totalParsed += cov.parsed;
            result.totalOpaque += cov.opaque;
            result.coverage.push_back(std::move(cov));
        }
    }

    // Count all classes found in parsed AST
    static int countClasses(const Module* mod) {
        if (!mod) return 0;
        return static_cast<int>(mod->getChildren("classes").size());
    }

    // Count all functions found in parsed AST
    static int countFunctions(const Module* mod) {
        if (!mod) return 0;
        return static_cast<int>(mod->getChildren("functions").size());
    }

    // Count all statements of a specific concept type
    static int countStatements(const Module* mod, const std::string& conceptType) {
        if (!mod) return 0;
        int count = 0;
        for (auto* s : mod->getChildren("statements")) {
            if (s->conceptType == conceptType) count++;
        }
        return count;
    }

    // Get list of all class names
    static std::vector<std::string> getClassNames(const Module* mod) {
        std::vector<std::string> names;
        if (!mod) return names;
        for (auto* c : mod->getChildren("classes")) {
            names.push_back(static_cast<ClassDeclaration*>(c)->name);
        }
        return names;
    }

    // Get list of all function names
    static std::vector<std::string> getFunctionNames(const Module* mod) {
        std::vector<std::string> names;
        if (!mod) return names;
        for (auto* f : mod->getChildren("functions")) {
            names.push_back(static_cast<Function*>(f)->name);
        }
        return names;
    }

    // Check if a specific construct was parsed (by type and name)
    static bool hasParsedConstruct(const Module* mod,
                                    const std::string& type,
                                    const std::string& name) {
        if (!mod) return false;
        if (type == "class" || type == "struct") {
            for (auto* c : mod->getChildren("classes")) {
                if (static_cast<ClassDeclaration*>(c)->name == name) return true;
            }
        } else if (type == "function") {
            for (auto* f : mod->getChildren("functions")) {
                if (static_cast<Function*>(f)->name == name) return true;
            }
        }
        return false;
    }

    // Generate a coverage report string
    static std::string coverageReport(const SelfHostResult& result) {
        std::ostringstream out;
        out << "Self-Host Report: " << result.filePath << "\n";
        out << "  Parse success: " << (result.parseSuccess ? "yes" : "no") << "\n";
        out << "  Overall coverage: " << (result.overallCoverage() * 100) << "%"
            << " (" << result.totalParsed << "/" << result.totalExpected << ")\n";

        for (const auto& cov : result.coverage) {
            out << "  " << cov.constructType << ": "
                << cov.parsed << "/" << cov.expected
                << " (" << (cov.ratio() * 100) << "%)";
            if (cov.opaque > 0) out << " [" << cov.opaque << " opaque]";
            out << "\n";
        }

        for (const auto& w : result.warnings) {
            out << "  WARNING: " << w << "\n";
        }
        return out.str();
    }
};
