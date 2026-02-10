// Step 174: Icon system checks.

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
    assert(fs::exists("src/IconSet.h"));
    const std::string iconSet = readFile("src/IconSet.h");
    assertContains(iconSet, "FileIconType");
    assertContains(iconSet, "PanelIconType");
    assertContains(iconSet, "DiagnosticIconType");
    assertContains(iconSet, "AnnotationIconType");

    printf("step174_test: all assertions passed\n");
    return 0;
}
