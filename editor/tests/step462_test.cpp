// Step 462: ARM Assembly Parser Tests (12 tests)

#include "ast/ArmAssemblyParser.h"
#include "ast/AssemblyNodes.h"
#include "ast/EnumNamespaceNodes.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_arm_basic_instructions() {
    TEST(parse_arm_basic_instructions);
    std::string src = ".text\nmain:\n  MOV r0, r1\n  ADD r0, r0, #1\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    CHECK(mod->getChildren("functions").size() == 1, "expected one function");
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    CHECK(fn->name == "main", "expected main function");
    CHECK(fn->getChildren("body").size() >= 3, "expected label + instructions");
    PASS();
}

void test_parse_aarch64_registers() {
    TEST(parse_aarch64_registers);
    std::string src = ".text\nmain:\n  MOV x0, x1\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->registers.size() >= 2, "expected register extraction");
    CHECK(i->registers[0].sizeBits == 64, "expected 64-bit x register");
    PASS();
}

void test_parse_arm32_registers() {
    TEST(parse_arm32_registers);
    std::string src = ".text\nmain:\n  MOV r0, r1\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->registers.size() >= 2, "expected register extraction");
    CHECK(i->registers[0].sizeBits == 32, "expected 32-bit r register");
    PASS();
}

void test_parse_special_registers_sp_lr_pc() {
    TEST(parse_special_registers_sp_lr_pc);
    std::string src = ".text\nmain:\n  MOV sp, lr\n  MOV x0, pc\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    int special = 0;
    for (auto* n : fn->getChildren("body")) {
        if (n->conceptType != "AssemblyInstruction") continue;
        auto* i = static_cast<AssemblyInstruction*>(n);
        for (const auto& r : i->registers) {
            if (r.name == "sp" || r.name == "lr" || r.name == "pc") ++special;
        }
    }
    CHECK(special >= 2, "expected special register recognition");
    PASS();
}

void test_parse_memory_mode_base_only() {
    TEST(parse_memory_mode_base_only);
    std::string src = ".text\nmain:\n  LDR r0, [r1]\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected memory operand");
    CHECK(i->memoryOperands[0].baseRegister == "r1", "expected base r1");
    PASS();
}

void test_parse_memory_mode_base_plus_offset() {
    TEST(parse_memory_mode_base_plus_offset);
    std::string src = ".text\nmain:\n  LDR r0, [r1, #4]\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected memory operand");
    CHECK(i->memoryOperands[0].offset == 4, "expected offset 4");
    PASS();
}

void test_parse_memory_mode_index_shifted() {
    TEST(parse_memory_mode_index_shifted);
    std::string src = ".text\nmain:\n  STR x0, [x1, x2, LSL #2]\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected memory operand");
    CHECK(i->memoryOperands[0].indexRegister == "x2", "expected index x2");
    CHECK(i->memoryOperands[0].scale == 2, "expected scale 2");
    PASS();
}

void test_parse_directives_global_text_data_word_align() {
    TEST(parse_directives_global_text_data_word_align);
    std::string src = ".global main\n.text\nmain:\n  RET\n.data\nval: .word 1\n.align 4\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    int directives = 0;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == "AssemblyDirective") ++directives;
        if (s->conceptType == "NamespaceDeclaration") {
            auto* ns = static_cast<NamespaceDeclaration*>(s);
            directives += static_cast<int>(ns->getChildren("body").size());
        }
    }
    CHECK(directives >= 4, "expected directive parsing");
    PASS();
}

void test_parse_branch_and_call_opcodes() {
    TEST(parse_branch_and_call_opcodes);
    std::string src = ".text\nmain:\n  B loop\n  BL helper\n  CMP r0, r1\nloop:\n  RET\nhelper:\n  RET\n";
    auto res = ArmAssemblyParser::parseArmWithDiagnostics(src);
    CHECK(res.diagnostics.empty(), "expected known opcodes only");
    PASS();
}

void test_labels_and_global_flags() {
    TEST(labels_and_global_flags);
    std::string src = ".global main\n.text\nmain:\n  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* lbl = static_cast<AssemblyLabel*>(fn->getChildren("body")[0]);
    CHECK(lbl->isGlobal, "expected global label");
    PASS();
}

void test_section_namespaces_created() {
    TEST(section_namespaces_created);
    std::string src = ".text\nmain:\n RET\n.data\nx: .word 1\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    int nsCount = 0;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == "NamespaceDeclaration") ++nsCount;
    }
    CHECK(nsCount >= 2, "expected .text and .data namespaces");
    PASS();
}

void test_unknown_opcode_warning_emitted() {
    TEST(unknown_opcode_warning_emitted);
    std::string src = ".text\nmain:\n  FROB r0, r1\n  RET\n";
    auto res = ArmAssemblyParser::parseArmWithDiagnostics(src);
    CHECK(!res.diagnostics.empty(), "expected warning diagnostics");
    CHECK(res.diagnostics[0].severity == "warning", "expected warning severity");
    PASS();
}

int main() {
    std::cout << "Step 462: ARM Assembly Parser Tests\n";

    test_parse_arm_basic_instructions();                 // 1
    test_parse_aarch64_registers();                      // 2
    test_parse_arm32_registers();                        // 3
    test_parse_special_registers_sp_lr_pc();             // 4
    test_parse_memory_mode_base_only();                  // 5
    test_parse_memory_mode_base_plus_offset();           // 6
    test_parse_memory_mode_index_shifted();              // 7
    test_parse_directives_global_text_data_word_align(); // 8
    test_parse_branch_and_call_opcodes();                // 9
    test_labels_and_global_flags();                      // 10
    test_section_namespaces_created();                   // 11
    test_unknown_opcode_warning_emitted();               // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
