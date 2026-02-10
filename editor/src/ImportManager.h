#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

struct ImportIssue {
    int line = 0;
    std::string symbol;
    std::string message;
};

struct ImportEditResult {
    std::string text;
    bool changed = false;
};

static inline std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    if (!text.empty() && text.back() == '\n') lines.push_back("");
    return lines;
}

static inline std::string joinLines(const std::vector<std::string>& lines) {
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i) {
        out += lines[i];
        if (i + 1 < lines.size()) out += "\n";
    }
    return out;
}

static inline std::string trimStr(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static inline bool startsWith(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

static inline bool isIdentChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_' || c == '-';
}

static inline bool containsWord(const std::string& text, const std::string& word) {
    if (word.empty()) return false;
    size_t pos = 0;
    while ((pos = text.find(word, pos)) != std::string::npos) {
        bool leftOk = (pos == 0) || !isIdentChar(text[pos - 1]);
        bool rightOk = (pos + word.size() >= text.size()) ||
                       !isIdentChar(text[pos + word.size()]);
        if (leftOk && rightOk) return true;
        pos += word.size();
    }
    return false;
}

static inline std::string lastPathSegment(const std::string& path) {
    auto pos = path.find_last_of("/.");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

static inline ImportEditResult ensureImportPython(const std::string& text,
                                                  const std::string& library,
                                                  const std::string& symbol) {
    ImportEditResult out{ text, false };
    std::string line;
    if (startsWith(symbol, library + ".")) {
        line = "import " + library;
    } else {
        line = "from " + library + " import " + symbol;
    }

    auto lines = splitLines(text);
    std::vector<std::string> imports;
    std::vector<std::string> rest;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "import ") || startsWith(t, "from ")) {
            if (t != line) imports.push_back(l);
        } else {
            rest.push_back(l);
        }
    }
    imports.push_back(line);
    std::sort(imports.begin(), imports.end());
    out.text = joinLines(imports) + "\n" + joinLines(rest);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImportJs(const std::string& text,
                                              const std::string& library,
                                              const std::string& symbol) {
    ImportEditResult out{ text, false };
    std::string name = symbol;
    auto dot = name.find_last_of('.');
    if (dot != std::string::npos) name = name.substr(dot + 1);
    std::string line = "import { " + name + " } from '" + library + "';";

    auto lines = splitLines(text);
    std::vector<std::string> imports;
    std::vector<std::string> rest;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "import ")) {
            if (t != line) imports.push_back(l);
        } else {
            rest.push_back(l);
        }
    }
    imports.push_back(line);
    std::sort(imports.begin(), imports.end());
    out.text = joinLines(imports) + "\n" + joinLines(rest);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImportRust(const std::string& text,
                                                const std::string& library,
                                                const std::string& symbol) {
    ImportEditResult out{ text, false };
    std::string line;
    if (symbol.find("::") != std::string::npos) line = "use " + symbol + ";";
    else line = "use " + library + "::" + symbol + ";";

    auto lines = splitLines(text);
    std::vector<std::string> imports;
    std::vector<std::string> rest;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "use ")) {
            if (t != line) imports.push_back(l);
        } else {
            rest.push_back(l);
        }
    }
    imports.push_back(line);
    std::sort(imports.begin(), imports.end());
    out.text = joinLines(imports) + "\n" + joinLines(rest);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImportGo(const std::string& text,
                                              const std::string& library) {
    ImportEditResult out{ text, false };
    std::string path = "\"" + library + "\"";
    std::string line = "import " + path;
    auto lines = splitLines(text);
    int pkgLine = -1;
    int importStart = -1;
    int importEnd = -1;
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (startsWith(t, "package ")) pkgLine = i;
        if (startsWith(t, "import (")) { importStart = i; }
        if (importStart >= 0 && t == ")") { importEnd = i; break; }
        if (importStart < 0 && startsWith(t, "import \"")) {
            importStart = i;
            importEnd = i;
            break;
        }
    }

    if (importStart >= 0 && importEnd >= importStart && importStart != importEnd) {
        std::vector<std::string> imports;
        for (int i = importStart + 1; i < importEnd; ++i) {
            std::string t = trimStr(lines[i]);
            if (!t.empty()) imports.push_back(t);
        }
        if (std::find(imports.begin(), imports.end(), path) == imports.end()) {
            imports.push_back(path);
        }
        std::sort(imports.begin(), imports.end());
        std::vector<std::string> outLines;
        for (int i = 0; i <= importStart; ++i) outLines.push_back(lines[i]);
        for (const auto& imp : imports) outLines.push_back("    " + imp);
        for (int i = importEnd; i < (int)lines.size(); ++i) outLines.push_back(lines[i]);
        out.text = joinLines(outLines);
        out.changed = true;
        return out;
    }

    if (importStart >= 0 && importStart == importEnd) {
        std::string existing = trimStr(lines[importStart]);
        std::vector<std::string> imports = { existing };
        if (existing != line) imports.push_back(line);
        std::sort(imports.begin(), imports.end());
        std::vector<std::string> outLines;
        for (int i = 0; i < importStart; ++i) outLines.push_back(lines[i]);
        outLines.push_back("import (");
        for (const auto& imp : imports) {
            std::string t = trimStr(imp);
            if (startsWith(t, "import ")) t = trimStr(t.substr(6));
            outLines.push_back("    " + t);
        }
        outLines.push_back(")");
        for (int i = importStart + 1; i < (int)lines.size(); ++i) outLines.push_back(lines[i]);
        out.text = joinLines(outLines);
        out.changed = true;
        return out;
    }

    int insertAt = (pkgLine >= 0) ? pkgLine + 1 : 0;
    std::vector<std::string> outLines;
    for (int i = 0; i < insertAt; ++i) outLines.push_back(lines[i]);
    outLines.push_back("import (");
    outLines.push_back("    " + path);
    outLines.push_back(")");
    for (int i = insertAt; i < (int)lines.size(); ++i) outLines.push_back(lines[i]);
    out.text = joinLines(outLines);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImportElisp(const std::string& text,
                                                 const std::string& library) {
    ImportEditResult out{ text, false };
    std::string line = "(require '" + library + ")";
    auto lines = splitLines(text);
    for (const auto& l : lines) {
        if (trimStr(l) == line) return out;
    }
    std::vector<std::string> outLines;
    outLines.push_back(line);
    outLines.insert(outLines.end(), lines.begin(), lines.end());
    out.text = joinLines(outLines);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImportCpp(const std::string& text,
                                               const std::string& library) {
    ImportEditResult out{ text, false };
    std::string line = "#include <" + library + ">";
    auto lines = splitLines(text);
    std::vector<std::string> imports;
    std::vector<std::string> rest;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "#include")) {
            if (t != line) imports.push_back(l);
        } else {
            rest.push_back(l);
        }
    }
    imports.push_back(line);
    std::sort(imports.begin(), imports.end());
    out.text = joinLines(imports) + "\n" + joinLines(rest);
    out.changed = true;
    return out;
}

