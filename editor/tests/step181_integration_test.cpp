// Step 181: Status bar helper integration checks.

#include <cassert>

#include "NotificationSystem.h"

int main() {
    NotificationSystem system;
    system.notifyAt(NotificationLevel::Info, "Hello", 1.0);
    const Notification* latest = system.latest();
    assert(latest != nullptr);
    assert(latest->message == "Hello");

    printf("step181_integration_test: all assertions passed\n");
    return 0;
}
