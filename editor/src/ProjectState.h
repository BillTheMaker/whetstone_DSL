#pragma once
// Step 258: Project Model and Workspace Indexing
//
// Tracks workspace files, detects languages, and provides a project-level
// index of file paths (not content) for fast lookup. Works with
// HeadlessEditorState to manage multiple open buffers.

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <filesystem>
#include "FileTree.h"
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Import.h"
#include "CompactAST.h"  // getNodeName

namespace fs = std::filesystem;

// -----------------------------------------------------------------------
//  Language detection from file extension
// -----------------------------------------------------------------------
inline std::string detectLanguage(const std::string& filePath) {
    std::string ext = fs::path(filePath).extension().string();
    if (ext == ".py")                          return "python";
    if (ext == ".cpp" || ext == ".cc" ||
        ext == ".cxx" || ext == ".h" ||
        ext == ".hpp" || ext == ".hxx")        return "cpp";
    if (ext == ".rs")                          return "rust";
    if (ext == ".go")                          return "go";
    if (ext == ".java")                        return "java";
    if (ext == ".js" || ext == ".mjs")         return "javascript";
    if (ext == ".ts" || ext == ".tsx")         return "typescript";
    if (ext == ".el" || ext == ".elisp")       return "elisp";
    if (ext == ".org")                         return "org";
    return "";
}

// -----------------------------------------------------------------------
//  WorkspaceIndex — file path index (not content) for fast lookup
// -----------------------------------------------------------------------
struct IndexedFile {
    std::string path;        // absolute path
    std::string relativePath; // relative to workspace root
    std::string language;    // detected from extension
    bool        isDir = false;
};

class WorkspaceIndex {
public:
    void scan(const std::string& workspaceRoot) {
        files_.clear();
        root_ = workspaceRoot;
        if (root_.empty() || !fs::exists(root_)) return;

        FileTree tree;
        FileNode rootNode = tree.build(root_);
        collectFiles(rootNode);
    }

    const std::vector<IndexedFile>& files() const { return files_; }

    // Find files matching a glob-like pattern (extension filter)
    std::vector<IndexedFile> findByExtension(
            const std::string& ext) const {
        std::vector<IndexedFile> result;
        for (const auto& f : files_) {
            if (!f.isDir &&
                fs::path(f.path).extension().string() == ext)
                result.push_back(f);
        }
        return result;
    }

    // Find files by language
    std::vector<IndexedFile> findByLanguage(
            const std::string& lang) const {
        std::vector<IndexedFile> result;
        for (const auto& f : files_) {
            if (!f.isDir && f.language == lang)
                result.push_back(f);
        }
        return result;
    }

    // Lookup by relative path
    const IndexedFile* findByRelativePath(
            const std::string& relPath) const {
        auto it = byRelPath_.find(relPath);
        if (it != byRelPath_.end()) return &files_[it->second];
        return nullptr;
    }

    int fileCount() const {
        int count = 0;
        for (const auto& f : files_)
            if (!f.isDir) ++count;
        return count;
    }

    int dirCount() const {
        int count = 0;
        for (const auto& f : files_)
            if (f.isDir) ++count;
        return count;
    }

    const std::string& root() const { return root_; }

private:
    std::string root_;
    std::vector<IndexedFile> files_;
    std::map<std::string, size_t> byRelPath_;

    void collectFiles(const FileNode& node) {
        if (node.path == root_) {
            for (const auto& child : node.children)
                collectFiles(child);
            return;
        }

        IndexedFile entry;
        entry.path = node.path;
        entry.isDir = node.isDir;
        std::error_code ec;
        entry.relativePath = fs::relative(
            fs::path(node.path), fs::path(root_), ec).string();
        if (!node.isDir)
            entry.language = detectLanguage(node.path);

        size_t idx = files_.size();
        files_.push_back(entry);
        byRelPath_[entry.relativePath] = idx;

        if (node.isDir) {
            for (const auto& child : node.children)
                collectFiles(child);
        }
    }
};

