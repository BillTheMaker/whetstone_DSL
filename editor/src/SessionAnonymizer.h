#pragma once
// Step 231: Session anonymizer

#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class SessionAnonymizer {
public:
    enum class Level {
        Light,
        Medium,
        Full
    };

    void setLevel(Level level) { level_ = level; }
    void setSeed(uint64_t seed) { seed_ = seed; }

    json anonymize(const json& input) {
        reset();
        return anonymizeValue(input, "");
    }

private:
    Level level_ = Level::Medium;
    uint64_t seed_ = 0xC0FFEEULL;
    std::unordered_map<std::string, std::string> pathMap_;
    std::unordered_map<std::string, std::string> identMap_;
    std::unordered_map<std::string, std::string> genericMap_;

    void reset() {
        pathMap_.clear();
        identMap_.clear();
        genericMap_.clear();
    }

    static bool isIdentifierStart(char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    }

    static bool isIdentifierChar(char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    }

    static std::unordered_set<std::string> keywordSet() {
        return {
            "if", "else", "for", "while", "return", "class", "struct", "def", "fn",
            "let", "var", "const", "int", "float", "double", "bool", "string",
            "true", "false", "null", "none", "self", "this", "public", "private",
            "protected", "static", "void", "new", "delete", "import", "from", "as"
        };
    }

    static bool isLikelyPath(const std::string& s) {
        if (s.find("\\\\") != std::string::npos) return true;
        if (s.find("/") != std::string::npos) {
            if (s.find(".") != std::string::npos) return true;
        }
        if (s.size() > 2 && std::isalpha(static_cast<unsigned char>(s[0])) && s[1] == ':') return true;
        return false;
    }

    static bool keyLooksSensitive(const std::string& key) {
        std::string lower;
        lower.reserve(key.size());
        for (char c : key) lower.push_back((char)std::tolower((unsigned char)c));
        return lower.find("token") != std::string::npos ||
               lower.find("apikey") != std::string::npos ||
               lower.find("api_key") != std::string::npos ||
               lower.find("secret") != std::string::npos ||
               lower.find("password") != std::string::npos ||
               lower.find("key") != std::string::npos;
    }

    static bool keyLooksLikePath(const std::string& key) {
        std::string lower;
        lower.reserve(key.size());
        for (char c : key) lower.push_back((char)std::tolower((unsigned char)c));
        return lower.find("path") != std::string::npos || lower.find("file") != std::string::npos;
    }

    static bool keyIsCodePayload(const std::string& key) {
        return key == "source" || key == "content" || key == "spec" || key == "code";
    }

    static bool preserveValueForKey(const std::string& key) {
        return key == "method" || key == "jsonrpc" || key == "type" ||
               key == "annotationType" || key == "strategy" ||
               key == "sourceLanguage" || key == "targetLanguage" ||
               key == "language" || key == "role";
    }

    static std::string fileExtension(const std::string& path) {
        auto pos = path.find_last_of('.');
        if (pos == std::string::npos) return "";
        auto slash = path.find_last_of("/\\");
        if (slash != std::string::npos && pos < slash) return "";
        return path.substr(pos);
    }

    uint64_t fnv1a(const std::string& s) const {
        uint64_t hash = 1469598103934665603ULL ^ seed_;
        for (unsigned char c : s) {
            hash ^= c;
            hash *= 1099511628211ULL;
        }
        return hash;
    }

    std::string stableToken(const std::string& prefix, const std::string& value) {
        uint64_t h = fnv1a(value);
        return prefix + std::to_string(static_cast<unsigned long long>(h % 1000003ULL));
    }

    std::string anonymizePath(const std::string& path) {
        auto it = pathMap_.find(path);
        if (it != pathMap_.end()) return it->second;
        std::string ext = fileExtension(path);
        std::string token = stableToken("file_", path);
        std::string mapped = "/project/src/" + token + ext;
        pathMap_[path] = mapped;
        return mapped;
    }

    std::string anonymizeIdentifier(const std::string& ident) {
        auto it = identMap_.find(ident);
        if (it != identMap_.end()) return it->second;
        std::string mapped = stableToken("sym_", ident);
        identMap_[ident] = mapped;
        return mapped;
    }

    std::string anonymizeGeneric(const std::string& value) {
        auto it = genericMap_.find(value);
        if (it != genericMap_.end()) return it->second;
        std::string mapped = stableToken("val_", value);
        genericMap_[value] = mapped;
        return mapped;
    }

    std::string stripCommentsAndStrings(const std::string& code) {
        std::string out;
        out.reserve(code.size());
        bool inString = false;
        char stringDelim = 0;
        bool inLineComment = false;
        bool inBlockComment = false;
        for (size_t i = 0; i < code.size(); ++i) {
            char c = code[i];
            char next = (i + 1 < code.size()) ? code[i + 1] : '\0';

            if (inLineComment) {
                if (c == '\n') {
                    inLineComment = false;
                    out.push_back(c);
                }
                continue;
            }
            if (inBlockComment) {
                if (c == '*' && next == '/') {
                    inBlockComment = false;
                    ++i;
                }
                if (c == '\n') out.push_back(c);
                continue;
            }
            if (inString) {
                if (c == '\\' && next != '\0') {
                    ++i;
                    continue;
                }
                if (c == stringDelim) {
                    inString = false;
                    out.push_back(c);
                }
                continue;
            }

            if (c == '/' && next == '/') {
                inLineComment = true;
                ++i;
                continue;
            }
            if (c == '/' && next == '*') {
                inBlockComment = true;
                ++i;
                continue;
            }
            if (c == '#') {
                inLineComment = true;
                continue;
            }
            if (c == '\"' || c == '\'' || c == '`') {
                inString = true;
                stringDelim = c;
                out.push_back(c);
                out.append("STR");
                continue;
            }
            out.push_back(c);
        }
        return out;
    }

    std::string anonymizeIdentifiersInCode(const std::string& code) {
        std::string out;
        out.reserve(code.size());
        auto keywords = keywordSet();
        for (size_t i = 0; i < code.size();) {
            char c = code[i];
            if (isIdentifierStart(c)) {
                size_t start = i;
                ++i;
                while (i < code.size() && isIdentifierChar(code[i])) ++i;
                std::string token = code.substr(start, i - start);
                if (keywords.count(token)) {
                    out.append(token);
                } else {
                    out.append(anonymizeIdentifier(token));
                }
                continue;
            }
            out.push_back(c);
            ++i;
        }
        return out;
    }

    std::string anonymizeStringValue(const std::string& value, const std::string& key) {
        if (key == "method" || key == "jsonrpc") {
            return value;
        }
        if (!key.empty() && keyLooksLikePath(key)) {
            return anonymizePath(value);
        }
        if (!key.empty() && keyLooksSensitive(key)) {
            return "<redacted>";
        }
        if (isLikelyPath(value)) {
            return anonymizePath(value);
        }
        if (keyIsCodePayload(key)) {
            std::string stripped = stripCommentsAndStrings(value);
            if (level_ != Level::Light) {
                stripped = anonymizeIdentifiersInCode(stripped);
            }
            return stripped;
        }
        if (level_ == Level::Light) {
            return value;
        }
        if (level_ >= Level::Medium && isIdentifierStart(value[0])) {
            bool allIdent = true;
            for (char c : value) {
                if (!isIdentifierChar(c)) {
                    allIdent = false;
                    break;
                }
            }
            if (allIdent && !keywordSet().count(value)) {
                return anonymizeIdentifier(value);
            }
        }
        if (level_ == Level::Full && !preserveValueForKey(key)) {
            return anonymizeGeneric(value);
        }
        return value;
    }

    json anonymizeValue(const json& value, const std::string& key) {
        if (value.is_object()) {
            json out = json::object();
            for (auto it = value.begin(); it != value.end(); ++it) {
                out[it.key()] = anonymizeValue(it.value(), it.key());
            }
            return out;
        }
        if (value.is_array()) {
            json out = json::array();
            for (const auto& item : value) {
                out.push_back(anonymizeValue(item, key));
            }
            return out;
        }
        if (value.is_string()) {
            const std::string s = value.get<std::string>();
            if (s.empty()) return value;
            return anonymizeStringValue(s, key);
        }
        return value;
    }
};
