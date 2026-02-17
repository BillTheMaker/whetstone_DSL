// Step 584: Value-Forward Onboarding Flow (12 tests)

#include "ValueForwardOnboardingFlow.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_start_success() {
    TEST(start_success);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start should succeed");
    CHECK(state.active, "flow should be active");
    CHECK(state.cards.size() == 4, "default card count mismatch");
    PASS();
}

void test_start_fails_without_profile_id() {
    TEST(start_fails_without_profile_id);
    OnboardingFlowState state;
    std::string error;
    CHECK(!ValueForwardOnboardingFlow::start("", &state, &error), "start should fail");
    CHECK(error == "profile_id_missing", "wrong error");
    PASS();
}

void test_complete_current_marks_card_completed() {
    TEST(complete_current_marks_card_completed);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete current failed");
    CHECK(state.cards[0].completed, "current card should be completed");
    PASS();
}

void test_complete_current_fails_when_inactive() {
    TEST(complete_current_fails_when_inactive);
    OnboardingFlowState state;
    std::string error;
    CHECK(!ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete should fail");
    CHECK(error == "flow_inactive", "wrong error");
    PASS();
}

void test_next_requires_current_completion() {
    TEST(next_requires_current_completion);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(!ValueForwardOnboardingFlow::next(&state, &error), "next should fail");
    CHECK(error == "current_card_not_completed", "wrong error");
    PASS();
}

void test_next_advances_index_after_completion() {
    TEST(next_advances_index_after_completion);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete failed");
    CHECK(ValueForwardOnboardingFlow::next(&state, &error), "next failed");
    CHECK(state.currentIndex == 1, "index should advance");
    PASS();
}

void test_next_on_last_card_finishes_flow() {
    TEST(next_on_last_card_finishes_flow);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    for (int i = 0; i < 4; ++i) {
        CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete failed");
        CHECK(ValueForwardOnboardingFlow::next(&state, &error), "next failed");
    }
    CHECK(!state.active, "flow should be inactive at end");
    PASS();
}

void test_progress_reflects_completion_fraction() {
    TEST(progress_reflects_completion_fraction);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete failed");
    const float p = ValueForwardOnboardingFlow::progress(state);
    CHECK(p > 0.24f && p < 0.26f, "progress should be 0.25");
    PASS();
}

void test_progress_zero_for_empty_state() {
    TEST(progress_zero_for_empty_state);
    OnboardingFlowState state;
    CHECK(ValueForwardOnboardingFlow::progress(state) == 0.0f, "empty progress should be zero");
    PASS();
}

void test_value_highlights_include_completed_card_signals() {
    TEST(value_highlights_include_completed_card_signals);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete card 1 failed");
    CHECK(ValueForwardOnboardingFlow::next(&state, &error), "next failed");
    CHECK(ValueForwardOnboardingFlow::completeCurrent(&state, &error), "complete card 2 failed");
    const auto highlights = ValueForwardOnboardingFlow::valueHighlights(state);
    CHECK(highlights.size() == 2, "two highlights expected");
    CHECK(highlights[0] == "mcp_tool_depth", "first highlight mismatch");
    CHECK(highlights[1] == "safe_constrained_ops", "second highlight mismatch");
    PASS();
}

void test_value_highlights_empty_when_nothing_completed() {
    TEST(value_highlights_empty_when_nothing_completed);
    OnboardingFlowState state;
    std::string error;
    CHECK(ValueForwardOnboardingFlow::start("profile-1", &state, &error), "start failed");
    CHECK(ValueForwardOnboardingFlow::valueHighlights(state).empty(), "highlights should be empty");
    PASS();
}

void test_next_fails_when_inactive() {
    TEST(next_fails_when_inactive);
    OnboardingFlowState state;
    std::string error;
    CHECK(!ValueForwardOnboardingFlow::next(&state, &error), "next should fail");
    CHECK(error == "flow_inactive", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 584: Value-Forward Onboarding Flow\n";

    test_start_success();                                // 1
    test_start_fails_without_profile_id();               // 2
    test_complete_current_marks_card_completed();        // 3
    test_complete_current_fails_when_inactive();         // 4
    test_next_requires_current_completion();             // 5
    test_next_advances_index_after_completion();         // 6
    test_next_on_last_card_finishes_flow();              // 7
    test_progress_reflects_completion_fraction();        // 8
    test_progress_zero_for_empty_state();                // 9
    test_value_highlights_include_completed_card_signals();// 10
    test_value_highlights_empty_when_nothing_completed();// 11
    test_next_fails_when_inactive();                     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
