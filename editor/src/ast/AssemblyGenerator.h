#pragma once

// Step 463: Assembly Generator (x86 + ARM)
// Generates assembly text from Assembly AST nodes and provides a simple
// cross-architecture mapper for basic instruction sequences.

#include "AssemblyNodes.h"
#include "EnumNamespaceNodes.h"
#include "Function.h"
#include "Module.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

enum class X86OutputSyntax {
    Intel,
    ATT
};

class AssemblyGenerator {
public:
    static std::string commentPrefix(const std::string& arch) {
        if (arch == "x86") return "; ";
        if (arch == "arm") return "@ ";
        return "; ";
    }

    static std::string generateX86(const Module& module,
                                   X86OutputSyntax syntax = X86OutputSyntax::Intel) {
        std::ostringstream out;
        emitModuleDirectives(module, out);
        for (auto* fnNode : module.getChildren("functions")) {
            if (fnNode->conceptType != "Function") continue;
            auto* fn = static_cast<Function*>(fnNode);
            emitFunctionX86(*fn, out, syntax);
        }
        return out.str();
    }

    static std::string generateArm(const Module& module) {
        std::ostringstream out;
        emitModuleDirectives(module, out);
        for (auto* fnNode : module.getChildren("functions")) {
            if (fnNode->conceptType != "Function") continue;
            auto* fn = static_cast<Function*>(fnNode);
            emitFunctionArm(*fn, out);
        }
        return out.str();
    }

    static std::vector<AssemblyInstruction> translateX86ToArmSimple(
        const std::vector<AssemblyInstruction>& x86Instructions) {
        std::vector<AssemblyInstruction> out;
        for (const auto& i : x86Instructions) {
            std::string op = toLower(i.opcode);
            AssemblyInstruction mapped;
            mapped.operands = i.operands;
            if (op == "mov" || op == "movq" || op == "movl") mapped.opcode = "mov";
            else if (op == "add" || op == "addq" || op == "addl") mapped.opcode = "add";
            else if (op == "sub" || op == "subq" || op == "subl") mapped.opcode = "sub";
            else if (op == "cmp" || op == "cmpq" || op == "cmpl") mapped.opcode = "cmp";
            else if (op == "jmp") mapped.opcode = "b";
            else if (op == "call") mapped.opcode = "bl";
            else if (op == "ret") mapped.opcode = "ret";
            else mapped.opcode = op;
            out.push_back(std::move(mapped));
        }
        return out;
    }

private:
    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static std::string emitDirective(const AssemblyDirective& d) {
        if (d.value.empty()) return directiveTypeToString(d.type);
        return directiveTypeToString(d.type) + " " + d.value;
    }

    static std::string formatOperandX86(const std::string& operand, X86OutputSyntax syntax) {
        if (syntax == X86OutputSyntax::Intel) return operand;

        std::string o = operand;
        // naive register sigil conversion for AT&T
        if (!o.empty() && o[0] != '[' && o[0] != '%' && std::isalpha(static_cast<unsigned char>(o[0]))) {
            bool maybeReg = true;
            for (char c : o) {
                if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
                    maybeReg = false;
                    break;
                }
            }
            if (maybeReg) o = "%" + o;
        }
        if (!o.empty() && std::isdigit(static_cast<unsigned char>(o[0]))) {
            o = "$" + o;
        }
        return o;
    }

    static void emitFunctionX86(const Function& fn, std::ostringstream& out, X86OutputSyntax syntax) {
        out << fn.name << ":\n";
        for (auto* n : fn.getChildren("body")) {
            if (n->conceptType == "AssemblyLabel") continue; // function name already emitted
            if (n->conceptType == "AssemblyInstruction") {
                auto* i = static_cast<AssemblyInstruction*>(n);
                out << "  " << i->opcode;
                if (!i->operands.empty()) {
                    out << " ";
                    for (size_t k = 0; k < i->operands.size(); ++k) {
                        out << formatOperandX86(i->operands[k], syntax);
                        if (k + 1 < i->operands.size()) out << ", ";
                    }
                }
                out << "\n";
            } else if (n->conceptType == "AssemblyDirective") {
                auto* d = static_cast<AssemblyDirective*>(n);
                out << "  " << emitDirective(*d) << "\n";
            }
        }
    }

    static void emitFunctionArm(const Function& fn, std::ostringstream& out) {
        out << fn.name << ":\n";
        for (auto* n : fn.getChildren("body")) {
            if (n->conceptType == "AssemblyLabel") continue;
            if (n->conceptType == "AssemblyInstruction") {
                auto* i = static_cast<AssemblyInstruction*>(n);
                out << "  " << toLower(i->opcode);
                if (!i->operands.empty()) {
                    out << " ";
                    for (size_t k = 0; k < i->operands.size(); ++k) {
                        out << i->operands[k];
                        if (k + 1 < i->operands.size()) out << ", ";
                    }
                }
                out << "\n";
            } else if (n->conceptType == "AssemblyDirective") {
                auto* d = static_cast<AssemblyDirective*>(n);
                out << "  " << emitDirective(*d) << "\n";
            }
        }
    }

    static void emitModuleDirectives(const Module& module, std::ostringstream& out) {
        for (auto* s : module.getChildren("statements")) {
            if (s->conceptType == "AssemblyDirective") {
                auto* d = static_cast<AssemblyDirective*>(s);
                out << emitDirective(*d) << "\n";
            } else if (s->conceptType == "NamespaceDeclaration") {
                auto* ns = static_cast<NamespaceDeclaration*>(s);
                if (!ns->name.empty()) out << ns->name << "\n";
                for (auto* b : ns->getChildren("body")) {
                    if (b->conceptType == "AssemblyDirective") {
                        auto* d = static_cast<AssemblyDirective*>(b);
                        out << emitDirective(*d) << "\n";
                    }
                }
            }
        }
    }
};
