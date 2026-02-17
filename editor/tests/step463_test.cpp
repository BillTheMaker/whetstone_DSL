// Step 463: Assembly Generator Tests (12 tests)

#include "ast/ArmAssemblyParser.h"
#include "ast/AssemblyGenerator.h"
#include "ast/X86AssemblyParser.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_generate_x86_intel_contains_expected_instructions() {
    TEST(generate_x86_intel_contains_expected_instructions);
    auto mod = X86AssemblyParser::parseX86(".text\nmain:\n mov rax, rbx\n ret\n");
    auto out = AssemblyGenerator::generateX86(*mod, X86OutputSyntax::Intel);
    CHECK(out.find("main:") != std::string::npos, "missing label");
    CHECK(out.find("mov rax, rbx") != std::string::npos, "missing intel mov");
    PASS();
}

void test_generate_x86_att_adds_register_sigils() {
    TEST(generate_x86_att_adds_register_sigils);
    auto mod = X86AssemblyParser::parseX86(".text\nmain:\n mov rax, rbx\n ret\n");
    auto out = AssemblyGenerator::generateX86(*mod, X86OutputSyntax::ATT);
    CHECK(out.find("mov %rax, %rbx") != std::string::npos, "missing AT&T register sigils");
    PASS();
}

void test_generate_x86_preserves_directives() {
    TEST(generate_x86_preserves_directives);
    auto mod = X86AssemblyParser::parseX86(".global main\n.text\nmain:\n ret\n");
    auto out = AssemblyGenerator::generateX86(*mod);
    CHECK(out.find(".global main") != std::string::npos, "missing .global");
    CHECK(out.find(".text") != std::string::npos, "missing .text");
    PASS();
}

void test_generate_arm_contains_lowercase_opcodes() {
    TEST(generate_arm_contains_lowercase_opcodes);
    auto mod = ArmAssemblyParser::parseArm(".text\nmain:\n MOV r0, r1\n RET\n");
    auto out = AssemblyGenerator::generateArm(*mod);
    CHECK(out.find("mov r0, r1") != std::string::npos, "missing arm mov");
    CHECK(out.find("ret") != std::string::npos, "missing arm ret");
    PASS();
}

void test_generate_arm_preserves_sections() {
    TEST(generate_arm_preserves_sections);
    auto mod = ArmAssemblyParser::parseArm(".text\nmain:\n RET\n.data\nv: .word 1\n");
    auto out = AssemblyGenerator::generateArm(*mod);
    CHECK(out.find(".text") != std::string::npos, "missing .text");
    CHECK(out.find(".data") != std::string::npos, "missing .data");
    PASS();
}

void test_translate_x86_to_arm_maps_mov_add_sub_cmp() {
    TEST(translate_x86_to_arm_maps_mov_add_sub_cmp);
    std::vector<AssemblyInstruction> x86 = {
        AssemblyInstruction("mov", {"rax", "rbx"}),
        AssemblyInstruction("add", {"rax", "1"}),
        AssemblyInstruction("sub", {"rax", "2"}),
        AssemblyInstruction("cmp", {"rax", "rbx"})
    };
    auto arm = AssemblyGenerator::translateX86ToArmSimple(x86);
    CHECK(arm.size() == 4, "wrong mapped size");
    CHECK(arm[0].opcode == "mov", "mov mapping failed");
    CHECK(arm[1].opcode == "add", "add mapping failed");
    CHECK(arm[2].opcode == "sub", "sub mapping failed");
    CHECK(arm[3].opcode == "cmp", "cmp mapping failed");
    PASS();
}

void test_translate_x86_to_arm_maps_jmp_and_call() {
    TEST(translate_x86_to_arm_maps_jmp_and_call);
    std::vector<AssemblyInstruction> x86 = {
        AssemblyInstruction("jmp", {"label"}),
        AssemblyInstruction("call", {"helper"})
    };
    auto arm = AssemblyGenerator::translateX86ToArmSimple(x86);
    CHECK(arm[0].opcode == "b", "jmp should map to b");
    CHECK(arm[1].opcode == "bl", "call should map to bl");
    PASS();
}

void test_translate_x86_to_arm_maps_ret() {
    TEST(translate_x86_to_arm_maps_ret);
    std::vector<AssemblyInstruction> x86 = { AssemblyInstruction("ret", {}) };
    auto arm = AssemblyGenerator::translateX86ToArmSimple(x86);
    CHECK(arm[0].opcode == "ret", "ret mapping failed");
    PASS();
}

void test_translate_unknown_opcode_passthrough() {
    TEST(translate_unknown_opcode_passthrough);
    std::vector<AssemblyInstruction> x86 = { AssemblyInstruction("nop", {}) };
    auto arm = AssemblyGenerator::translateX86ToArmSimple(x86);
    CHECK(arm[0].opcode == "nop", "unknown opcode should pass through");
    PASS();
}

void test_comment_prefix_x86() {
    TEST(comment_prefix_x86);
    CHECK(AssemblyGenerator::commentPrefix("x86") == "; ", "wrong x86 comment prefix");
    PASS();
}

void test_comment_prefix_arm() {
    TEST(comment_prefix_arm);
    CHECK(AssemblyGenerator::commentPrefix("arm") == "@ ", "wrong arm comment prefix");
    PASS();
}

void test_roundtrip_parse_generate_parse_x86() {
    TEST(roundtrip_parse_generate_parse_x86);
    std::string src = ".text\nmain:\n mov rax, rbx\n add rax, 1\n ret\n";
    auto mod1 = X86AssemblyParser::parseX86(src);
    auto gen = AssemblyGenerator::generateX86(*mod1);
    auto mod2 = X86AssemblyParser::parseX86(gen);
    CHECK(mod2->getChildren("functions").size() == 1, "roundtrip function count mismatch");
    auto* fn = static_cast<Function*>(mod2->getChildren("functions")[0]);
    CHECK(fn->getChildren("body").size() >= 4, "roundtrip body too small");
    PASS();
}

int main() {
    std::cout << "Step 463: Assembly Generator Tests\n";

    test_generate_x86_intel_contains_expected_instructions(); // 1
    test_generate_x86_att_adds_register_sigils();             // 2
    test_generate_x86_preserves_directives();                 // 3
    test_generate_arm_contains_lowercase_opcodes();           // 4
    test_generate_arm_preserves_sections();                   // 5
    test_translate_x86_to_arm_maps_mov_add_sub_cmp();         // 6
    test_translate_x86_to_arm_maps_jmp_and_call();            // 7
    test_translate_x86_to_arm_maps_ret();                     // 8
    test_translate_unknown_opcode_passthrough();              // 9
    test_comment_prefix_x86();                                // 10
    test_comment_prefix_arm();                                // 11
    test_roundtrip_parse_generate_parse_x86();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
