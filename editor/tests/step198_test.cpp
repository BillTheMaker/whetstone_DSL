// Step 198: Virtual scrolling for large files.

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
    const std::string rendering = readFile("src/CodeEditorRendering.h");

    assertContains(rendering, "virtualBuffer");
    assertContains(rendering, "viewFirst");
    assertContains(rendering, "viewLines");
    assertContains(rendering, "isHiddenLine");

    printf("step198_test: all assertions passed\n");
    return 0;
}
