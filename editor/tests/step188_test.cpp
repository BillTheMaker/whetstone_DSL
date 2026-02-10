// Step 188: Command palette enhancements.

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
    const std::string palette = readFile("src/CommandPalette.h");
    const std::string dialogs = readFile("src/panels/DialogPanels.h");

    assertContains(palette, "CommandContext");
    assertContains(palette, "CommandMatch");
    assertContains(palette, "recent(");
    assertContains(palette, "inlineInput");
    assertContains(palette, "fuzzyMatch");

    assertContains(dialogs, "InputTextWithHint");
    assertContains(dialogs, "prefix '>'");
    assertContains(dialogs, "commandMode");

    printf("step188_test: all assertions passed\n");
    return 0;
}
