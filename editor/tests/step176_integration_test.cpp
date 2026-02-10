// Step 176: Animation settings integration checks.

#include <cassert>

#include "SettingsManager.h"
#include "AnimationUtils.h"

int main() {
    SettingsManager settings;
    settings.setReduceMotion(true);
    settings.setCursorBlinkRate(1.5f);
    assert(settings.getReduceMotion() == true);
    assert(settings.getCursorBlinkRate() == 1.5f);

    float eased = AnimationUtils::easeOutCubic(0.5f);
    assert(eased >= 0.0f && eased <= 1.0f);

    printf("step176_integration_test: all assertions passed\n");
    return 0;
}
