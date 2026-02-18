// Step 626: AST mutation preview (12 tests)

#include "AgentChatPanelModel.h"
#include "AgentMutationPreview.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string beforeCode() {
    return "def add(a, b):\n    return a + b\n";
}

static std::string afterCode() {
    return "def add_numbers(a, b):\n    return a + b\n";
}

void test_build_sets_preview_id() {
    TEST(build_sets_preview_id);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    CHECK(preview.previewId == "p1", "previewId mismatch");
    PASS();
}

void test_build_carries_mutation_json() {
    TEST(build_carries_mutation_json);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{\"type\":\"setProperty\"}", beforeCode(), afterCode());
    CHECK(preview.mutationJson.find("setProperty") != std::string::npos, "mutation json mismatch");
    PASS();
}

void test_build_sets_before_and_after_code() {
    TEST(build_sets_before_and_after_code);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    CHECK(preview.beforeCode == beforeCode(), "before code mismatch");
    CHECK(preview.afterCode == afterCode(), "after code mismatch");
    PASS();
}

void test_build_detects_changed_lines() {
    TEST(build_detects_changed_lines);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    CHECK(!preview.diff.beforeLines.empty(), "before diff should not be empty");
    CHECK(!preview.diff.afterLines.empty(), "after diff should not be empty");
    PASS();
}

void test_build_marks_has_changes_true_when_diff_exists() {
    TEST(build_marks_has_changes_true_when_diff_exists);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    CHECK(preview.hasChanges, "hasChanges should be true");
    PASS();
}

void test_build_marks_has_changes_false_when_equal() {
    TEST(build_marks_has_changes_false_when_equal);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), beforeCode());
    CHECK(!preview.hasChanges, "hasChanges should be false");
    PASS();
}

void test_before_lines_split_preserves_count() {
    TEST(before_lines_split_preserves_count);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    auto lines = AgentMutationPreviewBuilder::beforeLines(preview);
    CHECK(lines.size() >= 2, "expected at least 2 lines");
    PASS();
}

void test_after_lines_split_preserves_count() {
    TEST(after_lines_split_preserves_count);
    auto preview = AgentMutationPreviewBuilder::build("p1", "{}", beforeCode(), afterCode());
    auto lines = AgentMutationPreviewBuilder::afterLines(preview);
    CHECK(lines.size() >= 2, "expected at least 2 lines");
    PASS();
}

void test_chat_add_mutation_preview_appends_record() {
    TEST(chat_add_mutation_preview_appends_record);
    AgentChatState state;
    AgentChatPanelModel::addMutationPreview(&state, "preview-1", "{\"type\":\"setProperty\"}",
                                            beforeCode(), afterCode(), "11:10");
    CHECK(state.mutationPreviews.size() == 1, "preview count mismatch");
    CHECK(state.mutationPreviews[0].previewId == "preview-1", "preview id mismatch");
    PASS();
}

void test_chat_add_mutation_preview_appends_message() {
    TEST(chat_add_mutation_preview_appends_message);
    AgentChatState state;
    AgentChatPanelModel::addMutationPreview(&state, "preview-1", "{}", beforeCode(), afterCode(), "11:11");
    CHECK(state.messages.size() == 1, "message count mismatch");
    CHECK(state.messages[0].content.find("[PREVIEW] preview-1") != std::string::npos, "preview message missing");
    PASS();
}

void test_chat_preview_message_marks_change_state() {
    TEST(chat_preview_message_marks_change_state);
    AgentChatState state;
    AgentChatPanelModel::addMutationPreview(&state, "preview-1", "{}", beforeCode(), afterCode(), "11:12");
    CHECK(state.messages[0].content.find("(changes)") != std::string::npos, "expected changes marker");
    PASS();
}

void test_chat_preview_message_marks_no_change_state() {
    TEST(chat_preview_message_marks_no_change_state);
    AgentChatState state;
    AgentChatPanelModel::addMutationPreview(&state, "preview-2", "{}", beforeCode(), beforeCode(), "11:13");
    CHECK(state.messages[0].content.find("(no changes)") != std::string::npos, "expected no-changes marker");
    PASS();
}

int main() {
    std::cout << "Step 626: AST mutation preview\n";

    test_build_sets_preview_id();                            // 1
    test_build_carries_mutation_json();                      // 2
    test_build_sets_before_and_after_code();                 // 3
    test_build_detects_changed_lines();                      // 4
    test_build_marks_has_changes_true_when_diff_exists();    // 5
    test_build_marks_has_changes_false_when_equal();         // 6
    test_before_lines_split_preserves_count();               // 7
    test_after_lines_split_preserves_count();                // 8
    test_chat_add_mutation_preview_appends_record();         // 9
    test_chat_add_mutation_preview_appends_message();        // 10
    test_chat_preview_message_marks_change_state();          // 11
    test_chat_preview_message_marks_no_change_state();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
