#pragma once

#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Statement.h"
#include "Expression.h"
#include "Annotation.h"
#include "ClassDeclaration.h"
#include "EnumNamespaceNodes.h"
#include "AsyncNodes.h"
#include "../ast/Parser.h"

#include <string>
#include <sstream>
#include <vector>

class FSharpParser {
public:
    static std::unique_ptr<Module> parseFSharp(const std::string& source) {
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_fsharp_module";
        module->targetLanguage = "fsharp";
        parseTopLevel(source, module.get());
        return module;
    }

    static ParseResult parseFSharpWithDiagnostics(const std::string& source) {
        ParseResult result;
        result.module = parseFSharp(source);
        return result;
    }

private:
    static void parseTopLevel(const std::string& source, Module* module) {
        std::istringstream stream(source);
        std::string line;

        bool inUnionType = false;
        EnumDeclaration* currentUnion = nullptr;

        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;
            if (startsWith(trimmed, "//")) continue;
            if (startsWith(trimmed, "(*")) continue;

            const bool topLevel = indentOf(line) == 0;

            if (topLevel && startsWith(trimmed, "module ")) {
                auto* ns = new NamespaceDeclaration();
                ns->id = IdGenerator::next("ns");
                ns->name = trim(trimmed.substr(7));
                module->addChild("statements", ns);
                inUnionType = false;
                currentUnion = nullptr;
                continue;
            }

            if (topLevel && startsWith(trimmed, "type ")) {
                inUnionType = false;
                currentUnion = nullptr;
                parseTypeDecl(trimmed, module, inUnionType, currentUnion);
                continue;
            }

            if (inUnionType && startsWith(trimmed, "|") && currentUnion) {
                parseUnionCase(trimmed, currentUnion);
                continue;
            }

            if (topLevel && startsWith(trimmed, "let ")) {
                parseLetDecl(trimmed, module);
                continue;
            }

            if (trimmed.find("match ") != std::string::npos &&
                trimmed.find(" with") != std::string::npos) {
                auto* ifStmt = new IfStatement();
                ifStmt->id = IdGenerator::next("if");
                module->addChild("statements", ifStmt);
                continue;
            }

            if (trimmed.find("|>") != std::string::npos) {
                parsePipelineCalls(trimmed, module);
                continue;
            }
        }
    }

    static void parseTypeDecl(const std::string& line, Module* module,
                              bool& inUnionType, EnumDeclaration*& currentUnion) {
        const auto eqPos = line.find('=');
        if (eqPos == std::string::npos) return;

        const std::string left = trim(line.substr(0, eqPos));
        const std::string rhs = trim(line.substr(eqPos + 1));
        const std::string typeName = trim(left.substr(5)); // after "type "
        if (typeName.empty()) return;

        if (startsWith(rhs, "{")) {
            auto* cls = new ClassDeclaration();
            cls->id = IdGenerator::next("cls");
            cls->name = typeName;
            parseRecordFields(rhs, cls);
            module->addChild("classes", cls);
            inUnionType = false;
            currentUnion = nullptr;
            return;
        }

        if (startsWith(rhs, "|")) {
            auto* en = new EnumDeclaration();
            en->id = IdGenerator::next("enum");
            en->name = typeName;
            module->addChild("statements", en);
            currentUnion = en;
            inUnionType = true;
            parseUnionCase(rhs, en);
            return;
        }

        // Multiline union form:
        // type Shape =
        // | Circle
        // | Rectangle of int
        if (rhs.empty()) {
            auto* en = new EnumDeclaration();
            en->id = IdGenerator::next("enum");
            en->name = typeName;
            module->addChild("statements", en);
            currentUnion = en;
            inUnionType = true;
            return;
        }
    }

    static void parseRecordFields(const std::string& rhs, ClassDeclaration* cls) {
        auto start = rhs.find('{');
        auto end = rhs.rfind('}');
        if (start == std::string::npos || end == std::string::npos || end <= start) return;

        std::string fields = rhs.substr(start + 1, end - start - 1);
        std::stringstream ss(fields);
        std::string token;
        while (std::getline(ss, token, ';')) {
            std::string f = trim(token);
            if (f.empty()) continue;
            auto colon = f.find(':');
            std::string name = trim(colon == std::string::npos ? f : f.substr(0, colon));
            if (name.empty()) continue;
            auto* var = new Variable();
            var->id = IdGenerator::next("fld");
            var->name = name;
            cls->addChild("fields", var);
        }
    }

    static void parseUnionCase(const std::string& line, EnumDeclaration* en) {
        if (!en) return;
        std::string caseText = trim(line);
        if (!caseText.empty() && caseText[0] == '|') {
            caseText = trim(caseText.substr(1));
        }
        auto ofPos = caseText.find(" of ");
        std::string caseName = trim(ofPos == std::string::npos ? caseText : caseText.substr(0, ofPos));
        if (caseName.empty()) return;
        auto* member = new EnumMember();
        member->id = IdGenerator::next("case");
        member->name = caseName;
        en->addChild("members", member);
    }

    static void parseLetDecl(const std::string& line, Module* module) {
        std::string rest = trim(line.substr(4)); // after "let "

        bool isMutable = false;
        if (startsWith(rest, "mutable ")) {
            isMutable = true;
            rest = trim(rest.substr(8));
        }

        auto eqPos = rest.find('=');
        if (eqPos == std::string::npos) return;

        std::string left = trim(rest.substr(0, eqPos));
        std::string right = trim(rest.substr(eqPos + 1));
        if (left.empty()) return;

        std::vector<std::string> tokens = splitWhitespace(left);
        if (tokens.empty()) return;

        if (right.find("async {") != std::string::npos) {
            auto* fn = new AsyncFunction();
            fn->id = IdGenerator::next("fn");
            fn->name = tokens[0];
            module->addChild("functions", fn);
            return;
        }

        if (tokens.size() >= 2) {
            auto* fn = new Function();
            fn->id = IdGenerator::next("fn");
            fn->name = tokens[0];
            module->addChild("functions", fn);
            return;
        }

        auto* var = new Variable();
        var->id = IdGenerator::next("var");
        var->name = tokens[0];
        if (isMutable) {
            auto* mut = new MutAnnotation();
            mut->id = IdGenerator::next("mut");
            mut->depth = "shallow";
            var->addChild("annotations", mut);
        }
        module->addChild("variables", var);
    }

    static void parsePipelineCalls(const std::string& line, Module* module) {
        size_t pos = 0;
        while (true) {
            pos = line.find("|>", pos);
            if (pos == std::string::npos) break;
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            call->functionName = "pipeline";
            module->addChild("statements", call);
            pos += 2;
        }
    }

    static std::vector<std::string> splitWhitespace(const std::string& s) {
        std::vector<std::string> out;
        std::stringstream ss(s);
        std::string tok;
        while (ss >> tok) out.push_back(tok);
        return out;
    }

    static bool startsWith(const std::string& s, const std::string& prefix) {
        return s.rfind(prefix, 0) == 0;
    }

    static int indentOf(const std::string& line) {
        int indent = 0;
        for (char c : line) {
            if (c == ' ') indent++;
            else if (c == '\t') indent += 4;
            else break;
        }
        return indent;
    }

    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        auto end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }
};
