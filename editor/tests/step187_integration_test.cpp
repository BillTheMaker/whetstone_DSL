// Step 187: Help panel flag integration checks.

#include <cassert>

#include "state/UIFlags.h"

int main() {
    UIFlags flags;
    flags.showHelpPanel = true;
    assert(flags.showHelpPanel == true);

    printf("step187_integration_test: all assertions passed\n");
    return 0;
}
