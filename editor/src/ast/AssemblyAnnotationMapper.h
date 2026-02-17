#pragma once

// Step 464: Assembly Annotation Mapping
// Maps assembly constructs to semantic annotations and lightweight cross-language
// lift/lower helpers.

#include "AssemblyNodes.h"
#include "Function.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

struct AssemblyAnnotationReport {
    std::string functionName;
    std::vector<std::string> annotations;
    int instructionCount = 0;
    int branchCount = 0;
    int registerPressure = 0;
    std::string complexity; // low/medium/high
};

class AssemblyAnnotationMapper {
public:
    static AssemblyAnnotationReport analyzeFunction(const Function& fn) {
        AssemblyAnnotationReport report;
        report.functionName = fn.name;
        report.annotations.push_back("@Exec(native)");
        report.annotations.push_back("@Risk(high)");

        std::set<std::string> uniqueRegs;
        std::string target = "x86";
        for (auto* node : fn.getChildren("body")) {
            if (node->conceptType == "AssemblyInstruction") {
                auto* i = static_cast<AssemblyInstruction*>(node);
                ++report.instructionCount;
                if (isBranch(i->opcode)) ++report.branchCount;
                for (const auto& r : i->registers) uniqueRegs.insert(r.name);
                if (looksArmRegisterSet(i->registers)) target = "arm";
            } else if (node->conceptType == "AssemblyDirective") {
                auto* d = static_cast<AssemblyDirective*>(node);
                if (d->type == AssemblyDirectiveType::Align) {
                    report.annotations.push_back("@Align(" + d->value + ")");
                }
            }
        }

        report.registerPressure = static_cast<int>(uniqueRegs.size());
        report.complexity = inferComplexity(
            report.instructionCount, report.branchCount, report.registerPressure);
        report.annotations.push_back("@Complexity(" + report.complexity + ")");
        report.annotations.push_back("@Target(" + target + ")");
        return report;
    }

    static std::string lowerCToAssembly(const std::string& cFunctionSignature,
                                        const std::string& arch = "x86") {
        std::string name = extractFunctionName(cFunctionSignature);
        std::ostringstream out;
        if (arch == "arm") {
            out << ".text\n.global " << name << "\n" << name << ":\n";
            out << "  @ lowered from C function\n";
            out << "  ret\n";
        } else {
            out << ".text\n.global " << name << "\n" << name << ":\n";
            out << "  ; lowered from C function\n";
            out << "  ret\n";
        }
        return out.str();
    }

    static std::string liftAssemblyToC(const Function& fn) {
        std::ostringstream out;
        out << "int " << (fn.name.empty() ? "lifted_function" : fn.name) << "() {\n";
        out << "  // lifted from assembly (" << fn.getChildren("body").size() << " nodes)\n";
        out << "  return 0;\n";
        out << "}\n";
        return out.str();
    }

private:
    static bool isBranch(const std::string& opRaw) {
        std::string op = toLower(opRaw);
        return op == "jmp" || op == "call" || op == "ret" ||
               op == "b" || op == "bl" || op == "beq" || op == "bne" ||
               op == "bgt" || op == "blt" || op == "bge" || op == "ble";
    }

    static bool looksArmRegisterSet(const std::vector<AssemblyRegister>& regs) {
        for (const auto& r : regs) {
            if (r.name == "sp" || r.name == "lr" || r.name == "pc") return true;
            if (!r.name.empty() && (r.name[0] == 'r' || r.name[0] == 'x')) {
                bool numericSuffix = r.name.size() > 1;
                for (size_t i = 1; i < r.name.size(); ++i) {
                    if (!std::isdigit(static_cast<unsigned char>(r.name[i]))) {
                        numericSuffix = false;
                        break;
                    }
                }
                if (numericSuffix) return true; // r0-r15, x0-x30 style
            }
        }
        return false;
    }

    static std::string inferComplexity(int instructions, int branches, int regPressure) {
        int score = instructions + branches * 2 + regPressure;
        if (score <= 8) return "low";
        if (score <= 18) return "medium";
        return "high";
    }

    static std::string toLower(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static std::string extractFunctionName(const std::string& cSig) {
        size_t paren = cSig.find('(');
        if (paren == std::string::npos) return "function";
        std::string left = cSig.substr(0, paren);
        size_t sp = left.find_last_of(" \t");
        if (sp == std::string::npos) return left;
        std::string name = left.substr(sp + 1);
        return name.empty() ? "function" : name;
    }
};
