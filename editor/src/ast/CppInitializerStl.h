#pragma once

// Step 469: Initializer Lists + STL Patterns
// Focused helpers for parsing initializer-list expressions and recognizing
// common STL/iterator loop patterns with lightweight annotations.

#include "ASTNode.h"

#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using json = nlohmann::json;

class InitializerListExpression : public ASTNode {
public:
    std::vector<std::string> elements;

    InitializerListExpression() { conceptType = "InitializerListExpression"; }
};

struct StlContainerInfo {
    std::string rawType;       // std::vector<int>
    std::string containerKind; // vector/map/set/string/unknown
    std::string elementType;   // int, pair<...>, ...
    std::vector<std::string> annotations;
};

struct IteratorPatternInfo {
    bool isIteratorLoop = false;
    std::vector<std::string> annotations; // includes @Loop(...) when detected
};

class CppInitializerStl {
public:
    static bool parseInitializerList(const std::string& source, InitializerListExpression& out) {
        std::regex rx(R"(\{([^}]*)\})");
        std::smatch m;
        if (!std::regex_search(source, m, rx)) return false;
        out.elements = splitCsv(m[1].str());
        return true;
    }

    static StlContainerInfo recognizeContainer(const std::string& declaration) {
        StlContainerInfo info;
        std::regex rx(R"((std::[A-Za-z_]\w*(?:\s*<[^>]+>)?)\s+[A-Za-z_]\w*)");
        std::smatch m;
        if (!std::regex_search(declaration, m, rx)) {
            info.containerKind = "unknown";
            info.rawType = "";
            return info;
        }

        info.rawType = trim(m[1].str());
        auto lowered = toLower(info.rawType);
        if (lowered.find("std::vector") == 0) info.containerKind = "vector";
        else if (lowered.find("std::map") == 0) info.containerKind = "map";
        else if (lowered.find("std::set") == 0) info.containerKind = "set";
        else if (lowered.find("std::string") == 0) info.containerKind = "string";
        else info.containerKind = "unknown";

        info.elementType = extractTemplateArg(info.rawType);
        info.annotations.push_back("@Container(" + info.containerKind + ")");
        if (!info.elementType.empty()) info.annotations.push_back("@ElementType(" + info.elementType + ")");
        return info;
    }

    static IteratorPatternInfo detectIteratorPattern(const std::string& source) {
        IteratorPatternInfo info;
        std::string s = toLower(source);
        bool hasBeginEnd = s.find(".begin()") != std::string::npos && s.find(".end()") != std::string::npos;
        bool hasIteratorDecl = s.find("::iterator") != std::string::npos || s.find("auto it") != std::string::npos;
        bool hasIncrement = s.find("++it") != std::string::npos || s.find("it++") != std::string::npos;
        if (hasBeginEnd && hasIteratorDecl && hasIncrement) {
            info.isIteratorLoop = true;
            info.annotations.push_back("@Loop(iterator)");
        }
        return info;
    }

    static json toJson(const InitializerListExpression& e) {
        return {
            {"concept", e.conceptType},
            {"elements", e.elements}
        };
    }

    static InitializerListExpression fromJson(const json& j) {
        InitializerListExpression e;
        e.elements = j.value("elements", std::vector<std::string>{});
        return e;
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::string toLower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static std::vector<std::string> splitCsv(const std::string& in) {
        std::vector<std::string> out;
        std::string cur;
        int depth = 0;
        for (char c : in) {
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

    static std::string extractTemplateArg(const std::string& type) {
        size_t lt = type.find('<');
        size_t gt = type.rfind('>');
        if (lt == std::string::npos || gt == std::string::npos || gt <= lt) return "";
        return trim(type.substr(lt + 1, gt - lt - 1));
    }
};
