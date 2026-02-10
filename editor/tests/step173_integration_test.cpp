// Step 173: Theme gallery integration checks.

#include <cassert>
#include <string>

#include "ThemeEngine.h"

int main() {
    ImGui::CreateContext();
    ThemeEngine& themes = ThemeEngine::instance();
    themes.clear();
    themes.loadThemesFromDirectory("themes");
    ImU32 swatch[4] = {};
    bool ok = themes.getSwatch("Whetstone Dark", swatch);
    assert(ok);
    ImGui::DestroyContext();

    printf("step173_integration_test: all assertions passed\n");
    return 0;
}
