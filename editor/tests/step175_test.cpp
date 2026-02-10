// Step 175: Typography and spacing checks.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return {};
    return std::string((std::istreambuf_iterator<char>(f)),
                       std::istreambuf_iterator<char>());
}

static void assertContains(const std::string& text, const std::string& needle) {
    assert(text.find(needle) != std::string::npos);
}

int main() {
    const std::string settings = readFile("src/SettingsManager.h");
    assertContains(settings, "codeFont");
    assertContains(settings, "uiFont");
    assertContains(settings, "lineHeight");
    assertContains(settings, "letterSpacing");

    const std::string editorOpts = readFile("src/CodeEditorWidget.h");
    assertContains(editorOpts, "lineHeightScale");
    assertContains(editorOpts, "letterSpacing");

    const std::string theme = readFile("src/ThemeEngine.h");
    assertContains(theme, "panelPadding");
    assertContains(theme, "panelSpacing");

    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    assertContains(settingsPanel, "Code Font");
    assertContains(settingsPanel, "UI Font");
    assertContains(settingsPanel, "Line Height");
    assertContains(settingsPanel, "Letter Spacing");

    printf("step175_test: all assertions passed\n");
    return 0;
}
