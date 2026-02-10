// Step 170: UIEventBus integration checks.

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
    assert(fs::exists("src/UIEventBus.h"));

    const std::string editorState = readFile("src/EditorState.h");
    assertContains(editorState, "UIEventBus");
    assertContains(editorState, "events");

    const std::string settingsPanel = readFile("src/panels/SettingsPanel.h");
    assertContains(settingsPanel, "SettingsChanged");
    assertContains(settingsPanel, "ThemeChanged");

    const std::string mainCpp = readFile("src/main.cpp");
    assertContains(mainCpp, "events.tick");

    printf("step170_integration_test: all assertions passed\n");
    return 0;
}
