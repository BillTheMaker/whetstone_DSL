// Step 118 TDD Test: Settings persistence
#include "SettingsManager.h"
#include <iostream>
#include <filesystem>

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

    std::filesystem::path path = std::filesystem::temp_directory_path() / "whetstone_settings_test.json";

    SettingsManager settings;
    settings.setFontSize(18);
    settings.setTabSize(2);
    settings.setTheme("Light");
    settings.setAutoSaveSeconds(15);
    settings.setShowMinimap(true);
    settings.setShowLineNumbers(false);
    settings.setLayoutPreset("Emacs");
    settings.setKeybindingProfile("JetBrains");
    settings.setEmacsConfigPath("C:/emacs");
    bool saved = settings.saveToFile(path.string());
    expect(saved, "save settings", passed, failed);

    SettingsManager loaded;
    bool loadedOk = loaded.loadFromFile(path.string());
    expect(loadedOk, "load settings", passed, failed);
    expect(loaded.getFontSize() == 18, "font size", passed, failed);
    expect(loaded.getTabSize() == 2, "tab size", passed, failed);
    expect(loaded.getTheme() == "Light", "theme", passed, failed);
    expect(loaded.getAutoSaveSeconds() == 15, "auto-save", passed, failed);
    expect(loaded.getShowMinimap() == true, "minimap", passed, failed);
    expect(loaded.getShowLineNumbers() == false, "line numbers", passed, failed);
    expect(loaded.getLayoutPreset() == "Emacs", "layout preset", passed, failed);
    expect(loaded.getKeybindingProfile() == "JetBrains", "keybindings", passed, failed);
    expect(loaded.getEmacsConfigPath() == "C:/emacs", "emacs path", passed, failed);

    std::filesystem::remove(path);

    std::cout << "\n=== Step 118 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
