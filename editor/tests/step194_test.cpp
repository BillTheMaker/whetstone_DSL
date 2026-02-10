// Step 194: Semantic-filtered library browser.

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
    const std::string browser = readFile("src/LibraryBrowserPanel.h");
    const std::string registry = readFile("src/PrimitivesRegistry.h");

    assertContains(browser, "Filter by tag");
    assertContains(browser, "Active tags");
    assertContains(browser, "semanticTags");
    assertContains(registry, "setContextTags");

    printf("step194_test: all assertions passed\n");
    return 0;
}