static inline ImportEditResult ensureImport(const std::string& text,
                                            const std::string& language,
                                            const std::string& library,
                                            const std::string& symbol) {
    if (library.empty()) return {text, false};
    ImportEditResult res;
    if (language == "python") res = ensureImportPython(text, library, symbol);
    else if (language == "javascript" || language == "typescript")
        res = ensureImportJs(text, library, symbol);
    else if (language == "rust") res = ensureImportRust(text, library, symbol);
    else if (language == "go") res = ensureImportGo(text, library);
    else if (language == "elisp") res = ensureImportElisp(text, library);
    else res = ensureImportCpp(text, library);
    if (res.text == text) res.changed = false;
    return res;
}

static inline std::vector<ImportIssue> findUnusedImportsPython(const std::string& text) {
    std::vector<ImportIssue> issues;
    auto lines = splitLines(text);
    std::string body;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "import ") || startsWith(t, "from ")) continue;
        body += l;
        body += "\n";
    }
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        std::string symbol;
        if (startsWith(t, "import ")) {
            symbol = trimStr(t.substr(7));
        } else if (startsWith(t, "from ")) {
            auto pos = t.find("import");
            if (pos != std::string::npos) symbol = trimStr(t.substr(pos + 6));
        }
        if (!symbol.empty() && symbol != "*") {
            if (!containsWord(body, symbol)) {
                issues.push_back({i, symbol, "Unused import: " + symbol});
            }
        }
    }
    return issues;
}

