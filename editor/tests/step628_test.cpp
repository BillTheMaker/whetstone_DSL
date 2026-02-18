// Step 628: Phase 37a integration (8 tests)

#include "Phase37aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string baseCode() {
    return "def foo():\n    pass\n";
}

static std::string generatedCode() {
    return "def foo():\n    return 42\n";
}

void test_user_message_sent() {
    TEST(user_message_sent);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.sentUserMessage, "user message should be sent");
    PASS();
}

void test_tool_call_visualized() {
    TEST(tool_call_visualized);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.toolCallShown, "tool call should be shown");
    PASS();
}

void test_mutation_preview_shown() {
    TEST(mutation_preview_shown);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.previewShown, "preview should be shown");
    PASS();
}

void test_accept_path_marks_accepted() {
    TEST(accept_path_marks_accepted);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.accepted, "accept should be true");
    PASS();
}

void test_accept_path_updates_ast_code() {
    TEST(accept_path_updates_ast_code);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.astUpdated, "ast update should be true");
    CHECK(result.activeCode == generatedCode(), "active code should be updated");
    PASS();
}

void test_active_code_changes_from_before_to_after() {
    TEST(active_code_changes_from_before_to_after);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.activeCode != baseCode(), "code should differ from before");
    PASS();
}

void test_flow_handles_empty_spec_without_crash() {
    TEST(flow_handles_empty_spec_without_crash);
    auto result = Phase37aIntegration::run("", baseCode(), generatedCode());
    CHECK(!result.sentUserMessage, "empty spec should not send user message");
    CHECK(result.toolCallShown, "tool call simulation should still run");
    PASS();
}

void test_flow_refreshes_code_view_output() {
    TEST(flow_refreshes_code_view_output);
    auto result = Phase37aIntegration::run("implement foo", baseCode(), generatedCode());
    CHECK(result.activeCode.find("return 42") != std::string::npos, "expected generated content");
    PASS();
}

int main() {
    std::cout << "Step 628: Phase 37a integration\n";

    test_user_message_sent();                     // 1
    test_tool_call_visualized();                 // 2
    test_mutation_preview_shown();               // 3
    test_accept_path_marks_accepted();           // 4
    test_accept_path_updates_ast_code();         // 5
    test_active_code_changes_from_before_to_after();// 6
    test_flow_handles_empty_spec_without_crash();// 7
    test_flow_refreshes_code_view_output();      // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