// -----------------------------------------------------------------------
//  Step 259: ImportGraph — tracks which files import which
// -----------------------------------------------------------------------
class ImportGraph {
public:
    // Record that `fromFile` imports `moduleName`
    void addEdge(const std::string& fromFile,
                 const std::string& moduleName) {
        edges_[fromFile].insert(moduleName);
    }

    // Clear edges for a file (call before re-scanning)
    void clearFile(const std::string& filePath) {
        edges_.erase(filePath);
    }

    // Get modules imported by a file
    std::set<std::string> importsOf(const std::string& filePath) const {
        auto it = edges_.find(filePath);
        if (it != edges_.end()) return it->second;
        return {};
    }

    // Get files that import a given module name
    std::vector<std::string> importedBy(
            const std::string& moduleName) const {
        std::vector<std::string> result;
        for (const auto& [file, imports] : edges_) {
            if (imports.count(moduleName)) result.push_back(file);
        }
        return result;
    }

    // Rebuild import edges from an AST's Import nodes
    void updateFromAST(const std::string& filePath, Module* ast) {
        clearFile(filePath);
        if (!ast) return;
        for (auto* child : ast->allChildren()) {
            if (child->conceptType == "Import") {
                auto* imp = static_cast<const Import*>(child);
                if (!imp->moduleName.empty())
                    addEdge(filePath, imp->moduleName);
            }
        }
    }

    // Rebuild import edges from source text (fallback for parsers
    // that don't generate Import AST nodes, e.g. Python)
    void updateFromSource(const std::string& filePath,
                          const std::string& source) {
        // Simple line-by-line scan for import statements
        std::istringstream iss(source);
        std::string line;
        while (std::getline(iss, line)) {
            // Python: "import foo" or "from foo import bar"
            if (line.substr(0, 7) == "import ") {
                std::string mod = line.substr(7);
                // Trim whitespace
                while (!mod.empty() && mod.back() == ' ')
                    mod.pop_back();
                // Handle "import foo, bar"
                size_t comma = mod.find(',');
                if (comma != std::string::npos)
                    mod = mod.substr(0, comma);
                if (!mod.empty()) addEdge(filePath, mod);
            } else if (line.substr(0, 5) == "from ") {
                size_t space = line.find(' ', 5);
                if (space != std::string::npos) {
                    std::string mod = line.substr(5, space - 5);
                    if (!mod.empty()) addEdge(filePath, mod);
                }
            }
            // JS/TS: "import ... from 'foo'" handled by AST
            // C++: "#include" handled by AST
        }
    }

    const std::map<std::string, std::set<std::string>>& edges() const {
        return edges_;
    }

private:
    // filePath -> set of imported module names
    std::map<std::string, std::set<std::string>> edges_;
};

// -----------------------------------------------------------------------
//  ExportedSymbol — a symbol visible across files
// -----------------------------------------------------------------------
struct ExportedSymbol {
    std::string name;
    std::string kind;    // "function", "variable", "class"
    std::string nodeId;
    std::string filePath; // source file
};

// Collect exported (module-level) symbols from an AST
inline std::vector<ExportedSymbol> collectExportedSymbols(
        Module* ast, const std::string& filePath) {
    std::vector<ExportedSymbol> symbols;
    if (!ast) return symbols;
    for (auto* child : ast->getChildren("functions")) {
        if (child->conceptType == "Function") {
            auto* fn = static_cast<const Function*>(child);
            symbols.push_back({fn->name, "function", fn->id, filePath});
        }
    }
    for (auto* child : ast->getChildren("variables")) {
        if (child->conceptType == "Variable") {
            auto* v = static_cast<const Variable*>(child);
            symbols.push_back({v->name, "variable", v->id, filePath});
        }
    }
    for (auto* child : ast->getChildren("classes")) {
        if (child->conceptType == "Class") {
            symbols.push_back({getNodeName(child), "class",
                               child->id, filePath});
        }
    }
    return symbols;
}

// -----------------------------------------------------------------------
//  ProjectState — aggregates workspace index and open buffer tracking
// -----------------------------------------------------------------------
struct ProjectState {
    WorkspaceIndex index;
    ImportGraph    importGraph;

    void scanWorkspace(const std::string& workspaceRoot) {
        index.scan(workspaceRoot);
    }
};
