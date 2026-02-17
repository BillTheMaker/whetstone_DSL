// Step 464: Assembly Annotation Mapping Tests (12 tests)

#include "ast/AssemblyAnnotationMapper.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Function makeX86FunctionSample() {
    Function fn("fn1", "main");
    auto* lbl = new AssemblyLabel("main", true);
    fn.addChild("body", lbl);
    auto* mov = new AssemblyInstruction("mov", {"rax", "rbx"});
    mov->registers.emplace_back("rax", 64);
    mov->registers.emplace_back("rbx", 64);
    fn.addChild("body", mov);
    auto* add = new AssemblyInstruction("add", {"rax", "1"});
    add->registers.emplace_back("rax", 64);
    fn.addChild("body", add);
    auto* ret = new AssemblyInstruction("ret", {});
    fn.addChild("body", ret);
    return fn;
}

void test_adds_exec_native_annotation() {
    TEST(adds_exec_native_annotation);
    auto fn = makeX86FunctionSample();
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    bool found = false;
    for (const auto& a : r.annotations) if (a == "@Exec(native)") found = true;
    CHECK(found, "missing @Exec(native)");
    PASS();
}

void test_adds_target_x86_annotation() {
    TEST(adds_target_x86_annotation);
    auto fn = makeX86FunctionSample();
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    bool found = false;
    for (const auto& a : r.annotations) if (a == "@Target(x86)") found = true;
    CHECK(found, "missing @Target(x86)");
    PASS();
}

void test_adds_risk_high_annotation() {
    TEST(adds_risk_high_annotation);
    auto fn = makeX86FunctionSample();
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    bool found = false;
    for (const auto& a : r.annotations) if (a == "@Risk(high)") found = true;
    CHECK(found, "missing @Risk(high)");
    PASS();
}

void test_align_directive_maps_to_align_annotation() {
    TEST(align_directive_maps_to_align_annotation);
    Function fn("fn2", "aligned");
    auto* align = new AssemblyDirective(AssemblyDirectiveType::Align, "16");
    fn.addChild("body", align);
    fn.addChild("body", new AssemblyInstruction("ret", {}));
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    bool found = false;
    for (const auto& a : r.annotations) if (a == "@Align(16)") found = true;
    CHECK(found, "missing @Align annotation");
    PASS();
}

void test_complexity_low_for_small_function() {
    TEST(complexity_low_for_small_function);
    Function fn("fn3", "small");
    fn.addChild("body", new AssemblyInstruction("ret", {}));
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    CHECK(r.complexity == "low", "expected low complexity");
    PASS();
}

void test_complexity_high_for_branch_dense_function() {
    TEST(complexity_high_for_branch_dense_function);
    Function fn("fn4", "complex");
    for (int i = 0; i < 10; ++i) {
        auto* j = new AssemblyInstruction("jmp", {"L" + std::to_string(i)});
        fn.addChild("body", j);
    }
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    CHECK(r.complexity == "high", "expected high complexity");
    PASS();
}

void test_register_pressure_count_tracks_unique_registers() {
    TEST(register_pressure_count_tracks_unique_registers);
    Function fn("fn5", "regs");
    auto* i = new AssemblyInstruction("mov", {"rax", "rbx"});
    i->registers.emplace_back("rax", 64);
    i->registers.emplace_back("rbx", 64);
    fn.addChild("body", i);
    auto* j = new AssemblyInstruction("add", {"rcx", "rdx"});
    j->registers.emplace_back("rcx", 64);
    j->registers.emplace_back("rdx", 64);
    fn.addChild("body", j);
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    CHECK(r.registerPressure == 4, "expected 4 unique registers");
    PASS();
}

void test_branch_count_detected() {
    TEST(branch_count_detected);
    Function fn("fn6", "branches");
    fn.addChild("body", new AssemblyInstruction("jmp", {"L1"}));
    fn.addChild("body", new AssemblyInstruction("call", {"helper"}));
    fn.addChild("body", new AssemblyInstruction("ret", {}));
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    CHECK(r.branchCount == 3, "expected branch count 3");
    PASS();
}

void test_detects_arm_target_from_registers() {
    TEST(detects_arm_target_from_registers);
    Function fn("fn7", "armfn");
    auto* i = new AssemblyInstruction("mov", {"x0", "x1"});
    i->registers.emplace_back("x0", 64);
    i->registers.emplace_back("x1", 64);
    fn.addChild("body", i);
    auto r = AssemblyAnnotationMapper::analyzeFunction(fn);
    bool found = false;
    for (const auto& a : r.annotations) if (a == "@Target(arm)") found = true;
    CHECK(found, "expected @Target(arm)");
    PASS();
}

void test_lower_c_to_x86_assembly_contains_signature_name() {
    TEST(lower_c_to_x86_assembly_contains_signature_name);
    auto asmCode = AssemblyAnnotationMapper::lowerCToAssembly("int add(int a, int b)", "x86");
    CHECK(asmCode.find("add:") != std::string::npos, "missing function label");
    CHECK(asmCode.find(".global add") != std::string::npos, "missing global directive");
    PASS();
}

void test_lower_c_to_arm_assembly_uses_arm_comment_prefix_style() {
    TEST(lower_c_to_arm_assembly_uses_arm_comment_prefix_style);
    auto asmCode = AssemblyAnnotationMapper::lowerCToAssembly("int f()", "arm");
    CHECK(asmCode.find("@ lowered from C function") != std::string::npos, "missing arm-style comment");
    PASS();
}

void test_lift_assembly_to_c_generates_stub() {
    TEST(lift_assembly_to_c_generates_stub);
    auto fn = makeX86FunctionSample();
    auto c = AssemblyAnnotationMapper::liftAssemblyToC(fn);
    CHECK(c.find("int main()") != std::string::npos, "missing function signature");
    CHECK(c.find("return 0;") != std::string::npos, "missing return stub");
    PASS();
}

int main() {
    std::cout << "Step 464: Assembly Annotation Mapping Tests\n";

    test_adds_exec_native_annotation();                 // 1
    test_adds_target_x86_annotation();                  // 2
    test_adds_risk_high_annotation();                   // 3
    test_align_directive_maps_to_align_annotation();    // 4
    test_complexity_low_for_small_function();           // 5
    test_complexity_high_for_branch_dense_function();   // 6
    test_register_pressure_count_tracks_unique_registers(); // 7
    test_branch_count_detected();                       // 8
    test_detects_arm_target_from_registers();           // 9
    test_lower_c_to_x86_assembly_contains_signature_name(); // 10
    test_lower_c_to_arm_assembly_uses_arm_comment_prefix_style(); // 11
    test_lift_assembly_to_c_generates_stub();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
