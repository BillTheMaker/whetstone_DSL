// Step 173: Theme gallery UI checks.

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
    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    assertContains(settingsPanel, "themeGallery");
    assertContains(settingsPanel, "Hover to preview");
    assertContains(settingsPanel, "Apply");
    assertContains(settingsPanel, "Reset");

    printf("step173_test: all assertions passed\n");
    return 0;
}
