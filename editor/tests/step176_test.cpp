// Step 176: Smooth UI transitions checks.

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
    const std::string animUtils = readFile("src/AnimationUtils.h");
    assertContains(animUtils, "panelTransition");
    assertContains(animUtils, "tooltipAlpha");
    assertContains(animUtils, "blinkAlpha");

    const std::string settings = readFile("src/SettingsManager.h");
    assertContains(settings, "reduceMotion");
    assertContains(settings, "cursorBlinkRate");

    const std::string codeEditor = readFile("src/CodeEditorWidget.h");
    assertContains(codeEditor, "searchPulseLine");
    assertContains(codeEditor, "cursorBlinkRate");

    const std::string notifications = readFile("src/NotificationSystem.h");
    assertContains(notifications, "renderToasts");
    assertContains(notifications, "reduceMotion");

    printf("step176_test: all assertions passed\n");
    return 0;
}
