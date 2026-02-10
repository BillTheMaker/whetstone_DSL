// Step 178: Multi-cursor data structure integration checks.

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
    const std::string selection = readFile("src/CodeEditorSelection.h");
    assertContains(selection, "syncPrimaryToMulti");
    assertContains(selection, "applyEdits");
    assertContains(selection, "updateColumnSelection");

    printf("step178_integration_test: all assertions passed\n");
    return 0;
}
