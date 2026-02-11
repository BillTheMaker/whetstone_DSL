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
#include <filesystem>
#include "FileTree.h"

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
//  ProjectState — aggregates workspace index and open buffer tracking
// -----------------------------------------------------------------------
struct ProjectState {
    WorkspaceIndex index;

    void scanWorkspace(const std::string& workspaceRoot) {
        index.scan(workspaceRoot);
    }
};