static inline std::vector<ImportIssue> findUnusedImportsJs(const std::string& text) {
    std::vector<ImportIssue> issues;
    auto lines = splitLines(text);
    std::string body;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "import ")) continue;
        body += l;
        body += "\n";
    }
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (!startsWith(t, "import ")) continue;
        auto brace = t.find('{');
        auto from = t.find(" from ");
        if (brace != std::string::npos && from != std::string::npos) {
            auto end = t.find('}', brace);
            if (end == std::string::npos) continue;
            std::string names = t.substr(brace + 1, end - brace - 1);
            std::stringstream ss(names);
            std::string item;
            while (std::getline(ss, item, ',')) {
                item = trimStr(item);
                if (!item.empty() && !containsWord(body, item)) {
                    issues.push_back({i, item, "Unused import: " + item});
                }
            }
        }
    }
    return issues;
}

static inline std::vector<ImportIssue> findUnusedImportsRust(const std::string& text) {
    std::vector<ImportIssue> issues;
    auto lines = splitLines(text);
    std::string body;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "use ")) continue;
        body += l;
        body += "\n";
    }
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (!startsWith(t, "use ")) continue;
        auto pos = t.find("::");
        if (pos == std::string::npos) continue;
        std::string symbol = t.substr(t.find_last_of(':') + 1);
        if (!symbol.empty() && symbol.back() == ';') symbol.pop_back();
        if (!symbol.empty() && !containsWord(body, symbol)) {
            issues.push_back({i, symbol, "Unused import: " + symbol});
        }
    }
    return issues;
}

static inline std::vector<ImportIssue> findUnusedImportsGo(const std::string& text) {
    std::vector<ImportIssue> issues;
    auto lines = splitLines(text);
    std::string body;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "import ")) continue;
        body += l;
        body += "\n";
    }
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (startsWith(t, "import \"")) {
            std::string path = t.substr(8);
            if (!path.empty() && path.back() == '"') path.pop_back();
            std::string symbol = lastPathSegment(path);
            if (!symbol.empty() && body.find(symbol + ".") == std::string::npos) {
                issues.push_back({i, symbol, "Unused import: " + symbol});
            }
        }
    }
    return issues;
}

static inline std::vector<ImportIssue> findUnusedImportsElisp(const std::string& text) {
    std::vector<ImportIssue> issues;
    auto lines = splitLines(text);
    std::string body;
    for (const auto& l : lines) {
        std::string t = trimStr(l);
        if (startsWith(t, "(require '")) continue;
        body += l;
        body += "\n";
    }
    for (int i = 0; i < (int)lines.size(); ++i) {
        std::string t = trimStr(lines[i]);
        if (startsWith(t, "(require '")) {
            auto start = t.find('\'');
            auto end = t.find(')', start);
            if (start == std::string::npos || end == std::string::npos) continue;
            std::string symbol = t.substr(start + 1, end - start - 1);
            if (!symbol.empty() && body.find(symbol) == std::string::npos) {
                issues.push_back({i, symbol, "Unused import: " + symbol});
            }
        }
    }
    return issues;
}

static inline std::vector<ImportIssue> findUnusedImports(const std::string& text,
                                                         const std::string& language) {
    if (language == "python") return findUnusedImportsPython(text);
    if (language == "javascript" || language == "typescript") return findUnusedImportsJs(text);
    if (language == "rust") return findUnusedImportsRust(text);
    if (language == "go") return findUnusedImportsGo(text);
    if (language == "elisp") return findUnusedImportsElisp(text);
    return {};
}
