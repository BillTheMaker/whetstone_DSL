#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Type.h"
#include "Import.h"
#include "Statement.h"
#include "Expression.h"
#include "PreprocessorNodes.h"
#include "../ast/Parser.h"
#include <algorithm>
#include <regex>
#include <string>
#include <vector>

class WatParser {
public:
    static std::unique_ptr<Module> parseWat(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_wat_module";
        module->targetLanguage = "wat";

        std::string cleaned = stripComments(source);
        for (const auto& expr : topLevelExprs(cleaned)) {
            if (startsWith(expr, "(func")) {
                module->addChild("functions", parseFunction(expr));
            } else if (startsWith(expr, "(import")) {
                parseImport(expr, module.get());
            } else if (startsWith(expr, "(export")) {
                auto* p = new PragmaDirective(IdGenerator::next("prag"), "export");
                module->addChild("statements", p);
            } else if (startsWith(expr, "(global")) {
                parseGlobal(expr, module.get());
            } else if (startsWith(expr, "(memory")) {
                auto* p = new PragmaDirective(IdGenerator::next("prag"), "memory");
                module->addChild("statements", p);
            } else if (startsWith(expr, "(table")) {
                auto* p = new PragmaDirective(IdGenerator::next("prag"), "table");
                module->addChild("statements", p);
            }
        }

        return module;
    }

    static ParseResult parseWatWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseWat(source);
        int depth = 0;
        for (char c : source) {
            if (c == '(') ++depth;
            if (c == ')') --depth;
            if (depth < 0) {
                result.diagnostics.push_back(
                    {1, 1, "Unmatched closing parenthesis in WAT source", "error"});
                depth = 0;
                break;
            }
        }
        if (depth != 0) {
            result.diagnostics.push_back(
                {1, 1, "Unbalanced parentheses in WAT source", "error"});
        }
        if (source.find("(module") == std::string::npos) {
            result.diagnostics.push_back(
                {1, 1, "WAT source should contain a module form", "warning"});
        }
        return result;
    }

