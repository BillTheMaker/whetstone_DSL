// Step 521: Notification + Status Signaling Refresh (12 tests)

#include "NotificationStatusRefresh.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_info_style_has_short_duration() {
    TEST(info_style_has_short_duration);
    auto s = NotificationStatusRefresh::styleFor(NotifyLevel::Info);
    CHECK(s.defaultDurationMs > 0 && s.defaultDurationMs < 3000, "info duration invalid");
    PASS();
}

void test_warn_duration_longer_than_info() {
    TEST(warn_duration_longer_than_info);
    auto a = NotificationStatusRefresh::styleFor(NotifyLevel::Info);
    auto b = NotificationStatusRefresh::styleFor(NotifyLevel::Warn);
    CHECK(b.defaultDurationMs > a.defaultDurationMs, "warn should last longer");
    PASS();
}

void test_error_duration_longer_than_warn() {
    TEST(error_duration_longer_than_warn);
    auto a = NotificationStatusRefresh::styleFor(NotifyLevel::Warn);
    auto b = NotificationStatusRefresh::styleFor(NotifyLevel::Error);
    CHECK(b.defaultDurationMs > a.defaultDurationMs, "error should last longer");
    PASS();
}

void test_critical_is_persistent_by_default() {
    TEST(critical_is_persistent_by_default);
    auto s = NotificationStatusRefresh::styleFor(NotifyLevel::Critical);
    CHECK(s.defaultDurationMs == 0.0f, "critical should be persistent");
    PASS();
}

void test_readability_passes_all_levels() {
    TEST(readability_passes_all_levels);
    bool ok = true;
    for (int i = 0; i < 4; ++i) {
        if (!NotificationStatusRefresh::readable(
                NotificationStatusRefresh::styleFor(static_cast<NotifyLevel>(i)))) {
            ok = false;
        }
    }
    CHECK(ok, "one or more levels not readable");
    PASS();
}

void test_message_factory_uses_style_duration() {
    TEST(message_factory_uses_style_duration);
    auto m = NotificationStatusRefresh::makeMessage(NotifyLevel::Warn, "warn", 1000.0f);
    CHECK(m.durationMs == NotificationStatusRefresh::styleFor(NotifyLevel::Warn).defaultDurationMs,
          "factory should use style duration");
    PASS();
}

void test_expire_true_after_duration() {
    TEST(expire_true_after_duration);
    auto m = NotificationStatusRefresh::makeMessage(NotifyLevel::Info, "info", 0.0f);
    CHECK(NotificationStatusRefresh::shouldExpire(m, m.durationMs + 1.0f),
          "message should expire after duration");
    PASS();
}

void test_expire_false_before_duration() {
    TEST(expire_false_before_duration);
    auto m = NotificationStatusRefresh::makeMessage(NotifyLevel::Info, "info", 0.0f);
    CHECK(!NotificationStatusRefresh::shouldExpire(m, m.durationMs - 1.0f),
          "message should not expire early");
    PASS();
}

void test_dismissed_message_expires_immediately() {
    TEST(dismissed_message_expires_immediately);
    auto m = NotificationStatusRefresh::makeMessage(NotifyLevel::Critical, "crit", 0.0f);
    m.dismissed = true;
    CHECK(NotificationStatusRefresh::shouldExpire(m, 10.0f), "dismissed should expire");
    PASS();
}

void test_stack_policy_keeps_latest_messages() {
    TEST(stack_policy_keeps_latest_messages);
    std::vector<NotificationMessage> msgs;
    for (int i = 0; i < 8; ++i) msgs.push_back(NotificationStatusRefresh::makeMessage(NotifyLevel::Info, std::to_string(i), i));
    auto out = NotificationStatusRefresh::applyStackPolicy(msgs, 5);
    CHECK((int)out.size() == 5, "stack size should cap to 5");
    CHECK(out.front().text == "3" && out.back().text == "7", "should keep latest entries");
    PASS();
}

void test_stack_policy_no_change_when_under_limit() {
    TEST(stack_policy_no_change_when_under_limit);
    std::vector<NotificationMessage> msgs;
    for (int i = 0; i < 3; ++i) msgs.push_back(NotificationStatusRefresh::makeMessage(NotifyLevel::Info, std::to_string(i), i));
    auto out = NotificationStatusRefresh::applyStackPolicy(msgs, 5);
    CHECK((int)out.size() == 3, "stack should remain unchanged");
    PASS();
}

void test_critical_level_has_largest_stack_budget() {
    TEST(critical_level_has_largest_stack_budget);
    auto c = NotificationStatusRefresh::styleFor(NotifyLevel::Critical);
    auto i = NotificationStatusRefresh::styleFor(NotifyLevel::Info);
    CHECK(c.maxStack > i.maxStack, "critical stack budget should be higher");
    PASS();
}

int main() {
    std::cout << "Step 521: Notification + Status Signaling Refresh\n";

    test_info_style_has_short_duration();                // 1
    test_warn_duration_longer_than_info();               // 2
    test_error_duration_longer_than_warn();              // 3
    test_critical_is_persistent_by_default();            // 4
    test_readability_passes_all_levels();                // 5
    test_message_factory_uses_style_duration();          // 6
    test_expire_true_after_duration();                   // 7
    test_expire_false_before_duration();                 // 8
    test_dismissed_message_expires_immediately();        // 9
    test_stack_policy_keeps_latest_messages();           // 10
    test_stack_policy_no_change_when_under_limit();      // 11
    test_critical_level_has_largest_stack_budget();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
