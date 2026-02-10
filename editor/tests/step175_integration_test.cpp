// Step 175: Typography settings integration checks.

#include <cassert>

#include "SettingsManager.h"

int main() {
    SettingsManager settings;
    settings.setCodeFontPath("code.ttf");
    settings.setUiFontPath("ui.ttf");
    settings.setLineHeightScale(1.5f);
    settings.setLetterSpacing(1.0f);
    assert(settings.getCodeFontPath() == "code.ttf");
    assert(settings.getUiFontPath() == "ui.ttf");
    assert(settings.getLineHeightScale() == 1.5f);
    assert(settings.getLetterSpacing() == 1.0f);

    printf("step175_integration_test: all assertions passed\n");
    return 0;
}
