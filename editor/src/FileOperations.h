#pragma once
// Step 247: File Operation Tools for MCP
//
// Standalone header implementing file operations for the headless path.
// No GUI dependencies. All operations respect the workspace root.

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include "FileTree.h"

namespace fs = std::filesystem;

// --- Path security ---
// Resolves relative paths against workspace root.
// Rejects paths that escape the workspace.
inline std::pair<bool, std::string> fileOpsResolvePath(
    const std::string& workspaceRoot, const std::string& path) {
    if (workspaceRoot.empty())
        return {false, "No workspace root configured"};

    fs::path wsRoot = fs::weakly_canonical(fs::path(workspaceRoot));
    fs::path resolved;

    if (fs::path(path).is_absolute()) {
        resolved = fs::weakly_canonical(fs::path(path));
    } else {
        resolved = fs::weakly_canonical(wsRoot / path);
    }

    std::string resolvedStr = resolved.string();
    std::string wsStr = wsRoot.string();
    if (resolvedStr.find(wsStr) != 0)
        return {false, "Path escapes workspace: " + path};

    return {true, resolvedStr};
}

// --- Read file ---
// Returns {success, content, lineCount}
inline std::tuple<bool, std::string, int> fileOpsRead(
    const std::string& resolvedPath, int startLine = 0, int endLine = 0) {
    std::ifstream f(resolvedPath);
    if (!f.is_open())
        return {false, "Cannot open file: " + resolvedPath, 0};

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line))
        lines.push_back(line);

    int totalLines = (int)lines.size();

    if (startLine <= 0 && endLine <= 0) {
        std::string content;
        for (size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) content += "\n";
            content += lines[i];
        }
        return {true, content, totalLines};
    }

    // Line range (1-based)
    int s = std::max(1, startLine);
    int e = endLine > 0 ? std::min(endLine, totalLines) : totalLines;

    std::string content;
    for (int i = s; i <= e; ++i) {
        if (i > s) content += "\n";
        content += lines[i - 1];
    }
    return {true, content, totalLines};
}

// --- Write file ---
// Returns {success, message, bytesWritten}
inline std::tuple<bool, std::string, size_t> fileOpsWrite(
    const std::string& resolvedPath, const std::string& content) {
    fs::path parent = fs::path(resolvedPath).parent_path();
    if (!parent.empty() && !fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if (ec)
            return {false, "Cannot create directories: " + ec.message(), 0};
    }

    std::ofstream f(resolvedPath, std::ios::binary);
    if (!f.is_open())
        return {false, "Cannot write file: " + resolvedPath, 0};

    f.write(content.data(), (std::streamsize)content.size());
    f.close();
    return {true, resolvedPath, content.size()};
}

// --- Create file ---
// Returns {success, message}
inline std::pair<bool, std::string> fileOpsCreate(
    const std::string& resolvedPath,
    const std::string& language = "",
    const std::string& templateName = "") {
    fs::path parent = fs::path(resolvedPath).parent_path();
    if (!parent.empty() && !fs::exists(parent)) {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if (ec)
            return {false, "Cannot create directories: " + ec.message()};
    }

    if (fs::exists(resolvedPath))
        return {false, "File already exists: " + resolvedPath};

    std::string content;
    std::string lang = language;
    if (lang.empty()) {
        std::string ext = fs::path(resolvedPath).extension().string();
        if (ext == ".py") lang = "python";
        else if (ext == ".cpp" || ext == ".h" || ext == ".hpp") lang = "cpp";
        else if (ext == ".rs") lang = "rust";
        else if (ext == ".go") lang = "go";
        else if (ext == ".java") lang = "java";
        else if (ext == ".js") lang = "javascript";
        else if (ext == ".ts") lang = "typescript";
    }

    if (!templateName.empty() && templateName != "empty") {
        if (lang == "python") {
            std::string modName = fs::path(resolvedPath).stem().string();
            content = "\"\"\"" + modName + " module.\"\"\"\n\n\n"
                      "if __name__ == \"__main__\":\n    pass\n";
        } else if (lang == "cpp") {
            std::string ext = fs::path(resolvedPath).extension().string();
            if (ext == ".h" || ext == ".hpp") {
                content = "#pragma once\n\n";
            } else {
                content = "#include <iostream>\n\nint main() {\n"
                          "    return 0;\n}\n";
            }
        } else if (lang == "rust") {
            content = "fn main() {\n    println!(\"Hello, world!\");\n}\n";
        } else if (lang == "go") {
            content = "package main\n\nfunc main() {\n}\n";
        } else if (lang == "java") {
            std::string cls = fs::path(resolvedPath).stem().string();
            content = "public class " + cls +
                " {\n    public static void main(String[] args) {\n    }\n}\n";
        } else if (lang == "javascript" || lang == "typescript") {
            content = "// " +
                fs::path(resolvedPath).filename().string() + "\n\n";
        }
    }

    std::ofstream f(resolvedPath, std::ios::binary);
    if (!f.is_open())
        return {false, "Cannot create file: " + resolvedPath};

    if (!content.empty())
        f.write(content.data(), (std::streamsize)content.size());
    f.close();
    return {true, resolvedPath};
}

