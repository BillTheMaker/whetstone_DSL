#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>

class StubParser {
public:
    static std::vector<std::string> parseFile(const std::string& path,
                                              const std::string& languageHint) {
        std::filesystem::path p(path);
        std::string ext = p.extension().string();
        if (ext == ".pyi") return parsePythonStub(path);
        if (ext == ".d.ts") return parseTypeScriptStub(path);
        if (ext == ".h" || ext == ".hpp" || ext == ".hh") return parseHeaderStub(path);
        if (p.filename() == "lib.rs") return parseRustStub(path);
        if (languageHint == "python") return parsePythonStub(path);
        if (languageHint == "javascript" || languageHint == "typescript") return parseTypeScriptStub(path);
        if (languageHint == "rust") return parseRustStub(path);
        return parseHeaderStub(path);
    }

    static std::vector<std::string> scanWorkspaceForLibrary(const std::string& workspaceRoot,
                                                            const std::string& library,
                                                            const std::string& languageHint) {
        std::vector<std::string> out;
        if (workspaceRoot.empty() || library.empty()) return out;
        std::filesystem::path root(workspaceRoot);
        if (!std::filesystem::exists(root)) return out;

        const int maxFiles = 8;
        int scanned = 0;
        std::string lowerLib = toLower(library);

        for (auto it = std::filesystem::recursive_directory_iterator(root);
             it != std::filesystem::recursive_directory_iterator(); ++it) {
            if (scanned >= maxFiles) break;
            if (!it->is_regular_file()) continue;
            const auto& path = it->path();
            std::string filename = path.filename().string();
            std::string ext = path.extension().string();
            if (!isStubCandidate(filename, ext, languageHint)) continue;
            std::string pathLower = toLower(path.string());
            if (pathLower.find(lowerLib) == std::string::npos) continue;
            auto parsed = parseFile(path.string(), languageHint);
            if (!parsed.empty()) {
                out.insert(out.end(), parsed.begin(), parsed.end());
                scanned++;
            }
        }
        dedupe(out);
        return out;
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos || end == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return out;
    }

    static bool isIdentChar(char c) {
        return std::isalnum((unsigned char)c) || c == '_' || c == ':';
    }

    static void dedupe(std::vector<std::string>& items) {
        std::unordered_set<std::string> seen;
        std::vector<std::string> out;
        for (const auto& item : items) {
            if (item.empty()) continue;
            if (seen.insert(item).second) out.push_back(item);
        }
        items.swap(out);
    }

    static bool isStubCandidate(const std::string& filename,
                                const std::string& ext,
                                const std::string& languageHint) {
        if (ext == ".pyi") return true;
        if (ext == ".d.ts") return true;
        if (ext == ".h" || ext == ".hpp" || ext == ".hh") return true;
        if (filename == "lib.rs") return true;
        if (languageHint == "python" && ext == ".pyi") return true;
        if ((languageHint == "javascript" || languageHint == "typescript") && ext == ".d.ts") return true;
        if (languageHint == "rust" && filename == "lib.rs") return true;
        if (languageHint == "cpp" && (ext == ".h" || ext == ".hpp" || ext == ".hh")) return true;
        return false;
    }

    static std::vector<std::string> parsePythonStub(const std::string& path) {
        std::vector<std::string> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;
            if (line.rfind("def ", 0) == 0) {
                std::string name = extractAfterKeyword(line, "def");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("class ", 0) == 0) {
                std::string name = extractAfterKeyword(line, "class");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("CONST", 0) == 0 || line.find(":") != std::string::npos) {
                std::string name = extractBefore(line, ':');
                if (!name.empty()) out.push_back(name);
            }
        }
        dedupe(out);
        return out;
    }

    static std::vector<std::string> parseTypeScriptStub(const std::string& path) {
        std::vector<std::string> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line.rfind("//", 0) == 0) continue;
            if (line.rfind("export function", 0) == 0 || line.rfind("function", 0) == 0) {
                std::string name = extractAfterKeyword(line, "function");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("export class", 0) == 0 || line.rfind("class", 0) == 0) {
                std::string name = extractAfterKeyword(line, "class");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("interface", 0) == 0 || line.rfind("export interface", 0) == 0) {
                std::string name = extractAfterKeyword(line, "interface");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("const ", 0) == 0 || line.rfind("export const", 0) == 0) {
                std::string name = extractAfterKeyword(line, "const");
                if (!name.empty()) out.push_back(name);
            }
        }
        dedupe(out);
        return out;
    }

    static std::vector<std::string> parseHeaderStub(const std::string& path) {
        std::vector<std::string> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            line = stripCppComment(line);
            line = trim(line);
            if (line.empty()) continue;
            auto pos = line.find('(');
            if (pos == std::string::npos) continue;
            std::string left = trim(line.substr(0, pos));
            std::string name = lastIdentifier(left);
            if (!name.empty() && name != "if" && name != "for" && name != "while") {
                out.push_back(name);
            }
        }
        dedupe(out);
        return out;
    }

    static std::vector<std::string> parseRustStub(const std::string& path) {
        std::vector<std::string> out;
        std::ifstream in(path);
        if (!in.is_open()) return out;
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line.rfind("//", 0) == 0) continue;
            if (line.rfind("pub fn", 0) == 0) {
                std::string name = extractAfterKeyword(line, "fn");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("pub struct", 0) == 0) {
                std::string name = extractAfterKeyword(line, "struct");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("pub enum", 0) == 0) {
                std::string name = extractAfterKeyword(line, "enum");
                if (!name.empty()) out.push_back(name);
            } else if (line.rfind("pub trait", 0) == 0) {
                std::string name = extractAfterKeyword(line, "trait");
                if (!name.empty()) out.push_back(name);
            }
        }
        dedupe(out);
        return out;
    }

    static std::string extractAfterKeyword(const std::string& line, const std::string& keyword) {
        auto pos = line.find(keyword);
        if (pos == std::string::npos) return "";
        pos += keyword.size();
        while (pos < line.size() && std::isspace((unsigned char)line[pos])) pos++;
        std::string name;
        while (pos < line.size() && isIdentChar(line[pos])) {
            name.push_back(line[pos]);
            pos++;
        }
        return name;
    }

    static std::string extractBefore(const std::string& line, char delim) {
        auto pos = line.find(delim);
        if (pos == std::string::npos) return "";
        return trim(line.substr(0, pos));
    }

    static std::string stripCppComment(const std::string& line) {
        auto pos = line.find("//");
        if (pos == std::string::npos) return line;
        return line.substr(0, pos);
    }

    static std::string lastIdentifier(const std::string& text) {
        int i = (int)text.size() - 1;
        while (i >= 0 && !isIdentChar(text[i])) --i;
        if (i < 0) return "";
        int end = i;
        while (i >= 0 && isIdentChar(text[i])) --i;
        return text.substr(i + 1, end - i);
    }
};
