#pragma once

// Step 462: ARM Assembly Parser
// Supports ARM/AArch64 core instruction/directive/label forms.

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

class ArmAssemblyParser {
public:
    static std::unique_ptr<Module> parseArm(const std::string& source) {
        auto result = parseArmWithDiagnostics(source);
        return std::move(result.module);
    }

    static ParseResult parseArmWithDiagnostics(const std::string& source) {
        ParseResult out;
        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_arm_module";
        module->targetLanguage = "arm_asm";

        std::set<std::string> globalLabels;
        NamespaceDeclaration* currentSection = nullptr;
        Function* currentFunction = nullptr;

        std::istringstream in(source);
        std::string raw;
        int lineNo = 0;
        while (std::getline(in, raw)) {
            ++lineNo;
            std::string line = trim(stripComment(raw));
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
                std::string name = parseLabelName(line);
                bool isGlobal = globalLabels.count(name) > 0;
                auto* label = new AssemblyLabel(name, isGlobal);
                currentFunction = new Function(IdGenerator::next("fn"), name);
                currentFunction->addChild("body", label);
                module->addChild("functions", currentFunction);
                continue;
            }

            auto inst = parseInstruction(line);
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

    static std::string stripComment(const std::string& line) {
        size_t at = line.find('@');
        size_t sem = line.find(';');
        size_t slash = line.find("//");
        size_t cut = std::string::npos;
        if (at != std::string::npos) cut = at;
        if (sem != std::string::npos) cut = cut == std::string::npos ? sem : std::min(cut, sem);
        if (slash != std::string::npos) cut = cut == std::string::npos ? slash : std::min(cut, slash);
        return cut == std::string::npos ? line : line.substr(0, cut);
    }

    static bool isDirective(const std::string& line) {
        return !line.empty() && line[0] == '.';
    }

    static bool isLabel(const std::string& line) {
        return !line.empty() && line.back() == ':';
    }

    static std::string stripSigils(const std::string& token) {
        std::string out = token;
        while (!out.empty() && (out[0] == '#' || out[0] == '%')) out.erase(out.begin());
        return out;
    }

    static std::string parseLabelName(const std::string& line) {
        std::string n = line.substr(0, line.size() - 1);
        return stripSigils(trim(n));
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
        for (char c : s) {
            if (c == '[') ++bracketDepth;
            if (c == ']') --bracketDepth;
            if (c == ',' && bracketDepth == 0) {
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
        std::string r = toLower(stripSigils(raw));
        if (r == "sp" || r == "lr" || r == "pc") return true;
        if (r.size() >= 2 && (r[0] == 'r' || r[0] == 'x')) {
            for (size_t i = 1; i < r.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(r[i]))) return false;
            }
            int v = std::stoi(r.substr(1));
            if (r[0] == 'r') return v >= 0 && v <= 15;
            return v >= 0 && v <= 30;
        }
        return false;
    }

    static int registerSizeBits(const std::string& raw) {
        std::string r = toLower(stripSigils(raw));
        if (r == "sp" || r == "lr" || r == "pc") return 64;
        if (!r.empty() && r[0] == 'x') return 64;
        return 32;
    }

    static int parseInt(const std::string& raw) {
        std::string t = trim(stripSigils(raw));
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

    static bool parseMemoryOperand(const std::string& op, AssemblyMemoryOperand& out) {
        if (op.size() < 2 || op.front() != '[' || op.back() != ']') return false;
        std::string inner = trim(op.substr(1, op.size() - 2)); // r0, #4  OR r0, r1, LSL #2
        std::vector<std::string> parts = splitOperands(inner);
        if (parts.empty()) return false;
        out.baseRegister = stripSigils(trim(parts[0]));
        if (parts.size() >= 2) {
            std::string p1 = trim(parts[1]);
            if (isRegisterName(p1)) out.indexRegister = stripSigils(p1);
            else out.offset = parseInt(p1);
        }
        if (parts.size() >= 3) {
            std::string p2 = toLower(trim(parts[2]));
            std::regex lslRx(R"(lsl\s*#?(\d+))");
            std::smatch m;
            if (std::regex_search(p2, m, lslRx)) out.scale = parseInt(m[1].str());
        }
        return true;
    }

    static AssemblyInstruction parseInstruction(const std::string& line) {
        size_t sp = line.find_first_of(" \t");
        std::string op = toLower(trim(sp == std::string::npos ? line : line.substr(0, sp)));
        std::string ops = sp == std::string::npos ? "" : trim(line.substr(sp + 1));
        AssemblyInstruction inst(op, splitOperands(ops));

        for (const auto& operand : inst.operands) {
            if (isRegisterName(operand)) {
                inst.registers.emplace_back(stripSigils(operand), registerSizeBits(operand));
                continue;
            }
            AssemblyMemoryOperand mem;
            if (parseMemoryOperand(operand, mem)) inst.memoryOperands.push_back(mem);
        }
        return inst;
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static bool isKnownOpcode(const std::string& op) {
        static const std::set<std::string> known = {
            "mov", "add", "sub", "ldr", "str", "b", "bl", "cmp",
            "beq", "bne", "bgt", "blt", "bge", "ble", "ret"
        };
        return known.count(op) > 0;
    }
};
