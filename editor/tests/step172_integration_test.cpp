// Step 172: Theme engine lists bundled themes.

#include <cassert>
#include <string>
#include <vector>

#include "ThemeEngine.h"

static bool contains(const std::vector<std::string>& list, const std::string& value) {
    for (const auto& item : list) {
        if (item == value) return true;
    }
    return false;
}

int main() {
    ImGui::CreateContext();
    ThemeEngine& themes = ThemeEngine::instance();
    themes.clear();
    themes.loadThemesFromDirectory("themes");
    const auto names = themes.listThemes();
    assert(contains(names, "Whetstone Dark"));
    assert(contains(names, "Whetstone Light"));
    assert(contains(names, "Monokai Pro"));
    assert(contains(names, "Solarized Dark"));
    assert(contains(names, "Solarized Light"));
    assert(contains(names, "Dracula"));
    assert(contains(names, "Nord"));
    ImGui::DestroyContext();

    printf("step172_integration_test: all assertions passed\n");
    return 0;
}
