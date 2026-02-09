#pragma once
// Step 113: Project-wide search

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <algorithm>
#include <cctype>
#include "FileTree.h"

class ProjectSearch {
public:
    struct Match {
        int line = 0;
        int col = 0;
        std::string lineText;
    };

    struct FileResult {
        std::string path;
        std::vector<Match> matches;
    };

    void setRoot(const std::string& root) { root_ = root; }

    std::vector<FileResult> search(const std::string& query,
                                   const std::string& includeGlobs,
                                   const std::string& excludeGlobs,
                                   bool useRegex) {
        std::vector<FileResult> results;
        if (root_.empty() || query.empty()) return results;

        auto include = splitPatterns(includeGlobs);
        auto exclude = splitPatterns(excludeGlobs);

        FileTree tree;
        FileNode rootNode = tree.build(root_);
        std::vector<std::string> files;
        collectFiles(rootNode, files);

        std::regex rx;
        bool regexOk = false;
        if (useRegex) {
            try {
                rx = std::regex(query, std::regex_constants::ECMAScript);
                regexOk = true;
            } catch (...) {
                regexOk = false;
            }
        }

        for (const auto& path : files) {
            std::string rel = std::filesystem::relative(path, root_).generic_string();
            if (!matchesInclude(rel, include)) continue;
            if (matchesExclude(rel, exclude)) continue;

            std::ifstream in(path, std::ios::binary);
            if (!in.is_open()) continue;

            std::string line;
            int lineNum = 0;
            FileResult fileRes;
            fileRes.path = path;
            while (std::getline(in, line)) {
                if (regexOk) {
                    for (auto it = std::sregex_iterator(line.begin(), line.end(), rx);
                         it != std::sregex_iterator(); ++it) {
                        Match m;
                        m.line = lineNum;
                        m.col = (int)it->position();
                        m.lineText = line;
                        fileRes.matches.push_back(std::move(m));
                    }
                } else {
                    size_t pos = line.find(query);
                    while (pos != std::string::npos) {
                        Match m;
                        m.line = lineNum;
                        m.col = (int)pos;
                        m.lineText = line;
                        fileRes.matches.push_back(std::move(m));
                        pos = line.find(query, pos + 1);
                    }
                }
                ++lineNum;
            }
            if (!fileRes.matches.empty()) results.push_back(std::move(fileRes));
        }

        return results;
    }

private:
    std::string root_;

    static void collectFiles(const FileNode& node, std::vector<std::string>& out) {
        if (!node.isDir) {
            out.push_back(node.path);
            return;
        }
        for (const auto& child : node.children) {
            collectFiles(child, out);
        }
    }

    static std::vector<std::string> splitPatterns(const std::string& input) {
        std::vector<std::string> out;
        std::string cur;
        for (char c : input) {
            if (c == ',' || c == ';') {
                if (!cur.empty()) out.push_back(trim(cur));
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) out.push_back(trim(cur));
        return out;
    }

    static std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace((unsigned char)s[start])) ++start;
        size_t end = s.size();
        while (end > start && std::isspace((unsigned char)s[end - 1])) --end;
        return s.substr(start, end - start);
    }

    static bool matchGlob(const std::string& text, const std::string& pattern) {
        size_t t = 0, p = 0, star = std::string::npos, match = 0;
        while (t < text.size()) {
            if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
                ++t; ++p; continue;
            }
            if (p < pattern.size() && pattern[p] == '*') {
                star = p++;
                match = t;
                continue;
            }
            if (star != std::string::npos) {
                p = star + 1;
                t = ++match;
                continue;
            }
            return false;
        }
        while (p < pattern.size() && pattern[p] == '*') ++p;
        return p == pattern.size();
    }

    static bool matchesInclude(const std::string& path, const std::vector<std::string>& patterns) {
        if (patterns.empty()) return true;
        for (const auto& pat : patterns) {
            if (pat.empty()) continue;
            if (matchGlob(path, pat)) return true;
        }
        return false;
    }

    static bool matchesExclude(const std::string& path, const std::vector<std::string>& patterns) {
        for (const auto& pat : patterns) {
            if (pat.empty()) continue;
            if (matchGlob(path, pat)) return true;
        }
        return false;
    }
};
