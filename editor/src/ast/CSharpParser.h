#pragma once
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Statement.h"
#include "Expression.h"
#include "Type.h"
#include "Annotation.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "../ast/Parser.h"
#include <string>
#include <sstream>

class CSharpParser {
public:
    static std::unique_ptr<Module> parseCSharp(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_csharp_module";
        module->targetLanguage = "csharp";

        parseTopLevel(source, module.get());
        return module;
    }

    static ParseResult parseCSharpWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseCSharp(source);
        return result;
    }

private:
    static void parseTopLevel(const std::string& source, Module* module) {
        std::istringstream stream(source);
        std::string line;
        int braceDepth = 0;
        bool inFunction = false;
        bool inClass = false;
        std::string currentName;
        bool isAsync = false;

        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty() || trimmed.substr(0, 2) == "//") continue;
            if (trimmed.find("using ") == 0) continue;  // skip using directives
            if (trimmed.find("namespace ") == 0) continue;  // skip namespace

            if (braceDepth <= 1) {
                if (hasMethodSignature(trimmed)) {
                    isAsync = trimmed.find("async ") != std::string::npos;
                    currentName = extractMethodName(trimmed);
                    inFunction = true;
                }
                else if (trimmed.find("class ") != std::string::npos) {
                    currentName = extractAfterKeyword(trimmed, "class ");
                    inClass = true;
                }
                else if (trimmed.find("interface ") != std::string::npos) {
                    currentName = extractAfterKeyword(trimmed, "interface ");
                    auto* iface = new InterfaceDeclaration();
                    iface->id = IdGenerator::next("iface");
                    iface->name = currentName;
                    module->addChild("interfaces", iface);
                }
            }

            for (char c : trimmed) {
                if (c == '{') braceDepth++;
                else if (c == '}') braceDepth--;
            }

            if (braceDepth <= 1 && inFunction) {
                if (isAsync) {
                    auto* fn = new AsyncFunction();
                    fn->id = IdGenerator::next("fn");
                    fn->name = currentName;
                    module->addChild("functions", fn);
                } else {
                    auto* fn = new Function();
                    fn->id = IdGenerator::next("fn");
                    fn->name = currentName;
                    module->addChild("functions", fn);
                }
                inFunction = false;
                isAsync = false;
            }
            if (braceDepth == 0 && inClass) {
                auto* cls = new ClassDeclaration();
                cls->id = IdGenerator::next("cls");
                cls->name = currentName;
                module->addChild("classes", cls);
                inClass = false;
            }
        }
    }

    static bool hasMethodSignature(const std::string& line) {
        // Look for patterns like "public void Method(" or "static async Task<int> Method("
        if (line.find('(') == std::string::npos) return false;
        if (line.find("class ") != std::string::npos) return false;
        if (line.find("if ") != std::string::npos || line.find("if(") != std::string::npos) return false;
        if (line.find("while ") != std::string::npos) return false;
        if (line.find("for ") != std::string::npos || line.find("foreach ") != std::string::npos) return false;
        // Has visibility or return type keyword
        return line.find("void ") != std::string::npos ||
               line.find("int ") != std::string::npos ||
               line.find("string ") != std::string::npos ||
               line.find("Task") != std::string::npos ||
               line.find("public ") != std::string::npos ||
               line.find("private ") != std::string::npos ||
               line.find("static ") != std::string::npos;
    }

    static std::string extractMethodName(const std::string& line) {
        auto parenPos = line.find('(');
        if (parenPos == std::string::npos) return "unknown";
        // Walk back from '(' to find the method name
        auto nameEnd = parenPos;
        auto nameStart = line.rfind(' ', nameEnd - 1);
        if (nameStart == std::string::npos) nameStart = 0;
        else nameStart++;
        return trim(line.substr(nameStart, nameEnd - nameStart));
    }

    static std::string extractAfterKeyword(const std::string& line, const std::string& keyword) {
        auto pos = line.find(keyword);
        if (pos == std::string::npos) return "unknown";
        pos += keyword.size();
        auto end = line.find_first_of("{: <(", pos);
        if (end == std::string::npos) end = line.size();
        return trim(line.substr(pos, end - pos));
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};
