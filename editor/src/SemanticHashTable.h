#pragma once
// Step 1546: Semantic hash sidecar table persistence.

#include "ast/ASTNode.h"
#include "CompactAST.h"
#include "SemanticHash.h"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

using json = nlohmann::json;

struct SemanticHashTableSaveResult {
    bool success = false;
    std::string path;
    int entryCount = 0;
    std::string error;
};

struct SemanticHashTableLoadResult {
    bool success = false;
    std::string path;
    json table = json::object();
    std::string error;
};

inline std::string semanticHashTablePath(const std::string& workspaceRoot,
                                         const std::string& filePath) {
    namespace fs = std::filesystem;
    fs::path base = fs::path(workspaceRoot) / ".whetstone";
    return (base / (filePath + ".semantic_hash.json")).string();
}

inline bool shouldPersistSemanticHashEntry(const ASTNode* node, const json& semanticSummary) {
    if (!node) return false;
    if (!semanticSummary.empty()) return true;
    return node->conceptType == "Module" ||
           node->conceptType == "Function" ||
           node->conceptType == "MethodDeclaration" ||
           node->conceptType == "AsyncFunction" ||
           node->conceptType == "ClassDeclaration" ||
           node->conceptType == "InterfaceDeclaration" ||
           node->conceptType == "Variable" ||
           node->conceptType == "Parameter";
}

inline void collectSemanticHashEntries(ASTNode* node,
                                       json& entries,
                                       std::set<std::string>& primitiveCatalog,
                                       int& v2MismatchCount) {
    if (!node) return;

    const json sem = extractSemanticSummary(node);
    const SemanticHashDigests digests = computeSemanticHashDigests(node, sem);
    node->semanticHash = digests.h1;  // Active hash remains H1 during migration scaffold.

    if (shouldPersistSemanticHashEntry(node, sem)) {
        const std::vector<std::string> primitives = semanticPrimitivePaths(sem);
        for (const auto& p : primitives) primitiveCatalog.insert(p);
        if (digests.h1 != digests.h2) ++v2MismatchCount;
        entries.push_back({
            {"nodeId", node->id},
            {"concept", node->conceptType},
            {"irNodeKind", semanticIrNodeKindForConcept(node->conceptType)},
            {"name", getNodeName(node)},
            {"semanticHash", digests.h1},
            {"semanticHashH1", digests.h1},
            {"semanticHashH2", digests.h2},
            {"primitives", primitives},
            {"semantic", sem.is_object() ? sem : json::object()}
        });
    }

    for (auto* child : node->allChildren()) {
        collectSemanticHashEntries(child, entries, primitiveCatalog, v2MismatchCount);
    }
}

inline SemanticHashTableSaveResult saveSemanticHashTable(const std::string& workspaceRoot,
                                                         const std::string& filePath,
                                                         Module* ast) {
    SemanticHashTableSaveResult result;
    if (!ast) {
        result.error = "No AST to save";
        return result;
    }

    result.path = semanticHashTablePath(workspaceRoot, filePath);
    namespace fs = std::filesystem;
    fs::create_directories(fs::path(result.path).parent_path());

    json entries = json::array();
    std::set<std::string> primitiveCatalog;
    int v2MismatchCount = 0;
    collectSemanticHashEntries(ast, entries, primitiveCatalog, v2MismatchCount);
    result.entryCount = static_cast<int>(entries.size());

    json table = {
        {"version", "H1"},
        {"activeVersion", "H1"},
        {"dualEmitVersions", {"H1", "H2"}},
        {"hashEncoding", {
            {"algorithm", "fnv1a64"},
            {"digestByteOrder", "big-endian"},
            {"digestFormat", "hex16"},
            {"inputEncoding", "utf-8"}
        }},
        {"primitiveCatalogVersion", 1},
        {"file", filePath},
        {"migration", {
            {"h2ScaffoldEnabled", true},
            {"entriesCompared", static_cast<int>(entries.size())},
            {"h1h2MismatchCount", v2MismatchCount}
        }},
        {"primitiveCatalog", std::vector<std::string>(primitiveCatalog.begin(), primitiveCatalog.end())},
        {"entries", entries}
    };

    std::ofstream out(result.path);
    if (!out.is_open()) {
        result.error = "Cannot write to " + result.path;
        return result;
    }
    out << table.dump(2);
    out.close();

    result.success = true;
    return result;
}

inline SemanticHashTableLoadResult loadSemanticHashTable(const std::string& workspaceRoot,
                                                         const std::string& filePath) {
    SemanticHashTableLoadResult result;
    result.path = semanticHashTablePath(workspaceRoot, filePath);

    std::ifstream in(result.path);
    if (!in.is_open()) {
        result.error = "No semantic hash table: " + result.path;
        return result;
    }

    try {
        in >> result.table;
    } catch (const std::exception& e) {
        result.error = std::string("Failed to parse semantic hash table: ") + e.what();
        return result;
    }

    result.success = true;
    return result;
}

inline std::vector<std::string> listSemanticHashTables(const std::string& workspaceRoot) {
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    fs::path dir = fs::path(workspaceRoot) / ".whetstone";
    if (!fs::exists(dir)) return files;

    const std::string suffix = ".semantic_hash.json";
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        std::string full = entry.path().string();
        if (full.size() <= suffix.size()) continue;
        if (full.substr(full.size() - suffix.size()) != suffix) continue;

        std::error_code ec;
        fs::path rel = fs::relative(entry.path(), dir, ec);
        if (ec) {
            files.push_back(entry.path().filename().string());
        } else {
            std::string relStr = rel.generic_string();
            files.push_back(relStr.substr(0, relStr.size() - suffix.size()));
        }
    }

    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
    return files;
}