// --- Diff ---
// Simple line-by-line unified diff between string content and a file on disk.
// Returns {diffText, linesAdded, linesRemoved}
inline std::tuple<std::string, int, int> fileOpsDiff(
    const std::string& currentContent, const std::string& diskPath) {
    std::ifstream f(diskPath);
    std::string diskContent;
    if (f.is_open()) {
        std::ostringstream ss;
        ss << f.rdbuf();
        diskContent = ss.str();
    }

    auto splitLines = [](const std::string& s) -> std::vector<std::string> {
        std::vector<std::string> lines;
        std::istringstream iss(s);
        std::string line;
        while (std::getline(iss, line))
            lines.push_back(line);
        return lines;
    };

    auto diskLines = splitLines(diskContent);
    auto curLines = splitLines(currentContent);

    int added = 0, removed = 0;
    std::ostringstream diff;
    diff << "--- " << diskPath << "\n";
    diff << "+++ (buffer)\n";

    size_t di = 0, ci = 0;
    while (di < diskLines.size() || ci < curLines.size()) {
        if (di < diskLines.size() && ci < curLines.size() &&
            diskLines[di] == curLines[ci]) {
            diff << " " << diskLines[di] << "\n";
            ++di; ++ci;
        } else {
            bool found = false;
            for (size_t look = 1; look < 5 && !found; ++look) {
                if (ci + look < curLines.size() &&
                    di < diskLines.size() &&
                    curLines[ci + look] == diskLines[di]) {
                    for (size_t j = 0; j < look; ++j) {
                        diff << "+" << curLines[ci + j] << "\n";
                        ++added;
                    }
                    ci += look;
                    found = true;
                }
                if (!found && di + look < diskLines.size() &&
                    ci < curLines.size() &&
                    diskLines[di + look] == curLines[ci]) {
                    for (size_t j = 0; j < look; ++j) {
                        diff << "-" << diskLines[di + j] << "\n";
                        ++removed;
                    }
                    di += look;
                    found = true;
                }
            }
            if (!found) {
                if (di < diskLines.size()) {
                    diff << "-" << diskLines[di] << "\n";
                    ++removed; ++di;
                }
                if (ci < curLines.size()) {
                    diff << "+" << curLines[ci] << "\n";
                    ++added; ++ci;
                }
            }
        }
    }

    return {diff.str(), added, removed};
}

// --- List workspace files ---
struct FileOpsEntry {
    std::string path;
    size_t size = 0;
    bool isDir = false;
};

inline std::vector<FileOpsEntry> fileOpsListWorkspace(
    const std::string& workspaceRoot,
    const std::string& globPattern = "*") {
    std::vector<FileOpsEntry> results;
    if (workspaceRoot.empty() || !fs::exists(workspaceRoot))
        return results;

    FileTree tree;
    FileNode root = tree.build(workspaceRoot);

    auto matchGlob = [](const std::string& name,
                         const std::string& pattern) -> bool {
        if (pattern == "*") return true;
        if (pattern.empty()) return true;
        if (pattern.size() > 1 && pattern[0] == '*') {
            std::string suffix = pattern.substr(1);
            return name.size() >= suffix.size() &&
                name.compare(name.size() - suffix.size(),
                             suffix.size(), suffix) == 0;
        }
        return name == pattern;
    };

    std::function<void(const FileNode&)> collect =
        [&](const FileNode& node) {
            if (node.path == workspaceRoot) {
                for (const auto& child : node.children)
                    collect(child);
                return;
            }
            std::string relPath = fs::relative(
                fs::path(node.path), fs::path(workspaceRoot)).string();
            if (matchGlob(node.name, globPattern)) {
                FileOpsEntry entry;
                entry.path = relPath;
                entry.isDir = node.isDir;
                if (!node.isDir && fs::exists(node.path)) {
                    std::error_code ec;
                    entry.size = (size_t)fs::file_size(node.path, ec);
                }
                results.push_back(entry);
            }
            if (node.isDir) {
                for (const auto& child : node.children)
                    collect(child);
            }
        };

    collect(root);
    return results;
}
