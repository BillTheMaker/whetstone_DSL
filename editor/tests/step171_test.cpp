// Step 171: Theme engine core checks.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#include "ThemeEngine.h"

namespace fs = std::filesystem;

static void writeFile(const fs::path& path, const std::string& text) {
    std::ofstream out(path.string(), std::ios::binary);
    out << text;
}

int main() {
    ImGui::CreateContext();
    ThemeEngine& themes = ThemeEngine::instance();
    themes.clear();

    const std::string jsonText =
        "{"
        "\"name\":\"UnitTheme\","
        "\"syntax\":{\"keyword\":\"#010203\"},"
        "\"editor\":{\"editor_bg\":\"#040506\"}"
        "}";
    assert(themes.loadThemeFromJson(jsonText));
    assert(themes.applyTheme("UnitTheme"));
    ImU32 keyword = themes.getColor(ThemeColor::SyntaxKeyword, IM_COL32(0, 0, 0, 255));
    ImU32 editorBg = themes.getColor(ThemeColor::EditorBg, IM_COL32(0, 0, 0, 255));
    assert(keyword == IM_COL32(1, 2, 3, 255));
    assert(editorBg == IM_COL32(4, 5, 6, 255));

    fs::path tempDir = fs::current_path() / "test_theme_data";
    fs::create_directories(tempDir);
    fs::path themeFile = tempDir / "reload.json";
    writeFile(themeFile,
              "{"
              "\"name\":\"ReloadTheme\","
              "\"syntax\":{\"keyword\":\"#111111\"}"
              "}");

    assert(themes.loadThemesFromDirectory(tempDir.string()));
    assert(themes.applyTheme("ReloadTheme"));
    ImU32 original = themes.getColor(ThemeColor::SyntaxKeyword, IM_COL32(0, 0, 0, 255));
    assert(original == IM_COL32(17, 17, 17, 255));

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    writeFile(themeFile,
              "{"
              "\"name\":\"ReloadTheme\","
              "\"syntax\":{\"keyword\":\"#222222\"}"
              "}");
    assert(themes.refreshWatchedThemes());
    ImU32 updated = themes.getColor(ThemeColor::SyntaxKeyword, IM_COL32(0, 0, 0, 255));
    assert(updated == IM_COL32(34, 34, 34, 255));

    std::error_code ec;
    fs::remove(themeFile, ec);
    fs::remove(tempDir, ec);

    ImGui::DestroyContext();
    printf("step171_test: all assertions passed\n");
    return 0;
}
