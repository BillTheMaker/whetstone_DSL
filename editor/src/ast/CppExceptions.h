#pragma once

// Step 467: Exception Handling
// Models and parses C++ try/catch/throw/noexcept features with simple
// cross-language projection helpers.

#include "ASTNode.h"

#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

struct CatchClause {
    std::string exceptionType; // e.g. "const std::exception&"
    std::string variableName;  // e.g. "e"
    std::string bodyCode;
    bool catchAll = false;     // catch (...)
};

class TryCatchStatement : public ASTNode {
public:
    std::string tryBody;
    std::vector<CatchClause> catches;
    std::string finallyBody; // optional

    TryCatchStatement() { conceptType = "TryCatchStatement"; }
};

class ThrowExpression : public ASTNode {
public:
    std::string expression;

    ThrowExpression() { conceptType = "ThrowExpression"; }
};

class CppExceptions {
public:
    static bool parseTryCatch(const std::string& source, TryCatchStatement& out) {
        std::regex tryRx(R"(try\s*\{([\s\S]*?)\})");
        std::smatch tm;
        if (!std::regex_search(source, tm, tryRx)) return false;
        out.tryBody = trim(tm[1].str());

        std::regex catchRx(R"(catch\s*\(\s*([^)]+)\s*\)\s*\{([\s\S]*?)\})");
        out.catches.clear();
        for (auto it = std::sregex_iterator(source.begin(), source.end(), catchRx);
             it != std::sregex_iterator(); ++it) {
            CatchClause c;
            auto m = *it;
            std::string sig = trim(m[1].str());
            c.bodyCode = trim(m[2].str());
            if (sig == "...") {
                c.catchAll = true;
                c.exceptionType = "...";
            } else {
                c.catchAll = false;
                auto parts = splitLastToken(sig);
                c.exceptionType = parts.first;
                c.variableName = parts.second;
            }
            out.catches.push_back(c);
        }
        return !out.catches.empty();
    }

    static bool parseThrowExpression(const std::string& source, ThrowExpression& out) {
        std::regex rx(R"(throw\s+([^;]+);)");
        std::smatch m;
        if (!std::regex_search(source, m, rx)) return false;
        out.expression = trim(m[1].str());
        return !out.expression.empty();
    }

    static bool hasNoexcept(const std::string& functionSignature) {
        return functionSignature.find("noexcept") != std::string::npos;
    }

    static json toJson(const TryCatchStatement& t) {
        json catches = json::array();
        for (const auto& c : t.catches) {
            catches.push_back({
                {"exceptionType", c.exceptionType},
                {"variableName", c.variableName},
                {"bodyCode", c.bodyCode},
                {"catchAll", c.catchAll}
            });
        }
        return {
            {"concept", t.conceptType},
            {"tryBody", t.tryBody},
            {"catches", catches},
            {"finallyBody", t.finallyBody}
        };
    }

    static TryCatchStatement tryCatchFromJson(const json& j) {
        TryCatchStatement t;
        t.tryBody = j.value("tryBody", "");
        t.finallyBody = j.value("finallyBody", "");
        if (j.contains("catches")) {
            for (const auto& cj : j["catches"]) {
                CatchClause c;
                c.exceptionType = cj.value("exceptionType", "");
                c.variableName = cj.value("variableName", "");
                c.bodyCode = cj.value("bodyCode", "");
                c.catchAll = cj.value("catchAll", false);
                t.catches.push_back(c);
            }
        }
        return t;
    }

    static json toJson(const ThrowExpression& t) {
        return {
            {"concept", t.conceptType},
            {"expression", t.expression}
        };
    }

    static ThrowExpression throwFromJson(const json& j) {
        ThrowExpression t;
        t.expression = j.value("expression", "");
        return t;
    }

    static std::string projectTryCatchToRustResult(const TryCatchStatement& t) {
        (void)t;
        return "match do_work() {\n  Ok(v) => v,\n  Err(e) => { /* handle */ }\n}";
    }

    static std::string projectTryCatchToPython(const TryCatchStatement& t) {
        std::ostringstream out;
        out << "try:\n    " << (t.tryBody.empty() ? "pass" : t.tryBody) << "\n";
        for (const auto& c : t.catches) {
            if (c.catchAll) out << "except Exception:\n    " << (c.bodyCode.empty() ? "pass" : c.bodyCode) << "\n";
            else out << "except " << sanitizeTypeForPython(c.exceptionType) << " as "
                     << (c.variableName.empty() ? "e" : c.variableName)
                     << ":\n    " << (c.bodyCode.empty() ? "pass" : c.bodyCode) << "\n";
        }
        return out.str();
    }

    static std::string projectTryCatchToGo(const TryCatchStatement& t) {
        (void)t;
        return "if err != nil {\n    return err\n}";
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::pair<std::string, std::string> splitLastToken(const std::string& sig) {
        auto t = trim(sig);
        size_t sp = t.find_last_of(" \t");
        if (sp == std::string::npos) return {t, ""};
        return {trim(t.substr(0, sp)), trim(t.substr(sp + 1))};
    }

    static std::string sanitizeTypeForPython(std::string t) {
        // Collapse C++ qualifiers for simple projection.
        t = trim(t);
        std::string out;
        for (char c : t) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') out.push_back(c);
            else if (c == ':' && !out.empty() && out.back() != '_') out.push_back('_');
        }
        if (out.empty()) return "Exception";
        return out;
    }
};
