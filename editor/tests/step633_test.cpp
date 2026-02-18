// Step 633: Sprint 37 integration + summary (8 tests)

#include "Sprint37IntegrationSummary.h"

#include <filesystem>
#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string testWorkspace() {
    return "/tmp/whetstone_step633_workspace";
}

static Sprint37IntegrationResult runScenario() {
    const std::string workspace = testWorkspace();
    std::filesystem::remove_all(workspace);
    std::filesystem::create_directories(workspace);
    return Sprint37IntegrationSummary::run(
        workspace,
        "hivemind/bs_s1_01.whet",
        "BS-S1-01: generate drone and nexus modules",
        "module Drone {}\nmodule Nexus {}\n",
        "pub struct Drone;",
        "struct Nexus {};"
    );
}

void test_chat_panel_marked_open() {
    TEST(chat_panel_marked_open);
    auto result = runScenario();
    CHECK(result.chatPanelOpened, "chat panel should be marked open");
    PASS();
}

void test_context_is_injected_on_start() {
    TEST(context_is_injected_on_start);
    auto result = runScenario();
    CHECK(result.contextInjected, "context should be injected");
    PASS();
}

void test_user_task_description_is_sent() {
    TEST(user_task_description_is_sent);
    auto result = runScenario();
    CHECK(result.userMessageSent, "user message should be sent");
    PASS();
}

void test_background_task_is_running_and_overlay_visible() {
    TEST(background_task_is_running_and_overlay_visible);
    auto result = runScenario();
    CHECK(result.taskQueued, "task should be queued");
    CHECK(result.overlayVisible, "overlay should be visible");
    CHECK(result.runningTasks == 1, "running task count mismatch");
    CHECK(result.pendingTasks == 0, "pending task count mismatch");
    PASS();
}

void test_mutation_preview_is_present() {
    TEST(mutation_preview_is_present);
    auto result = runScenario();
    CHECK(result.previewShown, "preview should be shown");
    PASS();
}

void test_mutation_accept_path_is_successful() {
    TEST(mutation_accept_path_is_successful);
    auto result = runScenario();
    CHECK(result.mutationAccepted, "mutation should be accepted");
    PASS();
}

void test_code_updates_to_rust_and_cpp_output() {
    TEST(code_updates_to_rust_and_cpp_output);
    auto result = runScenario();
    CHECK(result.codeUpdated, "code should be updated");
    CHECK(result.activeCode.find("pub struct Drone;") != std::string::npos, "rust output missing");
    CHECK(result.activeCode.find("struct Nexus {};") != std::string::npos, "c++ output missing");
    PASS();
}

void test_session_is_saved_and_restored() {
    TEST(session_is_saved_and_restored);
    auto result = runScenario();
    CHECK(result.sessionSaved, "session should be saved");
    CHECK(result.sessionRestored, "session should be restored");
    CHECK(result.restoredMessageCount >= 3, "restored message count should include context + chat");
    PASS();
}

int main() {
    std::cout << "Step 633: Sprint 37 integration + summary\n";

    test_chat_panel_marked_open();                          // 1
    test_context_is_injected_on_start();                    // 2
    test_user_task_description_is_sent();                   // 3
    test_background_task_is_running_and_overlay_visible();  // 4
    test_mutation_preview_is_present();                     // 5
    test_mutation_accept_path_is_successful();              // 6
    test_code_updates_to_rust_and_cpp_output();             // 7
    test_session_is_saved_and_restored();                   // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
