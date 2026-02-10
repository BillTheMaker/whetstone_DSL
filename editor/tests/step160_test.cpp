// Step 160 TDD Test: Theme engine
#include "ThemeEngine.h"
#include "imgui.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    ImGui::CreateContext();

    ThemeEngine& themes = ThemeEngine::instance();
    themes.clear();

    std::string jsonText = R"({
        "name": "TestTheme",
        "syntax": { "keyword": "#FF0000" },
        "editor": { "gutter_bg": "#001122" },
        "imgui": { "WindowBg": "#112233" }
    })";

    expect(themes.loadThemeFromJson(jsonText), "load theme from json", passed, failed);
    expect(themes.applyTheme("TestTheme"), "apply theme", passed, failed);

    ImU32 keyword = themes.syntaxColor(TokenCategory::Keyword, IM_COL32(0,0,0,255));
    ImU32 gutter = themes.editorColor("gutter_bg", IM_COL32(0,0,0,255));
    expect(keyword == IM_COL32(255, 0, 0, 255), "syntax color parsed", passed, failed);
    expect(gutter == IM_COL32(0, 17, 34, 255), "editor color parsed", passed, failed);

    ImGui::DestroyContext();

    std::cout << "\n=== Step 160 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
