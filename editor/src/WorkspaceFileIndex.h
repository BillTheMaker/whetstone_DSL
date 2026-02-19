#pragma once
// Step 674: Workspace file symbol index for context assembly.

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

struct SymbolLocation {
    std::string file;
    int lineStart = 0;
    int lineEnd = 0;
    std::string symbolName;
    std::string symbolKind;
};

class WorkspaceFileIndex {
public:
    static WorkspaceFileIndex build(const std::string& workspaceRoot) {
        WorkspaceFileIndex out;
        namespace fs = std::filesystem;

        std::error_code ec;
        fs::path rootPath = fs::path(workspaceRoot);
        if (workspaceRoot.empty() || !fs::exists(rootPath, ec) || !fs::is_directory(rootPath, ec)) {
            return out;
        }

        for (fs::recursive_directory_iterator it(rootPath, ec), end; !ec && it != end; it.increment(ec)) {
            if (ec) break;
            if (!it->is_regular_file(ec)) continue;

            const fs::path absolutePath = it->path();
            const fs::path relativePath = absolutePath.lexically_relative(rootPath);
            const std::string rel = normalizePath(relativePath.generic_string());

            std::ifstream in(absolutePath);
            if (!in.good()) continue;

            out.fileCount_++;
            int lineNumber = 0;
            std::string line;
            while (std::getline(in, line)) {
                ++lineNumber;
                SymbolLocation loc;
                if (!extractSymbol(rel, line, lineNumber, &loc)) continue;
                out.symbols_.push_back(loc);
                out.byName_[loc.symbolName].push_back(loc);
                out.byFile_[loc.file].push_back(loc);
            }
        }

        return out;
    }

    std::vector<SymbolLocation> find(const std::string& symbolName) const {
        auto it = byName_.find(symbolName);
        if (it == byName_.end()) return {};
        return it->second;
    }

    std::vector<SymbolLocation> findInFile(const std::string& filePath) const {
        const std::string key = normalizePath(filePath);
        auto it = byFile_.find(key);
        if (it == byFile_.end()) return {};
        return it->second;
    }

    int fileCount() const { return fileCount_; }
    int symbolCount() const { return static_cast<int>(symbols_.size()); }

private:
    static std::string trim(const std::string& s) {
        size_t a = 0;
        while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
        size_t b = s.size();
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
        return s.substr(a, b - a);
    }

    static std::string normalizePath(const std::string& path) {
        std::string out = path;
        for (char& c : out) {
            if (c == '\\') c = '/';
        }
        return out;
    }

    static bool isIdentifierChar(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    static std::string extractIdentifierBefore(const std::string& s, size_t posExclusive) {
        if (posExclusive == 0 || posExclusive > s.size()) return "";
        size_t i = posExclusive;
        while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) --i;
        size_t end = i;
        while (i > 0 && isIdentifierChar(s[i - 1])) --i;
        if (end <= i) return "";
        return s.substr(i, end - i);
    }

    static bool isLikelyControl(const std::string& name) {
        return name == "if" || name == "for" || name == "while" || name == "switch" || name == "catch";
    }

    static std::string extractTypedefName(const std::string& line) {
        size_t semi = line.find(';');
        if (semi == std::string::npos) return "";
        return extractIdentifierBefore(line, semi);
    }

    static std::string extractConstName(const std::string& line) {
        size_t cut = line.find('=');
        if (cut == std::string::npos) cut = line.find(';');
        if (cut == std::string::npos) cut = line.find('{');
        if (cut == std::string::npos) cut = line.size();
        return extractIdentifierBefore(line, cut);
    }

    static bool extractSymbol(const std::string& file,
                              const std::string& rawLine,
                              int lineNumber,
                              SymbolLocation* out) {
        if (!out) return false;
        const std::string line = trim(rawLine);
        if (line.empty()) return false;
        if (line.rfind("//", 0) == 0) return false;

        static const std::regex kClassRe("^class\\s+([A-Za-z_][A-Za-z0-9_]*)\\b");
        static const std::regex kStructRe("^struct\\s+([A-Za-z_][A-Za-z0-9_]*)\\b");
        static const std::regex kUsingRe("^using\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*=");
        static const std::regex kTypedefRe("^typedef\\s+");

        std::smatch m;
        if (std::regex_search(line, m, kClassRe)) {
            *out = {file, lineNumber, lineNumber, m[1].str(), "class"};
            return true;
        }
        if (std::regex_search(line, m, kStructRe)) {
            *out = {file, lineNumber, lineNumber, m[1].str(), "struct"};
            return true;
        }
        if (std::regex_search(line, m, kUsingRe)) {
            *out = {file, lineNumber, lineNumber, m[1].str(), "type"};
            return true;
        }
        if (std::regex_search(line, m, kTypedefRe)) {
            const std::string name = extractTypedefName(line);
            if (!name.empty()) {
                *out = {file, lineNumber, lineNumber, name, "type"};
                return true;
            }
        }

        if ((line.rfind("const ", 0) == 0 || line.rfind("constexpr ", 0) == 0) &&
            line.find('(') == std::string::npos) {
            const std::string name = extractConstName(line);
            if (!name.empty()) {
                *out = {file, lineNumber, lineNumber, name, "const"};
                return true;
            }
        }

        const size_t openParen = line.find('(');
        const size_t closeParen = line.find(')');
        if (openParen != std::string::npos && closeParen != std::string::npos && closeParen > openParen) {
            const std::string name = extractIdentifierBefore(line, openParen);
            if (!name.empty() && !isLikelyControl(name)) {
                *out = {file, lineNumber, lineNumber, name, "function"};
                return true;
            }
        }

        return false;
    }

    std::vector<SymbolLocation> symbols_;
    std::unordered_map<std::string, std::vector<SymbolLocation>> byName_;
    std::unordered_map<std::string, std::vector<SymbolLocation>> byFile_;
    int fileCount_ = 0;
};

