// Step 185: Feature hints integration checks.

#include <cassert>

#include "FeatureHints.h"

int main() {
    FeatureHintsState state;
    queueFeatureHint(state, "hint.test", "Test hint");
    assert(!state.activeMessage.empty());

    printf("step185_integration_test: all assertions passed\n");
    return 0;
}
