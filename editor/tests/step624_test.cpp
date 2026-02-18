// Step 624: Agent Chat Panel model (12 tests)

#include "AgentChatPanelModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_default_state_is_empty() {
    TEST(default_state_is_empty);
    AgentChatState state;
    CHECK(state.messages.empty(), "messages should be empty");
    CHECK(state.draftInput.empty(), "draft should be empty");
    PASS();
}

void test_send_user_message_adds_history_entry() {
    TEST(send_user_message_adds_history_entry);
    AgentChatState state;
    state.draftInput = "build me a parser";
    CHECK(AgentChatPanelModel::sendUserMessage(&state, "10:00"), "send should succeed");
    CHECK(state.messages.size() == 1, "expected one message");
    CHECK(state.messages[0].role == AgentChatRole::User, "role mismatch");
    PASS();
}

void test_send_user_message_clears_draft() {
    TEST(send_user_message_clears_draft);
    AgentChatState state;
    state.draftInput = "hello";
    (void)AgentChatPanelModel::sendUserMessage(&state, "10:01");
    CHECK(state.draftInput.empty(), "draft should be cleared");
    PASS();
}

void test_send_user_message_trims_whitespace() {
    TEST(send_user_message_trims_whitespace);
    AgentChatState state;
    state.draftInput = "   hello agent   ";
    (void)AgentChatPanelModel::sendUserMessage(&state, "10:02");
    CHECK(state.messages[0].content == "hello agent", "trim failed");
    PASS();
}

void test_send_rejects_empty_draft() {
    TEST(send_rejects_empty_draft);
    AgentChatState state;
    state.draftInput = "";
    CHECK(!AgentChatPanelModel::sendUserMessage(&state, "10:03"), "empty draft should fail");
    CHECK(state.messages.empty(), "message should not be added");
    PASS();
}

void test_send_rejects_whitespace_only_draft() {
    TEST(send_rejects_whitespace_only_draft);
    AgentChatState state;
    state.draftInput = " \n\t ";
    CHECK(!AgentChatPanelModel::sendUserMessage(&state, "10:04"), "whitespace-only draft should fail");
    PASS();
}

void test_add_assistant_message_appends() {
    TEST(add_assistant_message_appends);
    AgentChatState state;
    AgentChatPanelModel::addAssistantMessage(&state, "Sure, running tools.", "10:05");
    CHECK(state.messages.size() == 1, "expected one message");
    CHECK(state.messages[0].role == AgentChatRole::Assistant, "role mismatch");
    PASS();
}

void test_add_tool_message_appends() {
    TEST(add_tool_message_appends);
    AgentChatState state;
    AgentChatPanelModel::addToolMessage(&state, "[TOOL] whetstone_mutate", "10:06");
    CHECK(state.messages.size() == 1, "expected one message");
    CHECK(state.messages[0].role == AgentChatRole::Tool, "role mismatch");
    PASS();
}

void test_message_timestamp_persists() {
    TEST(message_timestamp_persists);
    AgentChatState state;
    state.draftInput = "status";
    (void)AgentChatPanelModel::sendUserMessage(&state, "10:07");
    CHECK(state.messages[0].timestamp == "10:07", "timestamp mismatch");
    PASS();
}

void test_message_order_is_stable() {
    TEST(message_order_is_stable);
    AgentChatState state;
    state.draftInput = "one";
    (void)AgentChatPanelModel::sendUserMessage(&state, "10:08");
    AgentChatPanelModel::addAssistantMessage(&state, "two", "10:09");
    AgentChatPanelModel::addToolMessage(&state, "three", "10:10");
    CHECK(state.messages.size() == 3, "message count mismatch");
    CHECK(state.messages[0].content == "one", "index 0 mismatch");
    CHECK(state.messages[1].content == "two", "index 1 mismatch");
    CHECK(state.messages[2].content == "three", "index 2 mismatch");
    PASS();
}

void test_auto_scroll_enabled_on_append() {
    TEST(auto_scroll_enabled_on_append);
    AgentChatState state;
    state.autoScroll = false;
    AgentChatPanelModel::addAssistantMessage(&state, "reply", "10:11");
    CHECK(state.autoScroll, "autoScroll should be enabled");
    PASS();
}

void test_role_label_mapping() {
    TEST(role_label_mapping);
    CHECK(std::string(AgentChatPanelModel::roleLabel(AgentChatRole::User)) == "user", "user label mismatch");
    CHECK(std::string(AgentChatPanelModel::roleLabel(AgentChatRole::Assistant)) == "assistant", "assistant label mismatch");
    CHECK(std::string(AgentChatPanelModel::roleLabel(AgentChatRole::Tool)) == "tool", "tool label mismatch");
    PASS();
}

int main() {
    std::cout << "Step 624: Agent Chat Panel model\n";

    test_default_state_is_empty();            // 1
    test_send_user_message_adds_history_entry(); // 2
    test_send_user_message_clears_draft();    // 3
    test_send_user_message_trims_whitespace();// 4
    test_send_rejects_empty_draft();          // 5
    test_send_rejects_whitespace_only_draft();// 6
    test_add_assistant_message_appends();     // 7
    test_add_tool_message_appends();          // 8
    test_message_timestamp_persists();        // 9
    test_message_order_is_stable();           // 10
    test_auto_scroll_enabled_on_append();     // 11
    test_role_label_mapping();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
