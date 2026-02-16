#pragma once
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Type.h"
#include "ClassDeclaration.h"
#include "PreprocessorNodes.h"
#include "EnumNamespaceNodes.h"
#include "../ast/Parser.h"
#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

class CParser {
public:
    static std::unique_ptr<Module> parseC(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_c_module";
        module->targetLanguage = "c";

        parsePreprocessor(source, module.get());
        std::string cleaned = stripComments(source);
        parseTypedefStructs(cleaned, module.get());
        parseStructs(cleaned, module.get());
        parseEnums(cleaned, module.get());
        parseFunctions(cleaned, module.get());
        parseTopLevelVariables(cleaned, module.get());
        return module;
    }

    static ParseResult parseCWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseC(source);
        return result;
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.rfind(prefix, 0) == 0;
    }

    static std::string stripComments(const std::string& source) {
        std::string out;
        out.reserve(source.size());
        bool line = false, block = false, str = false, chr = false;
        for (size_t i = 0; i < source.size(); ++i) {
            char c = source[i];
            char n = (i + 1 < source.size()) ? source[i + 1] : '\0';
            if (line) {
                if (c == '\n') { line = false; out.push_back('\n'); } else out.push_back(' ');
                continue;
            }
            if (block) {
                if (c == '*' && n == '/') { block = false; out += "  "; ++i; }
                else out.push_back(c == '\n' ? '\n' : ' ');
                continue;
            }
            if (!str && !chr && c == '/' && n == '/') { line = true; out += "  "; ++i; continue; }
            if (!str && !chr && c == '/' && n == '*') { block = true; out += "  "; ++i; continue; }
            if (c == '"' && !chr && !(i > 0 && source[i - 1] == '\\')) str = !str;
            if (c == '\'' && !str && !(i > 0 && source[i - 1] == '\\')) chr = !chr;
            out.push_back(c);
        }
        return out;
    }

    static std::vector<std::string> splitTopLevel(const std::string& input, char sep) {
        std::vector<std::string> parts;
        std::string cur;
        int depth = 0;
        for (char c : input) {
            if (c == '(') depth++;
            if (c == ')') depth--;
            if (c == sep && depth == 0) { parts.push_back(trim(cur)); cur.clear(); }
            else cur.push_back(c);
        }
        if (!trim(cur).empty()) parts.push_back(trim(cur));
        return parts;
    }

    static void setType(ASTNode* owner, const std::string& role, const std::string& raw) {
        std::string t = trim(raw);
        if (t.empty()) return;
        static const std::vector<std::string> p = {
            "void","char","short","int","long","float","double","signed","unsigned",
            "size_t","ssize_t","bool","_Bool","const","static","extern"
        };
        bool primitive = false;
        for (const auto& k : p) {
            if (t == k || startsWith(t, k + " ") || startsWith(t, k + "*")) { primitive = true; break; }
        }
        if (primitive) owner->setChild(role, new PrimitiveType(IdGenerator::next("type"), t));
        else owner->setChild(role, new CustomType(IdGenerator::next("type"), t));
    }

    static void parsePreprocessor(const std::string& source, Module* module) {
        std::istringstream in(source);
        std::string line;
        std::regex inc(R"(^\s*#\s*include\s*([<"])([^>"]+)[>"]\s*$)");
        std::regex defFn(R"(^\s*#\s*define\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*(.*)$)");
        std::regex defObj(R"(^\s*#\s*define\s+([A-Za-z_]\w*)\s*(.*)$)");
        std::regex prag(R"(^\s*#\s*pragma\s+(.*)$)");
        std::smatch m;
        while (std::getline(in, line)) {
            std::string t = trim(line);
            if (std::regex_match(line, m, inc)) {
                auto* node = new IncludeDirective(IdGenerator::next("inc"), trim(m[2].str()), m[1].str() == "<");
                module->addChild("statements", node);
            } else if (std::regex_match(line, m, defFn)) {
                auto* node = new MacroDefinition(IdGenerator::next("mac"), m[1].str());
                node->isFunctionLike = true;
                node->body = trim(m[3].str());
                for (const auto& part : splitTopLevel(m[2].str(), ',')) if (!part.empty()) node->parameters.push_back(part);
                module->addChild("statements", node);
            } else if (std::regex_match(line, m, defObj)) {
                auto* node = new MacroDefinition(IdGenerator::next("mac"), m[1].str());
                node->isFunctionLike = false;
                node->body = trim(m[2].str());
                module->addChild("statements", node);
            } else if (std::regex_match(line, m, prag)) {
                auto* node = new PragmaDirective(IdGenerator::next("prag"), trim(m[1].str()));
                module->addChild("statements", node);
            } else if (startsWith(t, "#ifndef") || startsWith(t, "#ifdef") || startsWith(t, "#endif")) {
                auto* node = new PragmaDirective(IdGenerator::next("prag"), t.substr(1));
                module->addChild("statements", node);
            }
        }
    }

    static void parseStructBody(const std::string& body, ClassDeclaration* cls) {
        std::istringstream in(body);
        std::string line;
        std::regex field(R"((?:const\s+)?([A-Za-z_]\w*(?:\s+\w+)?(?:\s*\*+)?)\s+([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*;)");
        while (std::getline(in, line)) {
            for (auto it = std::sregex_iterator(line.begin(), line.end(), field); it != std::sregex_iterator(); ++it) {
                std::smatch m = *it;
                auto* v = new Variable(IdGenerator::next("var"), m[2].str());
                setType(v, "type", m[1].str());
                cls->addChild("fields", v);
            }
        }
    }

    static bool insideTypedefStruct(const std::string& src, size_t pos) {
        std::regex rx(R"(typedef\s+struct\s*([A-Za-z_]\w*)?\s*\{[\s\S]*?\}\s*([A-Za-z_]\w*)\s*;)");
        for (auto it = std::sregex_iterator(src.begin(), src.end(), rx); it != std::sregex_iterator(); ++it) {
            size_t p = static_cast<size_t>((*it).position());
            size_t len = static_cast<size_t>((*it).length());
            if (pos >= p && pos < p + len) return true;
        }
        return false;
    }

    static void parseTypedefStructs(const std::string& src, Module* module) {
        std::regex rx(R"(typedef\s+struct\s*([A-Za-z_]\w*)?\s*\{([\s\S]*?)\}\s*([A-Za-z_]\w*)\s*;)");
        for (auto it = std::sregex_iterator(src.begin(), src.end(), rx); it != std::sregex_iterator(); ++it) {
            std::smatch m = *it;
            std::string sName = trim(m[1].str());
            std::string alias = trim(m[3].str());
            std::string className = sName.empty() ? alias : sName;
            auto* cls = new ClassDeclaration(IdGenerator::next("cls"), className);
            parseStructBody(m[2].str(), cls);
            module->addChild("classes", cls);
            auto* ta = new TypeAlias(IdGenerator::next("typedef"), alias,
                                     sName.empty() ? ("struct " + alias) : ("struct " + sName), false);
            module->addChild("statements", ta);
        }
    }

    static void parseStructs(const std::string& src, Module* module) {
        std::regex rx(R"(struct\s+([A-Za-z_]\w*)\s*\{([\s\S]*?)\}\s*;)");
        for (auto it = std::sregex_iterator(src.begin(), src.end(), rx); it != std::sregex_iterator(); ++it) {
            size_t pos = static_cast<size_t>((*it).position());
            if (insideTypedefStruct(src, pos)) continue;
            std::smatch m = *it;
            auto* cls = new ClassDeclaration(IdGenerator::next("cls"), trim(m[1].str()));
            parseStructBody(m[2].str(), cls);
            module->addChild("classes", cls);
        }
    }

    static void parseEnums(const std::string& src, Module* module) {
        std::regex rx(R"(enum\s*([A-Za-z_]\w*)?\s*\{([\s\S]*?)\}\s*;)");
        int anon = 0;
        for (auto it = std::sregex_iterator(src.begin(), src.end(), rx); it != std::sregex_iterator(); ++it) {
            std::smatch m = *it;
            auto* e = new EnumDeclaration();
            e->id = IdGenerator::next("enum");
            e->name = trim(m[1].str());
            if (e->name.empty()) e->name = "anonymous_enum_" + std::to_string(++anon);
            for (const auto& part : splitTopLevel(m[2].str(), ',')) {
                if (part.empty()) continue;
                size_t eq = part.find('=');
                std::string n = trim(eq == std::string::npos ? part : part.substr(0, eq));
                std::string v = trim(eq == std::string::npos ? "" : part.substr(eq + 1));
                if (!n.empty()) e->addChild("members", new EnumMember(IdGenerator::next("em"), n, v));
            }
            module->addChild("statements", e);
        }
    }

    static std::vector<std::pair<std::string, std::string>> parseParams(const std::string& txt) {
        std::vector<std::pair<std::string, std::string>> out;
        std::string p = trim(txt);
        if (p.empty() || p == "void") return out;
        std::regex fnPtr(R"(^(.+?)\(\s*\*\s*([A-Za-z_]\w*)\s*\)\s*\((.*)\)\s*$)");
        for (const auto& raw : splitTopLevel(p, ',')) {
            std::string t = trim(raw);
            if (t.empty()) continue;
            std::smatch m;
            if (std::regex_match(t, m, fnPtr)) {
                out.push_back({trim(m[2].str()), trim(m[1].str()) + " (*)(" + trim(m[3].str()) + ")"});
                continue;
            }
            size_t sp = t.find_last_of(" \t");
            if (sp == std::string::npos) { out.push_back({t, "int"}); continue; }
            std::string name = trim(t.substr(sp + 1));
            std::string type = trim(t.substr(0, sp));
            while (!name.empty() && name[0] == '*') { type += "*"; name.erase(name.begin()); }
            out.push_back({trim(name.empty() ? "param" : name), trim(type.empty() ? "int" : type)});
        }
        return out;
    }

    static void parseFunctions(const std::string& src, Module* module) {
        std::regex rx(
            R"((^|[\n\r])\s*((?:static|extern|const|inline|volatile|register|signed|unsigned|\w|\s|\*)+?)\s+([A-Za-z_]\w*)\s*\(([^;{}]*)\)\s*\{)",
            std::regex::ECMAScript);
        for (auto it = std::sregex_iterator(src.begin(), src.end(), rx); it != std::sregex_iterator(); ++it) {
            std::smatch m = *it;
            std::string ret = trim(m[2].str());
            std::string name = trim(m[3].str());
            if (name == "if" || name == "for" || name == "while" || name == "switch") continue;
            if (ret.find("typedef") != std::string::npos) continue;
            auto* fn = new Function(IdGenerator::next("fn"), name);
            setType(fn, "returnType", ret);
            for (const auto& [pn, pt] : parseParams(m[4].str())) {
                auto* p = new Parameter(IdGenerator::next("param"), pn);
                setType(p, "type", pt);
                fn->addChild("parameters", p);
            }
            module->addChild("functions", fn);
        }
    }

    static bool looksLikeType(const std::string& t) {
        static const std::vector<std::string> k = {
            "const","static","extern","unsigned","signed","volatile","register",
            "char","short","int","long","float","double","void","size_t","struct","enum"
        };
        for (const auto& x : k) if (t == x || startsWith(t, x + " ") || startsWith(t, x + "*")) return true;
        return false;
    }

    static void parseTopLevelVariables(const std::string& src, Module* module) {
        std::istringstream in(src);
        std::string line;
        int depth = 0;
        std::regex rx(R"(^\s*([A-Za-z_]\w*(?:\s+\w+)?(?:\s*\*+)?)\s+([A-Za-z_]\w*)\s*(=\s*[^;]+)?\s*;\s*$)");
        std::smatch m;
        while (std::getline(in, line)) {
            for (char c : line) { if (c == '{') depth++; else if (c == '}') depth--; }
            if (depth != 0) continue;
            std::string t = trim(line);
            if (t.empty() || startsWith(t, "#") || t.find('(') != std::string::npos) continue;
            if (t.find("typedef") != std::string::npos || t.find("enum ") != std::string::npos) continue;
            if (!std::regex_match(t, m, rx)) continue;
            std::string type = trim(m[1].str());
            if (!looksLikeType(type)) continue;
            auto* v = new Variable(IdGenerator::next("var"), trim(m[2].str()));
            setType(v, "type", type);
            module->addChild("variables", v);
        }
    }
};
