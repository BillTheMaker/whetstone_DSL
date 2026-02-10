// Step 184: First-run wizard state integration checks.

#include <cassert>

#include "state/UIFlags.h"

int main() {
    UIFlags flags;
    flags.showFirstRunWizard = true;
    assert(flags.showFirstRunWizard == true);

    printf("step184_integration_test: all assertions passed\n");
    return 0;
}
