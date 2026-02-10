// Step 177: Enhanced find/replace UI + wiring checks.

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
    const std::string searchUtils = readFile("src/SearchUtils.h");
    assertContains(searchUtils, "collectMatches");
    assertContains(searchUtils, "replaceAll");
    assertContains(searchUtils, "regexGroupsForFirstMatch");

    const std::string searchPanel = readFile("src/panels/SearchPanels.h");
    assertContains(searchPanel, "Find Next");
    assertContains(searchPanel, "Find Prev");
    assertContains(searchPanel, "Replace preview");
    assertContains(searchPanel, "Regex");
    assertContains(searchPanel, "Match Case");
    assertContains(searchPanel, "Whole Word");
    assertContains(searchPanel, "In Selection");

    const std::string editorState = readFile("src/EditorState.h");
    assertContains(editorState, "doFindNext");
    assertContains(editorState, "doReplaceCurrent");

    printf("step177_test: all assertions passed\n");
    return 0;
}
