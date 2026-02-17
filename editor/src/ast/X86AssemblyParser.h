#pragma once

// Step 461: x86 Assembly Parser
// Supports Intel and AT&T syntax for core instruction/directive/label forms.

#include "AssemblyNodes.h"
#include "EnumNamespaceNodes.h"
#include "Function.h"
#include "Module.h"
#include "Parser.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

enum class X86SyntaxMode {
    Intel,
    ATT
};

class X86AssemblyParser {
public:
    static std::unique_ptr<Module> parseX86(const std::string& source,
                                            X86SyntaxMode mode = X86SyntaxMode::Intel) {
        auto result = parseX86WithDiagnostics(source, mode);
        return std::move(result.module);
    }

    static ParseResult parseX86WithDiagnostics(const std::string& source,
                                               X86SyntaxMode mode = X86SyntaxMode::Intel) {
        ParseResult out;
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_x86_module";
        module->targetLanguage = "x86_asm";

        std::set<std::string> globalLabels;
        NamespaceDeclaration* currentSection = nullptr;
        Function* currentFunction = nullptr;

        std::istringstream in(source);
        std::string raw;
        int lineNo = 0;
        while (std::getline(in, raw)) {
            ++lineNo;
            std::string line = stripComment(raw, mode);
            line = trim(line);
            if (line.empty()) continue;

            if (isDirective(line)) {
                auto dir = parseDirective(line);
                if (dir.type == AssemblyDirectiveType::Global) {
                    std::string g = trim(dir.value);
                    if (!g.empty()) globalLabels.insert(stripSigils(g));
                }
                if (dir.type == AssemblyDirectiveType::Text ||
                    dir.type == AssemblyDirectiveType::Data ||
                    dir.type == AssemblyDirectiveType::Section) {
                    std::string sectionName = dir.type == AssemblyDirectiveType::Section
                        ? trim(dir.value) : directiveTypeToString(dir.type);
                    currentSection = new NamespaceDeclaration(IdGenerator::next("ns"), sectionName);
                    module->addChild("statements", currentSection);
                    currentFunction = nullptr;
                }
                auto* d = new AssemblyDirective(dir.type, dir.value);
                if (currentSection) currentSection->addChild("body", d);
                else module->addChild("statements", d);
                continue;
            }

            if (isLabel(line)) {
                std::string labelName = parseLabelName(line);
                bool isGlobal = globalLabels.count(labelName) > 0;
                auto* label = new AssemblyLabel(labelName, isGlobal);

                currentFunction = new Function(IdGenerator::next("fn"), labelName);
                currentFunction->addChild("body", label);
                module->addChild("functions", currentFunction);
                continue;
            }

            auto inst = parseInstruction(line, mode);
            if (inst.opcode.empty()) {
                out.diagnostics.push_back({lineNo, 1, "Unrecognized assembly line: " + line, "warning"});
                continue;
            }
            if (!isKnownOpcode(inst.opcode)) {
                out.diagnostics.push_back({lineNo, 1, "Unknown opcode: " + inst.opcode, "warning"});
            }

            auto* node = new AssemblyInstruction(inst.opcode, inst.operands);
            node->registers = std::move(inst.registers);
            node->memoryOperands = std::move(inst.memoryOperands);

            if (!currentFunction) {
                currentFunction = new Function(IdGenerator::next("fn"), "_entry");
                module->addChild("functions", currentFunction);
            }
            currentFunction->addChild("body", node);
        }

        out.module = std::move(module);
        return out;
    }

private:
    static std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(" \t\r\n");
        return s.substr(start, end - start + 1);
    }

    static std::string stripComment(const std::string& line, X86SyntaxMode mode) {
        size_t pos = line.find(';');
        size_t pos2 = std::string::npos;
        if (mode == X86SyntaxMode::ATT) pos2 = line.find('#');
        size_t cut = std::string::npos;
        if (pos != std::string::npos) cut = pos;
        if (pos2 != std::string::npos) cut = (cut == std::string::npos) ? pos2 : std::min(cut, pos2);
        return cut == std::string::npos ? line : line.substr(0, cut);
    }

    static bool isDirective(const std::string& line) {
        return !line.empty() && line[0] == '.';
    }

    static bool isLabel(const std::string& line) {
        return !line.empty() && line.back() == ':';
    }

    static std::string parseLabelName(const std::string& line) {
        std::string n = line.substr(0, line.size() - 1);
        return stripSigils(trim(n));
    }

    static std::string stripSigils(const std::string& token) {
        std::string out = token;
        while (!out.empty() && (out[0] == '%' || out[0] == '$')) out.erase(out.begin());
        return out;
    }

    static AssemblyDirective parseDirective(const std::string& line) {
        size_t sp = line.find_first_of(" \t");
        std::string head = sp == std::string::npos ? line : line.substr(0, sp);
        std::string tail = sp == std::string::npos ? "" : trim(line.substr(sp + 1));
        return AssemblyDirective(directiveTypeFromString(head), tail);
    }

    static std::vector<std::string> splitOperands(const std::string& s) {
        std::vector<std::string> out;
        std::string cur;
        int bracketDepth = 0;
        int parenDepth = 0;
        for (char c : s) {
            if (c == '[') ++bracketDepth;
            if (c == ']') --bracketDepth;
            if (c == '(') ++parenDepth;
            if (c == ')') --parenDepth;
            if (c == ',' && bracketDepth == 0 && parenDepth == 0) {
                out.push_back(trim(cur));
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        if (!trim(cur).empty()) out.push_back(trim(cur));
        return out;
    }

    static bool isRegisterName(const std::string& raw) {
        std::string r = stripSigils(raw);
        static const std::set<std::string> regs = {
            "rax","rbx","rcx","rdx","rsi","rdi","rsp","rbp","rip",
            "eax","ebx","ecx","edx","esi","edi","esp","ebp","eip",
            "ax","bx","cx","dx","si","di","sp","bp","ip",
            "al","bl","cl","dl","sil","dil","spl","bpl",
            "r8","r9","r10","r11","r12","r13","r14","r15",
            "r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d",
            "r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w",
            "r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"
        };
        return regs.count(r) > 0;
    }

    static int registerSizeBits(const std::string& raw) {
        std::string r = stripSigils(raw);
        if (r.empty()) return 0;
        if (r == "rax" || r == "rbx" || r == "rcx" || r == "rdx" || r == "rsi" ||
            r == "rdi" || r == "rsp" || r == "rbp" || r == "rip" ||
            (r.size() >= 2 && r[0] == 'r' && std::isdigit(static_cast<unsigned char>(r[1])) &&
             (r.size() == 2 || (r[2] >= '0' && r[2] <= '9'))))
            return 64;
        if (r == "eax" || r == "ebx" || r == "ecx" || r == "edx" || r == "esi" ||
            r == "edi" || r == "esp" || r == "ebp" || r == "eip" ||
            (r.size() >= 3 && r[0] == 'r' && std::isdigit(static_cast<unsigned char>(r[1])) && r.back() == 'd'))
            return 32;
        if (r == "ax" || r == "bx" || r == "cx" || r == "dx" || r == "si" || r == "di" ||
            r == "sp" || r == "bp" || r == "ip" ||
            (r.size() >= 3 && r[0] == 'r' && std::isdigit(static_cast<unsigned char>(r[1])) && r.back() == 'w'))
            return 16;
        return 8;
    }

    static int parseInt(const std::string& raw) {
        std::string t = trim(raw);
        if (t.empty()) return 0;
        try {
            size_t idx = 0;
            int base = 10;
            if (t.size() > 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) base = 16;
            return std::stoi(t, &idx, base);
        } catch (...) {
            return 0;
        }
    }

    static bool parseIntelMemory(const std::string& op, AssemblyMemoryOperand& out) {
        if (op.size() < 2 || op.front() != '[' || op.back() != ']') return false;
        std::string inner = trim(op.substr(1, op.size() - 2));
        std::string token;
        std::string normalized;
        for (char c : inner) normalized.push_back(c == '-' ? '+' : c);
        std::stringstream ss(normalized);
        std::vector<std::string> parts;
        while (std::getline(ss, token, '+')) {
            token = trim(token);
            if (!token.empty()) parts.push_back(token);
        }
        bool sawNegative = inner.find('-') != std::string::npos;
        for (const auto& p : parts) {
            if (isRegisterName(p)) {
                if (out.baseRegister.empty()) out.baseRegister = stripSigils(p);
                else out.indexRegister = stripSigils(p);
            } else if (p.find('*') != std::string::npos) {
                size_t star = p.find('*');
                out.indexRegister = stripSigils(trim(p.substr(0, star)));
                out.scale = parseInt(trim(p.substr(star + 1)));
            } else {
                out.offset = parseInt(p);
            }
        }
        if (sawNegative && out.offset > 0) out.offset = -out.offset;
        return !out.baseRegister.empty() || !out.indexRegister.empty() || out.offset != 0;
    }

    static bool parseATTMemory(const std::string& op, AssemblyMemoryOperand& out) {
        // form: -8(%rbp,%rbx,4) or (%rax)
        std::regex rx(R"(^\s*([-+]?\d+)?\s*\(\s*%?([A-Za-z0-9]+)\s*(?:,\s*%?([A-Za-z0-9]+)\s*(?:,\s*(\d+)\s*)?)?\)\s*$)");
        std::smatch m;
        if (!std::regex_match(op, m, rx)) return false;
        out.offset = m[1].matched ? parseInt(m[1].str()) : 0;
        out.baseRegister = stripSigils(m[2].str());
        if (m[3].matched) out.indexRegister = stripSigils(m[3].str());
        if (m[4].matched) out.scale = parseInt(m[4].str());
        return true;
    }

    static AssemblyInstruction parseInstruction(const std::string& line, X86SyntaxMode mode) {
        size_t sp = line.find_first_of(" \t");
        std::string op = trim(sp == std::string::npos ? line : line.substr(0, sp));
        std::string ops = sp == std::string::npos ? "" : trim(line.substr(sp + 1));
        AssemblyInstruction inst(op, splitOperands(ops));

        for (const auto& operand : inst.operands) {
            if (isRegisterName(operand)) {
                inst.registers.emplace_back(stripSigils(operand), registerSizeBits(operand));
                continue;
            }
            AssemblyMemoryOperand mem;
            bool ok = (mode == X86SyntaxMode::Intel)
                ? parseIntelMemory(operand, mem)
                : parseATTMemory(operand, mem);
            if (ok) inst.memoryOperands.push_back(mem);
        }
        return inst;
    }

    static bool isKnownOpcode(const std::string& opRaw) {
        std::string op = opRaw;
        std::transform(op.begin(), op.end(), op.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        // Normalize AT&T suffixes (movq, addl, etc.).
        if (op.size() > 3 && (op.back() == 'b' || op.back() == 'w' || op.back() == 'l' || op.back() == 'q')) {
            std::string trimmed = op.substr(0, op.size() - 1);
            if (trimmed == "mov" || trimmed == "add" || trimmed == "sub" || trimmed == "cmp" ||
                trimmed == "push" || trimmed == "pop") op = trimmed;
        }
        static const std::set<std::string> known = {
            "mov", "add", "sub", "cmp", "jmp", "call", "ret", "push", "pop",
            "lea", "test", "and", "or", "xor", "nop"
        };
        return known.count(op) > 0;
    }
};
