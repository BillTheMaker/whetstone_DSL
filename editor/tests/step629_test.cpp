// Step 629: Project context auto-injection (12 tests)

#include "AgentChatContextInjector.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_from_editor_without_active_buffer_uses_defaults() {
    TEST(from_editor_without_active_buffer_uses_defaults);
    AgentChatRuntimeSnapshot snapshot;
    snapshot.workspacePath = "/tmp/ws";
    auto block = AgentChatContextInjector::fromSnapshot(snapshot);
    CHECK(block.activeFile == "(no-active-buffer)", "active file mismatch");
    CHECK(block.language == "python", "language mismatch");
    CHECK(block.workspacePath == "/tmp/ws", "workspace mismatch");
    PASS();
}

void test_from_editor_uses_active_buffer_path_and_language() {
    TEST(from_editor_uses_active_buffer_path_and_language);
    AgentChatRuntimeSnapshot snapshot;
    snapshot.activeFile = "src/main.cpp";
    snapshot.language = "cpp";
    auto block = AgentChatContextInjector::fromSnapshot(snapshot);
    CHECK(block.activeFile == "src/main.cpp", "active file mismatch");
    CHECK(block.language == "cpp", "language mismatch");
    PASS();
}

void test_from_editor_uses_dot_workspace_when_empty() {
    TEST(from_editor_uses_dot_workspace_when_empty);
    AgentChatRuntimeSnapshot snapshot;
    auto block = AgentChatContextInjector::fromSnapshot(snapshot);
    CHECK(block.workspacePath == ".", "workspace fallback mismatch");
    PASS();
}

void test_from_editor_counts_zero_without_ast() {
    TEST(from_editor_counts_zero_without_ast);
    AgentChatRuntimeSnapshot snapshot;
    auto block = AgentChatContextInjector::fromSnapshot(snapshot);
    CHECK(block.astNodeCount == 0, "ast count should be zero");
    CHECK(block.openAnnotationCount == 0, "annotation count should be zero");
    PASS();
}

void test_prompt_text_contains_active_file_line() {
    TEST(prompt_text_contains_active_file_line);
    AgentChatContextBlock block;
    block.activeFile = "file.cpp";
    block.language = "cpp";
    block.workspacePath = "/tmp/ws";
    std::string text = AgentChatContextInjector::toPromptText(block);
    CHECK(text.find("activeFile: file.cpp") != std::string::npos, "activeFile line missing");
    PASS();
}

void test_prompt_text_contains_language_line() {
    TEST(prompt_text_contains_language_line);
    AgentChatContextBlock block;
    block.language = "rust";
    std::string text = AgentChatContextInjector::toPromptText(block);
    CHECK(text.find("language: rust") != std::string::npos, "language line missing");
    PASS();
}

void test_prompt_text_contains_ast_count_line() {
    TEST(prompt_text_contains_ast_count_line);
    AgentChatContextBlock block;
    block.astNodeCount = 17;
    std::string text = AgentChatContextInjector::toPromptText(block);
    CHECK(text.find("astNodeCount: 17") != std::string::npos, "ast count line missing");
    PASS();
}

void test_prompt_text_contains_annotation_count_line() {
    TEST(prompt_text_contains_annotation_count_line);
    AgentChatContextBlock block;
    block.openAnnotationCount = 3;
    std::string text = AgentChatContextInjector::toPromptText(block);
    CHECK(text.find("openAnnotationCount: 3") != std::string::npos, "annotation count line missing");
    PASS();
}

void test_prompt_text_contains_workspace_line() {
    TEST(prompt_text_contains_workspace_line);
    AgentChatContextBlock block;
    block.workspacePath = "/home/user/proj";
    std::string text = AgentChatContextInjector::toPromptText(block);
    CHECK(text.find("workspacePath: /home/user/proj") != std::string::npos, "workspace line missing");
    PASS();
}

void test_inject_sets_system_context() {
    TEST(inject_sets_system_context);
    AgentChatRuntimeSnapshot snapshot;
    AgentChatState chat;
    AgentChatContextInjector::inject(&chat, snapshot, "12:30");
    CHECK(!chat.systemContext.empty(), "systemContext should be set");
    PASS();
}

void test_inject_appends_assistant_message() {
    TEST(inject_appends_assistant_message);
    AgentChatRuntimeSnapshot snapshot;
    AgentChatState chat;
    AgentChatContextInjector::inject(&chat, snapshot, "12:31");
    CHECK(chat.messages.size() == 1, "one message expected");
    CHECK(chat.messages[0].role == AgentChatRole::Assistant, "role should be assistant");
    PASS();
}

void test_inject_enables_auto_scroll() {
    TEST(inject_enables_auto_scroll);
    AgentChatRuntimeSnapshot snapshot;
    AgentChatState chat;
    chat.autoScroll = false;
    AgentChatContextInjector::inject(&chat, snapshot, "12:32");
    CHECK(chat.autoScroll, "autoScroll should be enabled");
    PASS();
}

int main() {
    std::cout << "Step 629: Project context auto-injection\n";

    test_from_editor_without_active_buffer_uses_defaults(); // 1
    test_from_editor_uses_active_buffer_path_and_language();// 2
    test_from_editor_uses_dot_workspace_when_empty();       // 3
    test_from_editor_counts_zero_without_ast();             // 4
    test_prompt_text_contains_active_file_line();           // 5
    test_prompt_text_contains_language_line();              // 6
    test_prompt_text_contains_ast_count_line();             // 7
    test_prompt_text_contains_annotation_count_line();      // 8
    test_prompt_text_contains_workspace_line();             // 9
    test_inject_sets_system_context();                      // 10
    test_inject_appends_assistant_message();                // 11
    test_inject_enables_auto_scroll();                      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