private:
    static std::string stripComments(const std::string& source) {
        std::string out;
        out.reserve(source.size());
        bool lineComment = false;
        for (size_t i = 0; i < source.size(); ++i) {
            char c = source[i];
            char n = (i + 1 < source.size()) ? source[i + 1] : '\0';
            if (!lineComment && c == ';' && n == ';') {
                lineComment = true;
                out += "  ";
                ++i;
                continue;
            }
            if (lineComment && c == '\n') {
                lineComment = false;
                out.push_back(c);
                continue;
            }
            out.push_back(lineComment ? ' ' : c);
        }
        return out;
    }

    static std::vector<std::string> topLevelExprs(const std::string& source) {
        std::vector<std::string> out;
        int depth = 0;
        int moduleDepth = -1;
        size_t moduleStart = std::string::npos;

        for (size_t i = 0; i < source.size(); ++i) {
            if (source[i] == '(') {
                ++depth;
                if (moduleDepth == -1 && startsWithAt(source, i, "(module")) {
                    moduleDepth = depth;
                } else if (moduleDepth != -1 && depth == moduleDepth + 1) {
                    moduleStart = i;
                }
            } else if (source[i] == ')') {
                if (moduleDepth != -1 && moduleStart != std::string::npos &&
                    depth == moduleDepth + 1) {
                    out.push_back(trim(source.substr(moduleStart, i - moduleStart + 1)));
                    moduleStart = std::string::npos;
                }
                --depth;
                if (depth < moduleDepth) moduleDepth = -1;
            }
        }

        if (out.empty()) {
            std::string t = trim(source);
            if (startsWith(t, "(func") || startsWith(t, "(module")) out.push_back(t);
        }
        return out;
    }

    static Function* parseFunction(const std::string& expr) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        fn->name = parseFunctionName(expr);

        std::regex paramRx(R"(\(\s*param\s+(\$[A-Za-z_][\w\.-]*|[A-Za-z_][\w\.-]*)\s+([A-Za-z0-9_\.]+)\s*\))");
        for (auto it = std::sregex_iterator(expr.begin(), expr.end(), paramRx);
             it != std::sregex_iterator(); ++it) {
            std::smatch m = *it;
            auto* p = new Parameter(IdGenerator::next("param"), trim(m[1].str()));
            p->setChild("type", makeType(m[2].str()));
            fn->addChild("parameters", p);
        }

        std::regex resultRx(R"(\(\s*result\s+([A-Za-z0-9_\.]+)\s*\))");
        std::smatch resultMatch;
        if (std::regex_search(expr, resultMatch, resultRx)) {
            fn->setChild("returnType", makeType(resultMatch[1].str()));
        }

        std::regex localRx(R"(\(\s*local\s+(\$[A-Za-z_][\w\.-]*|[A-Za-z_][\w\.-]*)\s+([A-Za-z0-9_\.]+)\s*\))");
        for (auto it = std::sregex_iterator(expr.begin(), expr.end(), localRx);
             it != std::sregex_iterator(); ++it) {
            std::smatch m = *it;
            auto* v = new Variable(IdGenerator::next("var"), trim(m[1].str()));
            v->setChild("type", makeType(m[2].str()));
            fn->addChild("body", v);
        }

        appendInstructionNodes(expr, fn);
        return fn;
    }

    static void parseImport(const std::string& expr, Module* module) {
        std::regex importRx(
            R"wat(\(\s*import\s+"([^"]+)"\s+"([^"]+)"\s+\(\s*func\s*(\$[A-Za-z_][\w\.-]*)?)wat");
        std::smatch m;
        if (std::regex_search(expr, m, importRx)) {
            std::string modName = trim(m[1].str());
            std::string name = trim(m[2].str());
            auto* imp = new Import(IdGenerator::next("imp"), modName + "." + name, "runtime", "");
            module->addChild("imports", imp);
        }
    }

    static void parseGlobal(const std::string& expr, Module* module) {
        std::regex gRx(R"(\(\s*global\s+(\$[A-Za-z_][\w\.-]*|[A-Za-z_][\w\.-]*)?)");
        std::smatch m;
        std::string name = "global_" + IdGenerator::next("g");
        if (std::regex_search(expr, m, gRx) && !trim(m[1].str()).empty()) {
            name = trim(m[1].str());
        }
        auto* v = new Variable(IdGenerator::next("var"), name);
        v->setChild("type", new PrimitiveType(IdGenerator::next("type"), "i32"));
        module->addChild("variables", v);
    }

    static void appendInstructionNodes(const std::string& expr, Function* fn) {
        if (expr.find("i32.add") != std::string::npos) {
            auto* b = new BinaryOperation();
            b->id = IdGenerator::next("bin");
            b->op = "+";
            b->setChild("left", new IntegerLiteral(IdGenerator::next("lit"), 0));
            b->setChild("right", new IntegerLiteral(IdGenerator::next("lit"), 0));
            fn->addChild("body", b);
        }

        std::regex callRx(R"(\bcall\s+(\$[A-Za-z_][\w\.-]*|[A-Za-z_][\w\.-]*))");
        std::smatch callMatch;
        if (std::regex_search(expr, callMatch, callRx)) {
            auto* c = new FunctionCall();
            c->id = IdGenerator::next("call");
            c->functionName = trim(callMatch[1].str());
            fn->addChild("body", c);
        }

        if (expr.find("(if") != std::string::npos) {
            auto* ifs = new IfStatement();
            ifs->id = IdGenerator::next("if");
            ifs->setChild("condition", new BooleanLiteral(IdGenerator::next("bool"), true));
            fn->addChild("body", ifs);
        }

        if (expr.find("(block") != std::string::npos || expr.find("(loop") != std::string::npos) {
            auto* block = new Block();
            block->id = IdGenerator::next("blk");
            fn->addChild("body", block);
        }
    }

    static ASTNode* makeType(const std::string& watType) {
        std::string t = trim(watType);
        return new PrimitiveType(IdGenerator::next("type"), t);
    }

    static std::string parseFunctionName(const std::string& expr) {
        std::regex nameRx(R"(\(\s*func\s+(\$[A-Za-z_][\w\.-]*|[A-Za-z_][\w\.-]*))");
        std::smatch m;
        if (std::regex_search(expr, m, nameRx)) {
            return trim(m[1].str());
        }
        return "wat_function_" + IdGenerator::next("fn");
    }

    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static bool startsWith(const std::string& value, const std::string& prefix) {
        return value.rfind(prefix, 0) == 0;
    }

    static bool startsWithAt(const std::string& value, size_t pos, const std::string& prefix) {
        if (pos + prefix.size() > value.size()) return false;
        return value.compare(pos, prefix.size(), prefix) == 0;
    }
};
