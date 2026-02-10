// Step 184: First-run wizard checks.

#include <cassert>
#include <fstream>
#include <string>

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
    const std::string wizard = readFile("src/FirstRunWizard.h");
    assertContains(wizard, "Setup Wizard");
    assertContains(wizard, "Finish");

    const std::string uiFlags = readFile("src/state/UIFlags.h");
    assertContains(uiFlags, "showFirstRunWizard");

    printf("step184_test: all assertions passed\n");
    return 0;
}
