#pragma once
// Step 85: Filesystem tree with basic .gitignore support

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

struct FileNode {
    std::string name;
    std::string path;
    bool isDir = false;
    std::vector<FileNode> children;
};

class FileTree {
public:
    FileNode build(const std::string& rootPath) {
        root_ = std::filesystem::path(rootPath);
        ignorePatterns_ = loadGitIgnore(root_ / ".gitignore");
        return buildNode(root_);
    }

private:
    std::filesystem::path root_;
    std::vector<std::string> ignorePatterns_;

    FileNode buildNode(const std::filesystem::path& p) {
        FileNode node;
        node.name = p.filename().string();
        node.path = p.string();
        node.isDir = std::filesystem::is_directory(p);
        if (node.isDir) {
            for (auto& entry : std::filesystem::directory_iterator(p)) {
                const auto childPath = entry.path();
                if (shouldIgnore(childPath)) continue;
                node.children.push_back(buildNode(childPath));
            }
            std::sort(node.children.begin(), node.children.end(),
                [](const FileNode& a, const FileNode& b) {
                    if (a.isDir != b.isDir) return a.isDir > b.isDir;
                    return a.name < b.name;
                });
        }
        return node;
    }

    std::vector<std::string> loadGitIgnore(const std::filesystem::path& p) {
        std::vector<std::string> patterns;
        std::ifstream f(p.string());
        if (!f.is_open()) return patterns;
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            patterns.push_back(line);
        }
        return patterns;
    }

    bool shouldIgnore(const std::filesystem::path& p) const {
        std::string rel = std::filesystem::relative(p, root_).generic_string();
        std::string name = p.filename().string();
        bool isDir = std::filesystem::is_directory(p);

        for (const auto& pat : ignorePatterns_) {
            if (pat.empty()) continue;
            if (pat.back() == '/' && isDir) {
                std::string dirName = pat.substr(0, pat.size() - 1);
                if (name == dirName) return true;
                if (rel.rfind(dirName + "/", 0) == 0) return true;
            } else if (pat.front() == '*') {
                std::string suffix = pat.substr(1);
                if (suffix.size() <= name.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
                    return true;
            } else if (pat.front() == '/') {
                std::string exact = pat.substr(1);
                if (rel == exact) return true;
            } else {
                if (name == pat) return true;
            }
        }
        return false;
    }
};
