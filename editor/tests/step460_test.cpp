// Step 460: Assembly AST Nodes Tests (12 tests)

#include "ast/AssemblyNodes.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_instruction_node_fields() {
    TEST(instruction_node_fields);
    AssemblyInstruction ins("mov", {"rax", "rbx"});
    CHECK(ins.conceptType == "AssemblyInstruction", "wrong conceptType");
    CHECK(ins.opcode == "mov", "wrong opcode");
    CHECK(ins.operands.size() == 2, "wrong operand count");
    PASS();
}

void test_label_node_fields() {
    TEST(label_node_fields);
    AssemblyLabel label("main", true);
    CHECK(label.conceptType == "AssemblyLabel", "wrong conceptType");
    CHECK(label.name == "main", "wrong label name");
    CHECK(label.isGlobal, "expected global label");
    PASS();
}

void test_directive_node_fields() {
    TEST(directive_node_fields);
    AssemblyDirective d(AssemblyDirectiveType::Text, "");
    CHECK(d.conceptType == "AssemblyDirective", "wrong conceptType");
    CHECK(d.type == AssemblyDirectiveType::Text, "wrong directive type");
    PASS();
}

void test_register_node_fields() {
    TEST(register_node_fields);
    AssemblyRegister r("rax", 64);
    CHECK(r.conceptType == "AssemblyRegister", "wrong conceptType");
    CHECK(r.name == "rax", "wrong register name");
    CHECK(r.sizeBits == 64, "wrong register size");
    PASS();
}

void test_memory_operand_fields() {
    TEST(memory_operand_fields);
    AssemblyMemoryOperand m;
    m.baseRegister = "rbp";
    m.offset = -8;
    m.indexRegister = "rbx";
    m.scale = 4;
    CHECK(m.conceptType == "AssemblyMemoryOperand", "wrong conceptType");
    CHECK(m.baseRegister == "rbp", "wrong base register");
    CHECK(m.offset == -8, "wrong offset");
    CHECK(m.scale == 4, "wrong scale");
    PASS();
}

void test_directive_type_string_roundtrip() {
    TEST(directive_type_string_roundtrip);
    auto s = directiveTypeToString(AssemblyDirectiveType::Global);
    auto t = directiveTypeFromString(s);
    CHECK(s == ".global", "wrong directive string");
    CHECK(t == AssemblyDirectiveType::Global, "wrong directive parse");
    PASS();
}

void test_register_json_roundtrip() {
    TEST(register_json_roundtrip);
    AssemblyRegister r("eax", 32);
    auto j = toJson(r);
    auto out = registerFromJson(j);
    CHECK(out.name == "eax", "wrong register from json");
    CHECK(out.sizeBits == 32, "wrong register bits from json");
    PASS();
}

void test_memory_operand_json_roundtrip() {
    TEST(memory_operand_json_roundtrip);
    AssemblyMemoryOperand m;
    m.baseRegister = "rax";
    m.offset = 8;
    m.indexRegister = "rcx";
    m.scale = 2;
    auto j = toJson(m);
    auto out = memoryOperandFromJson(j);
    CHECK(out.baseRegister == "rax", "wrong base register from json");
    CHECK(out.offset == 8, "wrong offset from json");
    CHECK(out.indexRegister == "rcx", "wrong index register from json");
    CHECK(out.scale == 2, "wrong scale from json");
    PASS();
}

void test_instruction_json_roundtrip_with_structured_operands() {
    TEST(instruction_json_roundtrip_with_structured_operands);
    AssemblyInstruction i("add", {"rax", "[rbp-8]"});
    i.registers.push_back(AssemblyRegister("rax", 64));
    AssemblyMemoryOperand m;
    m.baseRegister = "rbp";
    m.offset = -8;
    i.memoryOperands.push_back(m);
    auto j = toJson(i);
    auto out = instructionFromJson(j);
    CHECK(out.opcode == "add", "wrong opcode from json");
    CHECK(out.operands.size() == 2, "wrong operand count from json");
    CHECK(out.registers.size() == 1, "wrong register count from json");
    CHECK(out.memoryOperands.size() == 1, "wrong mem operand count from json");
    PASS();
}

void test_label_json_roundtrip() {
    TEST(label_json_roundtrip);
    AssemblyLabel l("loop_start", false);
    auto j = toJson(l);
    auto out = labelFromJson(j);
    CHECK(out.name == "loop_start", "wrong label from json");
    CHECK(!out.isGlobal, "wrong global flag from json");
    PASS();
}

void test_directive_json_roundtrip() {
    TEST(directive_json_roundtrip);
    AssemblyDirective d(AssemblyDirectiveType::Align, "16");
    auto j = toJson(d);
    auto out = directiveFromJson(j);
    CHECK(out.type == AssemblyDirectiveType::Align, "wrong directive type from json");
    CHECK(out.value == "16", "wrong directive value from json");
    PASS();
}

void test_unknown_directive_parses_to_unknown() {
    TEST(unknown_directive_parses_to_unknown);
    auto t = directiveTypeFromString(".bogus");
    CHECK(t == AssemblyDirectiveType::Unknown, "expected unknown directive type");
    PASS();
}

int main() {
    std::cout << "Step 460: Assembly AST Nodes Tests\n";

    test_instruction_node_fields();                    // 1
    test_label_node_fields();                          // 2
    test_directive_node_fields();                      // 3
    test_register_node_fields();                       // 4
    test_memory_operand_fields();                      // 5
    test_directive_type_string_roundtrip();            // 6
    test_register_json_roundtrip();                    // 7
    test_memory_operand_json_roundtrip();              // 8
    test_instruction_json_roundtrip_with_structured_operands(); // 9
    test_label_json_roundtrip();                       // 10
    test_directive_json_roundtrip();                   // 11
    test_unknown_directive_parses_to_unknown();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
