#pragma once

// Step 466: Range-Based For + Structured Bindings
// Focused parsing/serialization/generation helpers for remaining C++ gaps.

#include "ASTNode.h"

#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

class RangeForStatement : public ASTNode {
public:
    std::string iteratorDecl;   // e.g. "const auto& x"
    std::string iteratorName;   // e.g. "x"
    std::string containerExpr;  // e.g. "items"
    std::string bodyCode;       // lightweight body capture

    RangeForStatement() { conceptType = "RangeForStatement"; }
};

class StructuredBinding : public ASTNode {
public:
    std::vector<std::string> names; // e.g. key, value
    std::string initializerExpr;    // e.g. pair

    StructuredBinding() { conceptType = "StructuredBinding"; }
};

class CppRangeStructured {
public:
    static bool parseRangeFor(const std::string& source, RangeForStatement& out) {
        std::regex rx(R"(for\s*\(\s*([^:]+)\s*:\s*([^)]+)\)\s*\{?([^}]*)\}?)");
        std::smatch m;
        if (!std::regex_search(source, m, rx)) return false;
        out.iteratorDecl = trim(m[1].str());
        out.iteratorName = inferIteratorName(out.iteratorDecl);
        out.containerExpr = trim(m[2].str());
        out.bodyCode = trim(m[3].str());
        return true;
    }

    static bool parseStructuredBinding(const std::string& source, StructuredBinding& out) {
        std::regex rx(R"(auto\s*\[\s*([^\]]+)\s*\]\s*=\s*([^;]+);?)");
        std::smatch m;
        if (!std::regex_search(source, m, rx)) return false;
        auto names = split(trim(m[1].str()), ',');
        out.names.clear();
        for (auto& n : names) {
            auto t = trim(n);
            if (!t.empty()) out.names.push_back(t);
        }
        out.initializerExpr = trim(m[2].str());
        return !out.names.empty();
    }

    static json toJson(const RangeForStatement& r) {
        return {
            {"concept", r.conceptType},
            {"iteratorDecl", r.iteratorDecl},
            {"iteratorName", r.iteratorName},
            {"containerExpr", r.containerExpr},
            {"bodyCode", r.bodyCode}
        };
    }

    static RangeForStatement rangeForFromJson(const json& j) {
        RangeForStatement r;
        r.iteratorDecl = j.value("iteratorDecl", "");
        r.iteratorName = j.value("iteratorName", "");
        r.containerExpr = j.value("containerExpr", "");
        r.bodyCode = j.value("bodyCode", "");
        return r;
    }

    static json toJson(const StructuredBinding& b) {
        return {
            {"concept", b.conceptType},
            {"names", b.names},
            {"initializerExpr", b.initializerExpr}
        };
    }

    static StructuredBinding structuredBindingFromJson(const json& j) {
        StructuredBinding b;
        b.names = j.value("names", std::vector<std::string>{});
        b.initializerExpr = j.value("initializerExpr", "");
        return b;
    }

    static std::string generateCpp(const RangeForStatement& r) {
        std::ostringstream out;
        out << "for (" << r.iteratorDecl << " : " << r.containerExpr << ") {\n";
        if (!r.bodyCode.empty()) out << "  " << r.bodyCode << "\n";
        out << "}";
        return out.str();
    }

    static std::string generateCpp(const StructuredBinding& b) {
        std::ostringstream out;
        out << "auto [";
        for (size_t i = 0; i < b.names.size(); ++i) {
            out << b.names[i];
            if (i + 1 < b.names.size()) out << ", ";
        }
        out << "] = " << b.initializerExpr << ";";
        return out.str();
    }

    static std::string projectRangeForToPython(const RangeForStatement& r) {
        return "for " + r.iteratorName + " in " + r.containerExpr + ":\n    pass";
    }

    static std::string projectRangeForToRust(const RangeForStatement& r) {
        return "for " + r.iteratorName + " in " + r.containerExpr + " {\n    // ...\n}";
    }

    static std::string projectRangeForToJava(const RangeForStatement& r) {
        return "for (var " + r.iteratorName + " : " + r.containerExpr + ") {\n    // ...\n}";
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> out;
        std::string cur;
        std::istringstream in(s);
        while (std::getline(in, cur, delim)) out.push_back(cur);
        return out;
    }

    static std::string inferIteratorName(const std::string& iteratorDecl) {
        auto t = trim(iteratorDecl);
        size_t sp = t.find_last_of(" \t");
        if (sp == std::string::npos) return t;
        std::string name = trim(t.substr(sp + 1));
        while (!name.empty() && (name[0] == '*' || name[0] == '&')) name.erase(name.begin());
        return name;
    }
};
