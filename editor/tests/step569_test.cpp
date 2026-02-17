// Step 569: Disassembly + Register View Baseline (12 tests)

#include "DisassemblyRegisterViewModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static DisassemblyInstruction inst(std::uint64_t address, const std::string& opcode, const std::string& operands) {
    return {address, opcode, operands, false};
}

void test_set_instructions_success() {
    TEST(set_instructions_success);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1000, "mov", "rax, rbx"), inst(0x1004, "ret", "")}, &error), "set should succeed");
    CHECK(model.instructions().size() == 2, "instruction count mismatch");
    PASS();
}

void test_set_instructions_rejects_empty() {
    TEST(set_instructions_rejects_empty);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(!model.setInstructions({}, &error), "empty instruction list should fail");
    CHECK(error == "instruction_list_empty", "wrong error");
    PASS();
}

void test_set_instructions_rejects_invalid_address() {
    TEST(set_instructions_rejects_invalid_address);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(!model.setInstructions({inst(0, "mov", "rax,rbx")}, &error), "zero address should fail");
    CHECK(error == "instruction_address_invalid", "wrong error");
    PASS();
}

void test_set_instructions_rejects_missing_opcode() {
    TEST(set_instructions_rejects_missing_opcode);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(!model.setInstructions({inst(0x1000, "", "rax,rbx")}, &error), "missing opcode should fail");
    CHECK(error == "instruction_opcode_missing", "wrong error");
    PASS();
}

void test_instructions_are_sorted_by_address() {
    TEST(instructions_are_sorted_by_address);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1010, "ret", ""), inst(0x1000, "push", "rbp")}, &error), "set should succeed");
    CHECK(model.instructions()[0].address == 0x1000, "first instruction should be lowest address");
    CHECK(model.instructions()[1].address == 0x1010, "second instruction should be highest address");
    PASS();
}

void test_set_current_instruction_pointer_success() {
    TEST(set_current_instruction_pointer_success);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1000, "nop", ""), inst(0x1001, "ret", "")}, &error), "set should succeed");
    CHECK(model.setCurrentInstructionPointer(0x1001, &error), "set ip should succeed");
    CHECK(!model.instructions()[0].isCurrentInstruction, "first instruction should not be current");
    CHECK(model.instructions()[1].isCurrentInstruction, "second instruction should be current");
    PASS();
}

void test_set_current_instruction_pointer_rejects_unknown_address() {
    TEST(set_current_instruction_pointer_rejects_unknown_address);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1000, "nop", "")}, &error), "set should succeed");
    CHECK(!model.setCurrentInstructionPointer(0x2000, &error), "unknown ip should fail");
    CHECK(error == "instruction_pointer_not_found", "wrong error");
    PASS();
}

void test_window_around_address_returns_radius_slice() {
    TEST(window_around_address_returns_radius_slice);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1000, "a", ""), inst(0x1001, "b", ""), inst(0x1002, "c", ""), inst(0x1003, "d", "")}, &error), "set should succeed");
    const auto window = model.windowAroundAddress(0x1001, 1);
    CHECK(window.size() == 3, "window size mismatch");
    CHECK(window[0].address == 0x1000, "window start mismatch");
    CHECK(window[2].address == 0x1002, "window end mismatch");
    PASS();
}

void test_window_around_unknown_address_is_empty() {
    TEST(window_around_unknown_address_is_empty);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setInstructions({inst(0x1000, "a", "")}, &error), "set should succeed");
    const auto window = model.windowAroundAddress(0x9999, 1);
    CHECK(window.empty(), "window should be empty for unknown center");
    PASS();
}

void test_set_register_value_success() {
    TEST(set_register_value_success);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setRegisterValue("rax", 123, &error), "set register should succeed");
    CHECK(model.hasRegister("rax"), "register should exist");
    CHECK(model.registerValueOrZero("rax") == 123, "register value mismatch");
    PASS();
}

void test_set_register_value_rejects_empty_name() {
    TEST(set_register_value_rejects_empty_name);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(!model.setRegisterValue("", 123, &error), "empty register name should fail");
    CHECK(error == "register_name_missing", "wrong error");
    PASS();
}

void test_sorted_registers_order_by_name() {
    TEST(sorted_registers_order_by_name);
    DisassemblyRegisterViewModel model;
    std::string error;
    CHECK(model.setRegisterValue("rbx", 2, &error), "set rbx failed");
    CHECK(model.setRegisterValue("rax", 1, &error), "set rax failed");
    const auto regs = model.sortedRegisters();
    CHECK(regs.size() == 2, "register count mismatch");
    CHECK(regs[0].name == "rax", "register order mismatch");
    CHECK(regs[1].name == "rbx", "register order mismatch");
    PASS();
}

int main() {
    std::cout << "Step 569: Disassembly + Register View Baseline\n";

    test_set_instructions_success();                             // 1
    test_set_instructions_rejects_empty();                      // 2
    test_set_instructions_rejects_invalid_address();            // 3
    test_set_instructions_rejects_missing_opcode();             // 4
    test_instructions_are_sorted_by_address();                  // 5
    test_set_current_instruction_pointer_success();             // 6
    test_set_current_instruction_pointer_rejects_unknown_address();// 7
    test_window_around_address_returns_radius_slice();          // 8
    test_window_around_unknown_address_is_empty();              // 9
    test_set_register_value_success();                          // 10
    test_set_register_value_rejects_empty_name();               // 11
    test_sorted_registers_order_by_name();                      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
