#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Statement.h"
#include "ClassDeclaration.h"
#include "EnumNamespaceNodes.h"
#include "../ast/Parser.h"

#include <string>
#include <sstream>
#include <cctype>

class VBNetParser {
public:
    static std::unique_ptr<Module> parseVBNet(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_vbnet_module";
        module->targetLanguage = "vbnet";
        parseTopLevel(source, module.get());
        return module;
    }

    static ParseResult parseVBNetWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseVBNet(source);
        return result;
    }

private:
    static void parseTopLevel(const std::string& source, Module* module) {
        std::istringstream stream(source);
        std::string line;

        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;
            if (startsWithCaseInsensitive(trimmed, "'")) continue;

            if (startsWithCaseInsensitive(trimmed, "Class ")) {
                auto* cls = new ClassDeclaration();
                cls->id = IdGenerator::next("cls");
                cls->name = extractNameAfterKeyword(trimmed, "Class ");
                module->addChild("classes", cls);
                continue;
            }

            if (startsWithCaseInsensitive(trimmed, "Interface ")) {
                auto* iface = new InterfaceDeclaration();
                iface->id = IdGenerator::next("iface");
                iface->name = extractNameAfterKeyword(trimmed, "Interface ");
                module->addChild("interfaces", iface);
                continue;
            }

            if (startsWithCaseInsensitive(trimmed, "Module ")) {
                auto* ns = new NamespaceDeclaration();
                ns->id = IdGenerator::next("ns");
                ns->name = extractNameAfterKeyword(trimmed, "Module ");
                module->addChild("statements", ns);
                continue;
            }

            if (containsCaseInsensitive(trimmed, "Sub ") &&
                !startsWithCaseInsensitive(trimmed, "End Sub")) {
                auto* fn = new Function();
                fn->id = IdGenerator::next("fn");
                fn->name = extractMethodName(trimmed, "Sub ");
                module->addChild("functions", fn);
                continue;
            }

            if (containsCaseInsensitive(trimmed, "Function ") &&
                !startsWithCaseInsensitive(trimmed, "End Function")) {
                auto* fn = new Function();
                fn->id = IdGenerator::next("fn");
                fn->name = extractMethodName(trimmed, "Function ");
                module->addChild("functions", fn);
                continue;
            }

            if (startsWithCaseInsensitive(trimmed, "Dim ")) {
                auto* var = new Variable();
                var->id = IdGenerator::next("var");
                var->name = extractDimName(trimmed);
                module->addChild("variables", var);
                continue;
            }

            if (startsWithCaseInsensitive(trimmed, "If ") &&
                containsCaseInsensitive(trimmed, " Then")) {
                auto* ifs = new IfStatement();
                ifs->id = IdGenerator::next("if");
                module->addChild("statements", ifs);
                continue;
            }

            if (startsWithCaseInsensitive(trimmed, "For Each ") &&
                containsCaseInsensitive(trimmed, " In ")) {
                auto* loop = new ForLoop();
                loop->id = IdGenerator::next("for");
                loop->iteratorName = extractForEachIterator(trimmed);
                module->addChild("statements", loop);
                continue;
            }
        }
    }

    static std::string extractForEachIterator(const std::string& line) {
        std::string lower = toLower(line);
        size_t start = lower.find("for each ");
        if (start == std::string::npos) return "item";
        start += 9;
        size_t inPos = lower.find(" in ", start);
        if (inPos == std::string::npos) return "item";
        std::string token = trim(line.substr(start, inPos - start));
        // "x As Type" => keep x only
        auto asPos = toLower(token).find(" as ");
        if (asPos != std::string::npos) token = trim(token.substr(0, asPos));
        return token.empty() ? "item" : token;
    }

    static std::string extractMethodName(const std::string& line, const std::string& keyword) {
        std::string lower = toLower(line);
        std::string lowerKeyword = toLower(keyword);
        auto pos = lower.find(lowerKeyword);
        if (pos == std::string::npos) return "unknown";
        pos += keyword.size();
        auto end = line.find('(', pos);
        if (end == std::string::npos) end = line.find(' ', pos);
        if (end == std::string::npos) end = line.size();
        return trim(line.substr(pos, end - pos));
    }

    static std::string extractDimName(const std::string& line) {
        auto after = trim(line.substr(4)); // after "Dim "
        auto asPos = toLower(after).find(" as ");
        auto eqPos = after.find('=');
        size_t end = std::string::npos;
        if (asPos != std::string::npos && eqPos != std::string::npos) end = std::min(asPos, eqPos);
        else if (asPos != std::string::npos) end = asPos;
        else if (eqPos != std::string::npos) end = eqPos;
        if (end == std::string::npos) end = after.size();
        return trim(after.substr(0, end));
    }

    static std::string extractNameAfterKeyword(const std::string& line, const std::string& keyword) {
        auto pos = line.find(keyword);
        if (pos == std::string::npos) return "unknown";
        pos += keyword.size();
        auto end = line.find_first_of("(: ", pos);
        if (end == std::string::npos) end = line.size();
        return trim(line.substr(pos, end - pos));
    }

    static bool startsWithCaseInsensitive(const std::string& s, const std::string& prefix) {
        if (prefix.size() > s.size()) return false;
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(s[i])) !=
                std::tolower(static_cast<unsigned char>(prefix[i]))) return false;
        }
        return true;
    }

    static bool containsCaseInsensitive(const std::string& s, const std::string& needle) {
        return toLower(s).find(toLower(needle)) != std::string::npos;
    }

    static std::string toLower(const std::string& s) {
        std::string out = s;
        for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};
