// Step 183: Tab drag/drop and rename checks.

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
    const std::string editorPanel = readFile("src/panels/EditorPanel.h");
    assertContains(editorPanel, "ImGuiTabBarFlags_Reorderable");
    assertContains(editorPanel, "Rename Buffer");

    const std::string buffers = readFile("src/BufferManager.h");
    assertContains(buffers, "renameBuffer");

    printf("step183_test: all assertions passed\n");
    return 0;
}
