// Step 171: Theme engine integration checks.

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
    assert(fs::exists("src/ThemeEngine.h"));
    const std::string themeEngine = readFile("src/ThemeEngine.h");
    assertContains(themeEngine, "ThemeColor");
    assertContains(themeEngine, "refreshWatchedThemes");
    assertContains(themeEngine, "userThemeDirectory");

    const std::string mainCpp = readFile("src/main.cpp");
    assertContains(mainCpp, "userThemeDirectory");
    assertContains(mainCpp, "refreshWatchedThemes");

    printf("step171_integration_test: all assertions passed\n");
    return 0;
}
