#pragma once

// Step 468: Operator Overloading + Friend
// Focused node + parser helpers for C++ operator overload declarations.

#include "ASTNode.h"

#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

class OperatorOverload : public ASTNode {
public:
    std::string operatorSymbol;        // <, <<, ==, +, ...
    std::vector<std::string> parameters;
    std::string returnType;
    bool isFriend = false;
    bool isConst = false;

    OperatorOverload() { conceptType = "OperatorOverload"; }
};

class CppOperators {
public:
    static bool parseOperatorOverload(const std::string& decl, OperatorOverload& out) {
        // e.g. bool operator<(const Foo&) const;
        // e.g. friend ostream& operator<<(ostream&, const Foo&);
        std::regex rx(R"(^\s*(friend\s+)?(.+?)\s+operator\s*([^\s(]+)\s*\(([^)]*)\)\s*(const)?\s*;?\s*$)");
        std::smatch m;
        if (!std::regex_match(decl, m, rx)) return false;

        out.isFriend = m[1].matched;
        out.returnType = trim(m[2].str());
        out.operatorSymbol = trim(m[3].str());
        out.parameters = parseParamList(m[4].str());
        out.isConst = m[5].matched;
        return !out.operatorSymbol.empty();
    }

    static json toJson(const OperatorOverload& o) {
        return {
            {"concept", o.conceptType},
            {"operatorSymbol", o.operatorSymbol},
            {"parameters", o.parameters},
            {"returnType", o.returnType},
            {"isFriend", o.isFriend},
            {"isConst", o.isConst}
        };
    }

    static OperatorOverload fromJson(const json& j) {
        OperatorOverload o;
        o.operatorSymbol = j.value("operatorSymbol", "");
        o.parameters = j.value("parameters", std::vector<std::string>{});
        o.returnType = j.value("returnType", "");
        o.isFriend = j.value("isFriend", false);
        o.isConst = j.value("isConst", false);
        return o;
    }

    static std::string generateCppDeclaration(const OperatorOverload& o) {
        std::ostringstream out;
        if (o.isFriend) out << "friend ";
        out << o.returnType << " operator" << o.operatorSymbol << "(";
        for (size_t i = 0; i < o.parameters.size(); ++i) {
            out << o.parameters[i];
            if (i + 1 < o.parameters.size()) out << ", ";
        }
        out << ")";
        if (o.isConst) out << " const";
        out << ";";
        return out.str();
    }

    static std::string projectToJavaMethod(const OperatorOverload& o) {
        if (o.operatorSymbol == "<") return "int compareTo(Foo other)";
        if (o.operatorSymbol == "==") return "boolean equals(Object other)";
        if (o.operatorSymbol == "<<") return "String toDisplayString()";
        return "Object operator" + sanitize(o.operatorSymbol) + "(Object other)";
    }

    static std::string projectToPythonMethod(const OperatorOverload& o) {
        if (o.operatorSymbol == "<") return "def __lt__(self, other):";
        if (o.operatorSymbol == "==") return "def __eq__(self, other):";
        if (o.operatorSymbol == "<<") return "def __str__(self):";
        return "def __op_" + sanitize(o.operatorSymbol) + "(self, other):";
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::vector<std::string> parseParamList(const std::string& params) {
        std::vector<std::string> out;
        std::string cur;
        int depth = 0;
        for (char c : params) {
            if (c == '<') ++depth;
            if (c == '>') --depth;
            if (c == ',' && depth == 0) {
                auto t = trim(cur);
                if (!t.empty()) out.push_back(t);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        auto t = trim(cur);
        if (!t.empty()) out.push_back(t);
        return out;
    }

    static std::string sanitize(const std::string& op) {
        std::string out;
        for (char c : op) {
            if (std::isalnum(static_cast<unsigned char>(c))) out.push_back(c);
            else out.push_back('_');
        }
        return out.empty() ? "unknown" : out;
    }
};
