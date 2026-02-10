// Step 185: Feature hints checks.

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
    const std::string hints = readFile("src/FeatureHints.h");
    assertContains(hints, "FeatureHintsState");
    assertContains(hints, "renderFeatureHintBar");

    printf("step185_test: all assertions passed\n");
    return 0;
}
