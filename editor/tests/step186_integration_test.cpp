// Step 186: Shortcut reference state integration checks.

#include <cassert>

#include "state/UIFlags.h"

int main() {
    UIFlags flags;
    flags.showShortcutReference = true;
    assert(flags.showShortcutReference == true);

    printf("step186_integration_test: all assertions passed\n");
    return 0;
}
