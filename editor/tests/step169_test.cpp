// Step 169: Notification system unit checks.

#include <cassert>
#include <string>

#include "NotificationSystem.h"

static const Notification* findById(const std::vector<Notification>& list, int id) {
    for (const auto& n : list) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

int main() {
    NotificationSystem notifications;
    notifications.toastDurationSeconds = 5.0f;

    notifications.notifyAt(NotificationLevel::Info, "hello", 1.0);
    assert(notifications.getHistory().size() == 1);
    assert(notifications.unreadCount() == 1);

    NotificationTarget target;
    target.path = "C:\\temp\\example.cpp";
    target.line = 2;
    target.col = 5;
    notifications.notifyAt(NotificationLevel::Error, "boom", 2.0, &target);
    assert(notifications.getHistory().size() == 2);

    const Notification& last = notifications.getHistory().back();
    assert(last.hasTarget);
    assert(last.target.path == target.path);
    assert(last.target.line == target.line);
    assert(last.target.col == target.col);

    int id = last.id;
    notifications.dismiss(id);
    const Notification* found = findById(notifications.getHistory(), id);
    assert(found);
    assert(found->dismissed);
    assert(found->read);

    notifications.markAllRead();
    assert(notifications.unreadCount() == 0);

    printf("step169_test: all assertions passed\n");
    return 0;
}
