// Step 461: x86 Assembly Parser Tests (12 tests)

#include "ast/AssemblyNodes.h"
#include "ast/EnumNamespaceNodes.h"
#include "ast/X86AssemblyParser.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_intel_basic_instructions() {
    TEST(parse_intel_basic_instructions);
    std::string src = ".text\nmain:\n  mov rax, rbx\n  add rax, 1\n  ret\n";
    auto mod = X86AssemblyParser::parseX86(src, X86SyntaxMode::Intel);
    CHECK(mod->getChildren("functions").size() == 1, "expected one function");
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    CHECK(fn->name == "main", "expected function name main");
    CHECK(fn->getChildren("body").size() >= 3, "expected label + instructions");
    PASS();
}

void test_parse_att_basic_instructions() {
    TEST(parse_att_basic_instructions);
    std::string src = ".text\nmain:\n  movq %rax, %rbx\n  ret\n";
    auto mod = X86AssemblyParser::parseX86(src, X86SyntaxMode::ATT);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    bool sawMov = false;
    for (auto* n : fn->getChildren("body")) {
        if (n->conceptType == "AssemblyInstruction") {
            auto* i = static_cast<AssemblyInstruction*>(n);
            if (i->opcode == "movq") sawMov = true;
        }
    }
    CHECK(sawMov, "expected movq opcode");
    PASS();
}

void test_parse_labels_and_global_flags() {
    TEST(parse_labels_and_global_flags);
    std::string src = ".global main\n.text\nmain:\n  ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* lbl = static_cast<AssemblyLabel*>(fn->getChildren("body")[0]);
    CHECK(lbl->conceptType == "AssemblyLabel", "expected label node");
    CHECK(lbl->isGlobal, "expected global label");
    PASS();
}

void test_parse_directives_and_sections_as_namespaces() {
    TEST(parse_directives_and_sections_as_namespaces);
    std::string src = ".text\nmain:\n ret\n.data\nx: .word 1\n";
    auto mod = X86AssemblyParser::parseX86(src);
    int nsCount = 0;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType == "NamespaceDeclaration") ++nsCount;
    }
    CHECK(nsCount >= 2, "expected namespace-like sections");
    PASS();
}

void test_register_recognition_sizes() {
    TEST(register_recognition_sizes);
    std::string src = ".text\nmain:\n mov rax, eax\n mov ax, al\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    int has64 = 0, has32 = 0, has16 = 0, has8 = 0;
    for (auto* n : fn->getChildren("body")) {
        if (n->conceptType != "AssemblyInstruction") continue;
        auto* i = static_cast<AssemblyInstruction*>(n);
        for (const auto& r : i->registers) {
            if (r.sizeBits == 64) ++has64;
            if (r.sizeBits == 32) ++has32;
            if (r.sizeBits == 16) ++has16;
            if (r.sizeBits == 8) ++has8;
        }
    }
    CHECK(has64 > 0 && has32 > 0 && has16 > 0 && has8 > 0, "expected all register sizes");
    PASS();
}

void test_parse_intel_memory_base_offset() {
    TEST(parse_intel_memory_base_offset);
    std::string src = ".text\nmain:\n mov rax, [rbp-8]\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src, X86SyntaxMode::Intel);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected one memory operand");
    CHECK(i->memoryOperands[0].baseRegister == "rbp", "expected base rbp");
    CHECK(i->memoryOperands[0].offset == -8, "expected offset -8");
    PASS();
}

void test_parse_intel_memory_base_index_scale_offset() {
    TEST(parse_intel_memory_base_index_scale_offset);
    std::string src = ".text\nmain:\n mov rax, [rax+rbx*4+8]\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected one memory operand");
    CHECK(i->memoryOperands[0].baseRegister == "rax", "expected base rax");
    CHECK(i->memoryOperands[0].indexRegister == "rbx", "expected index rbx");
    CHECK(i->memoryOperands[0].scale == 4, "expected scale 4");
    CHECK(i->memoryOperands[0].offset == 8, "expected offset 8");
    PASS();
}

void test_parse_att_memory_addressing() {
    TEST(parse_att_memory_addressing);
    std::string src = ".text\nmain:\n movq -8(%rbp,%rbx,4), %rax\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src, X86SyntaxMode::ATT);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto* i = static_cast<AssemblyInstruction*>(fn->getChildren("body")[1]);
    CHECK(i->memoryOperands.size() == 1, "expected one memory operand");
    CHECK(i->memoryOperands[0].baseRegister == "rbp", "expected base rbp");
    CHECK(i->memoryOperands[0].indexRegister == "rbx", "expected index rbx");
    CHECK(i->memoryOperands[0].scale == 4, "expected scale 4");
    CHECK(i->memoryOperands[0].offset == -8, "expected offset -8");
    PASS();
}

void test_parse_ignores_comments() {
    TEST(parse_ignores_comments);
    std::string src = ".text ; section\nmain: ; label\n mov rax, rbx ; comment\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    CHECK(fn->getChildren("body").size() >= 3, "expected parsed nodes despite comments");
    PASS();
}

void test_known_opcode_set_covers_control_flow_and_stack() {
    TEST(known_opcode_set_covers_control_flow_and_stack);
    std::string src = ".text\nmain:\n push rax\n call foo\n jmp done\n pop rax\n ret\nfoo:\n ret\ndone:\n ret\n";
    auto res = X86AssemblyParser::parseX86WithDiagnostics(src);
    CHECK(res.diagnostics.empty(), "expected no unknown opcode warnings");
    PASS();
}

void test_unknown_opcode_emits_warning() {
    TEST(unknown_opcode_emits_warning);
    std::string src = ".text\nmain:\n frobnicate rax, rbx\n ret\n";
    auto res = X86AssemblyParser::parseX86WithDiagnostics(src);
    CHECK(!res.diagnostics.empty(), "expected warning diagnostics");
    CHECK(res.diagnostics[0].severity == "warning", "expected warning severity");
    PASS();
}

void test_parse_text_data_bss_sections() {
    TEST(parse_text_data_bss_sections);
    std::string src = ".text\nmain:\n ret\n.data\nx: .word 1\n.bss\ny: .byte 0\n";
    auto mod = X86AssemblyParser::parseX86(src);
    bool sawText = false, sawData = false;
    for (auto* s : mod->getChildren("statements")) {
        if (s->conceptType != "NamespaceDeclaration") continue;
        auto* ns = static_cast<NamespaceDeclaration*>(s);
        if (ns->name == ".text") sawText = true;
        if (ns->name == ".data") sawData = true;
    }
    CHECK(sawText, "expected .text namespace");
    CHECK(sawData, "expected .data namespace");
    PASS();
}

int main() {
    std::cout << "Step 461: x86 Assembly Parser Tests\n";

    test_parse_intel_basic_instructions();              // 1
    test_parse_att_basic_instructions();                // 2
    test_parse_labels_and_global_flags();               // 3
    test_parse_directives_and_sections_as_namespaces(); // 4
    test_register_recognition_sizes();                  // 5
    test_parse_intel_memory_base_offset();              // 6
    test_parse_intel_memory_base_index_scale_offset();  // 7
    test_parse_att_memory_addressing();                 // 8
    test_parse_ignores_comments();                      // 9
    test_known_opcode_set_covers_control_flow_and_stack(); // 10
    test_unknown_opcode_emits_warning();                // 11
    test_parse_text_data_bss_sections();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
