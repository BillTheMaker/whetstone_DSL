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
#include <regex>

class KotlinParser {
public:
    static std::unique_ptr<Module> parseKotlin(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_kotlin_module";
        module->targetLanguage = "kotlin";

        parseTopLevel(source, module.get());
        return module;
    }

    static ParseResult parseKotlinWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseKotlin(source);
        return result;
    }

private:
    static void parseTopLevel(const std::string& source, Module* module) {
        std::istringstream stream(source);
        std::string line;
        std::string buffer;
        int braceDepth = 0;
        bool inFunction = false;
        bool inClass = false;
        std::string currentName;
        bool isSuspend = false;
        bool isDataClass = false;

        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);

            // Skip empty lines and comments
            if (trimmed.empty() || trimmed.substr(0, 2) == "//") continue;

            if (braceDepth == 0) {
                // Top-level declarations
                if (trimmed.find("fun ") != std::string::npos ||
                    trimmed.find("suspend fun ") != std::string::npos) {
                    isSuspend = trimmed.find("suspend ") != std::string::npos;
                    currentName = extractFunName(trimmed);
                    inFunction = true;
                    buffer = trimmed;
                }
                else if (trimmed.find("class ") != std::string::npos ||
                         trimmed.find("data class ") != std::string::npos) {
                    isDataClass = trimmed.find("data class ") != std::string::npos;
                    currentName = extractClassName(trimmed);
                    inClass = true;
                    buffer = trimmed;
                }
                else if (trimmed.find("val ") == 0 || trimmed.find("var ") == 0) {
                    auto* var = new Variable();
                    var->id = IdGenerator::next("var");
                    var->name = extractValName(trimmed);
                    module->addChild("variables", var);
                    continue;
                }
            }

            // Count braces
            for (char c : trimmed) {
                if (c == '{') braceDepth++;
                else if (c == '}') braceDepth--;
            }

            if (braceDepth > 0 || (inFunction || inClass)) {
                if (buffer != trimmed) buffer += "\n" + trimmed;
            }

            if (braceDepth == 0 && (inFunction || inClass)) {
                if (inFunction) {
                    if (isSuspend) {
                        auto* fn = new AsyncFunction();
                        fn->id = IdGenerator::next("fn");
                        fn->name = currentName;
                        fn->isAsync = true;
                        module->addChild("functions", fn);
                    } else {
                        auto* fn = new Function();
                        fn->id = IdGenerator::next("fn");
                        fn->name = currentName;
                        module->addChild("functions", fn);
                    }
                    inFunction = false;
                    isSuspend = false;
                }
                if (inClass) {
                    auto* cls = new ClassDeclaration();
                    cls->id = IdGenerator::next("cls");
                    cls->name = currentName;
                    module->addChild("classes", cls);
                    inClass = false;
                    isDataClass = false;
                }
                buffer.clear();
            }
        }
    }

    static std::string extractFunName(const std::string& line) {
        auto pos = line.find("fun ");
        if (pos == std::string::npos) return "unknown";
        pos += 4;
        auto end = line.find('(', pos);
        if (end == std::string::npos) end = line.size();
        return trim(line.substr(pos, end - pos));
    }

    static std::string extractClassName(const std::string& line) {
        std::string search = "class ";
        auto pos = line.find(search);
        if (pos == std::string::npos) return "unknown";
        pos += search.size();
        auto end = line.find_first_of("({: ", pos);
        if (end == std::string::npos) end = line.size();
        return trim(line.substr(pos, end - pos));
    }

    static std::string extractValName(const std::string& line) {
        auto pos = line.find(' ');
        if (pos == std::string::npos) return "unknown";
        pos++;
        auto end = line.find_first_of(":= ", pos);
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
