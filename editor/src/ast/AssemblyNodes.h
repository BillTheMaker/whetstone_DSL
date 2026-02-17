#pragma once

// Step 460: Assembly AST Nodes
// Node types for assembly instructions, labels, directives, registers, and
// memory operands. These can be attached under Function nodes as statement-like
// children for assembly-backed function bodies.

#include "ASTNode.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

enum class AssemblyDirectiveType {
    Data,
    Text,
    Global,
    Section,
    Byte,
    Word,
    Align,
    Unknown
};

inline std::string directiveTypeToString(AssemblyDirectiveType t) {
    switch (t) {
        case AssemblyDirectiveType::Data: return ".data";
        case AssemblyDirectiveType::Text: return ".text";
        case AssemblyDirectiveType::Global: return ".global";
        case AssemblyDirectiveType::Section: return ".section";
        case AssemblyDirectiveType::Byte: return ".byte";
        case AssemblyDirectiveType::Word: return ".word";
        case AssemblyDirectiveType::Align: return ".align";
        case AssemblyDirectiveType::Unknown: return ".unknown";
    }
    return ".unknown";
}

inline AssemblyDirectiveType directiveTypeFromString(const std::string& s) {
    if (s == ".data") return AssemblyDirectiveType::Data;
    if (s == ".text") return AssemblyDirectiveType::Text;
    if (s == ".global") return AssemblyDirectiveType::Global;
    if (s == ".section") return AssemblyDirectiveType::Section;
    if (s == ".byte") return AssemblyDirectiveType::Byte;
    if (s == ".word") return AssemblyDirectiveType::Word;
    if (s == ".align") return AssemblyDirectiveType::Align;
    return AssemblyDirectiveType::Unknown;
}

class AssemblyRegister : public ASTNode {
public:
    std::string name;     // rax, eax, x0, r0, etc.
    int sizeBits = 0;     // 8/16/32/64

    AssemblyRegister() { conceptType = "AssemblyRegister"; }
    AssemblyRegister(std::string n, int bits) : name(std::move(n)), sizeBits(bits) {
        conceptType = "AssemblyRegister";
    }
};

class AssemblyMemoryOperand : public ASTNode {
public:
    std::string baseRegister; // rbp
    int offset = 0;           // -8
    std::string indexRegister; // rbx
    int scale = 1;            // 1/2/4/8

    AssemblyMemoryOperand() { conceptType = "AssemblyMemoryOperand"; }
};

class AssemblyInstruction : public ASTNode {
public:
    std::string opcode; // mov/add/sub...
    std::vector<std::string> operands; // textual fallback operands
    // Optional structured operand pieces for richer addressing semantics.
    std::vector<AssemblyRegister> registers;
    std::vector<AssemblyMemoryOperand> memoryOperands;

    AssemblyInstruction() { conceptType = "AssemblyInstruction"; }
    AssemblyInstruction(std::string op, std::vector<std::string> ops)
        : opcode(std::move(op)), operands(std::move(ops)) {
        conceptType = "AssemblyInstruction";
    }
};

class AssemblyLabel : public ASTNode {
public:
    std::string name;
    bool isGlobal = false;

    AssemblyLabel() { conceptType = "AssemblyLabel"; }
    AssemblyLabel(std::string n, bool global) : name(std::move(n)), isGlobal(global) {
        conceptType = "AssemblyLabel";
    }
};

class AssemblyDirective : public ASTNode {
public:
    AssemblyDirectiveType type = AssemblyDirectiveType::Unknown;
    std::string value; // .section name, .word literal value, etc.

    AssemblyDirective() { conceptType = "AssemblyDirective"; }
    AssemblyDirective(AssemblyDirectiveType t, std::string v)
        : type(t), value(std::move(v)) {
        conceptType = "AssemblyDirective";
    }
};

inline json toJson(const AssemblyRegister& r) {
    return {
        {"concept", r.conceptType},
        {"name", r.name},
        {"sizeBits", r.sizeBits}
    };
}

inline AssemblyRegister registerFromJson(const json& j) {
    AssemblyRegister r;
    r.name = j.value("name", "");
    r.sizeBits = j.value("sizeBits", 0);
    return r;
}

inline json toJson(const AssemblyMemoryOperand& m) {
    return {
        {"concept", m.conceptType},
        {"baseRegister", m.baseRegister},
        {"offset", m.offset},
        {"indexRegister", m.indexRegister},
        {"scale", m.scale}
    };
}

inline AssemblyMemoryOperand memoryOperandFromJson(const json& j) {
    AssemblyMemoryOperand m;
    m.baseRegister = j.value("baseRegister", "");
    m.offset = j.value("offset", 0);
    m.indexRegister = j.value("indexRegister", "");
    m.scale = j.value("scale", 1);
    return m;
}

inline json toJson(const AssemblyInstruction& i) {
    json regs = json::array();
    for (const auto& r : i.registers) regs.push_back(toJson(r));
    json mems = json::array();
    for (const auto& m : i.memoryOperands) mems.push_back(toJson(m));
    return {
        {"concept", i.conceptType},
        {"opcode", i.opcode},
        {"operands", i.operands},
        {"registers", regs},
        {"memoryOperands", mems}
    };
}

inline AssemblyInstruction instructionFromJson(const json& j) {
    AssemblyInstruction i;
    i.opcode = j.value("opcode", "");
    i.operands = j.value("operands", std::vector<std::string>{});
    if (j.contains("registers")) {
        for (const auto& r : j["registers"]) i.registers.push_back(registerFromJson(r));
    }
    if (j.contains("memoryOperands")) {
        for (const auto& m : j["memoryOperands"]) i.memoryOperands.push_back(memoryOperandFromJson(m));
    }
    return i;
}

inline json toJson(const AssemblyLabel& l) {
    return {
        {"concept", l.conceptType},
        {"name", l.name},
        {"isGlobal", l.isGlobal}
    };
}

inline AssemblyLabel labelFromJson(const json& j) {
    AssemblyLabel l;
    l.name = j.value("name", "");
    l.isGlobal = j.value("isGlobal", false);
    return l;
}

inline json toJson(const AssemblyDirective& d) {
    return {
        {"concept", d.conceptType},
        {"type", directiveTypeToString(d.type)},
        {"value", d.value}
    };
}

inline AssemblyDirective directiveFromJson(const json& j) {
    AssemblyDirective d;
    d.type = directiveTypeFromString(j.value("type", ".unknown"));
    d.value = j.value("value", "");
    return d;
}
