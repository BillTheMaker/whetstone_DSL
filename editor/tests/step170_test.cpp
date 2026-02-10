// Step 170: UIEventBus unit checks.

#include <cassert>
#include <string>

#include "UIEventBus.h"

int main() {
    UIEventBus bus;
    int count = 0;
    std::string lastDetail;

    bus.subscribe(UIEventType::ASTChanged, [&](const UIEvent& ev) {
        ++count;
        lastDetail = ev.detail;
    });

    bus.publish(UIEventType::ASTChanged, "file.cpp", "first", 1.0);
    assert(count == 1);
    assert(lastDetail == "first");

    bus.publishDebounced(UIEventType::ASTChanged, "file.cpp", "debounce", 2.0, 0.5);
    bus.tick(2.3);
    assert(count == 1);
    bus.tick(2.6);
    assert(count == 2);
    assert(lastDetail == "debounce");

    bus.publishDebounced(UIEventType::ASTChanged, "file.cpp", "last", 3.0, 0.5);
    bus.publishDebounced(UIEventType::ASTChanged, "file.cpp", "last2", 3.2, 0.5);
    bus.tick(3.8);
    assert(count == 3);
    assert(lastDetail == "last2");

    printf("step170_test: all assertions passed\n");
    return 0;
}
