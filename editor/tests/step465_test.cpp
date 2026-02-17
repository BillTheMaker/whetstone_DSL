// Step 465: Phase 22a Integration Tests (8 tests)

#include "ast/ArmAssemblyParser.h"
#include "ast/AssemblyAnnotationMapper.h"
#include "ast/AssemblyGenerator.h"
#include "ast/X86AssemblyParser.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_realistic_x86_to_valid_ast() {
    TEST(parse_realistic_x86_to_valid_ast);
    std::string src =
        ".global main\n"
        ".text\n"
        "main:\n"
        "  push rbp\n"
        "  mov rbp, rsp\n"
        "  add rax, 1\n"
        "  pop rbp\n"
        "  ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    CHECK(mod->getChildren("functions").size() == 1, "expected one function");
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    CHECK(fn->getChildren("body").size() >= 6, "expected label + instructions");
    PASS();
}

void test_parse_realistic_arm_to_valid_ast() {
    TEST(parse_realistic_arm_to_valid_ast);
    std::string src =
        ".global main\n"
        ".text\n"
        "main:\n"
        "  MOV x0, x1\n"
        "  ADD x0, x0, #1\n"
        "  RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    CHECK(mod->getChildren("functions").size() == 1, "expected one function");
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    CHECK(fn->getChildren("body").size() >= 4, "expected label + instructions");
    PASS();
}

void test_cross_arch_x86_to_arm_simple_translation() {
    TEST(cross_arch_x86_to_arm_simple_translation);
    std::string src = ".text\nmain:\n mov rax, rbx\n add rax, 1\n jmp done\n ret\ndone:\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    std::vector<AssemblyInstruction> insts;
    for (auto* n : fn->getChildren("body")) {
        if (n->conceptType == "AssemblyInstruction")
            insts.push_back(*static_cast<AssemblyInstruction*>(n));
    }
    auto mapped = AssemblyGenerator::translateX86ToArmSimple(insts);
    CHECK(mapped.size() >= 3, "expected mapped instructions");
    CHECK(mapped[0].opcode == "mov", "mov mapping failed");
    CHECK(mapped[2].opcode == "b", "jmp should map to b");
    PASS();
}

void test_assembly_to_c_lifting_produces_stub() {
    TEST(assembly_to_c_lifting_produces_stub);
    std::string src = ".text\nmain:\n mov rax, rbx\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto c = AssemblyAnnotationMapper::liftAssemblyToC(*fn);
    CHECK(c.find("int main()") != std::string::npos, "missing lifted function");
    CHECK(c.find("return 0;") != std::string::npos, "missing return in lifted stub");
    PASS();
}

void test_c_to_assembly_lowering_annotations_and_shape() {
    TEST(c_to_assembly_lowering_annotations_and_shape);
    auto x86 = AssemblyAnnotationMapper::lowerCToAssembly("int add(int a, int b)", "x86");
    auto arm = AssemblyAnnotationMapper::lowerCToAssembly("int add(int a, int b)", "arm");
    CHECK(x86.find(".global add") != std::string::npos, "missing x86 global");
    CHECK(arm.find(".global add") != std::string::npos, "missing arm global");
    CHECK(arm.find("@ lowered from C function") != std::string::npos, "missing arm comment style");
    PASS();
}

void test_annotations_meaningful_for_assembly() {
    TEST(annotations_meaningful_for_assembly);
    std::string src = ".text\nmain:\n mov rax, rbx\n add rax, 1\n ret\n";
    auto mod = X86AssemblyParser::parseX86(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto report = AssemblyAnnotationMapper::analyzeFunction(*fn);
    bool hasExec = false, hasRisk = false, hasComplexity = false;
    for (const auto& a : report.annotations) {
        if (a == "@Exec(native)") hasExec = true;
        if (a == "@Risk(high)") hasRisk = true;
        if (a.find("@Complexity(") != std::string::npos) hasComplexity = true;
    }
    CHECK(hasExec && hasRisk && hasComplexity, "expected core assembly annotations");
    PASS();
}

void test_end_to_end_parse_annotate_generate_arm() {
    TEST(end_to_end_parse_annotate_generate_arm);
    std::string src = ".text\nmain:\n MOV r0, r1\n RET\n";
    auto mod = ArmAssemblyParser::parseArm(src);
    auto* fn = static_cast<Function*>(mod->getChildren("functions")[0]);
    auto report = AssemblyAnnotationMapper::analyzeFunction(*fn);
    auto out = AssemblyGenerator::generateArm(*mod);
    CHECK(!report.annotations.empty(), "expected annotations");
    CHECK(out.find("main:") != std::string::npos, "expected generated arm label");
    CHECK(out.find("mov r0, r1") != std::string::npos, "expected generated arm instruction");
    PASS();
}

void test_assembly_ast_node_roundtrip_serialization() {
    TEST(assembly_ast_node_roundtrip_serialization);
    AssemblyInstruction i("add", {"r0", "r1"});
    i.registers.emplace_back("r0", 32);
    i.registers.emplace_back("r1", 32);
    AssemblyMemoryOperand m;
    m.baseRegister = "r2";
    m.offset = 4;
    i.memoryOperands.push_back(m);
    auto j = toJson(i);
    auto out = instructionFromJson(j);
    CHECK(out.opcode == "add", "opcode roundtrip failed");
    CHECK(out.operands.size() == 2, "operand roundtrip failed");
    CHECK(out.registers.size() == 2, "register roundtrip failed");
    CHECK(out.memoryOperands.size() == 1, "memory roundtrip failed");
    PASS();
}

int main() {
    std::cout << "Step 465: Phase 22a Integration Tests\n";

    test_parse_realistic_x86_to_valid_ast();              // 1
    test_parse_realistic_arm_to_valid_ast();              // 2
    test_cross_arch_x86_to_arm_simple_translation();      // 3
    test_assembly_to_c_lifting_produces_stub();           // 4
    test_c_to_assembly_lowering_annotations_and_shape();  // 5
    test_annotations_meaningful_for_assembly();           // 6
    test_end_to_end_parse_annotate_generate_arm();        // 7
    test_assembly_ast_node_roundtrip_serialization();     // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
