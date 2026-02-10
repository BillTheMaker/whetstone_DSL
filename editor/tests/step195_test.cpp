// Step 195: Security & semantic UX tests.

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
    const std::string depPanel = readFile("src/DependencyPanel.h");
    const std::string editorState = readFile("src/EditorState.h");
    const std::string primReg = readFile("src/PrimitivesRegistry.h");
    const std::string semanticTags = readFile("src/SemanticTags.h");
    const std::string browser = readFile("src/LibraryBrowserPanel.h");
    const std::string agent = readFile("src/AgentCodeGen.h");

    // 1) Vulnerable package badge + advisory details.
    assertContains(depPanel, "drawSecurityBadge");
    assertContains(depPanel, "Security");
    assertContains(depPanel, "Vulnerabilities found");

    // 2) Security diagnostics with severity mapping.
    assertContains(editorState, "ed.message = \"[Security]");
    assertContains(editorState, "top.severity");
    assertContains(editorState, "ed.severity = severity");

    // 3) Semantic tag auto-assignment (numpy -> @math).
    assertContains(semanticTags, "numpy");
    assertContains(semanticTags, "@math");

    // 4) Library browser tag filtering + badges.
    assertContains(browser, "Filter by tag");
    assertContains(browser, "symbolMatchesTags");
    assertContains(browser, "joinTags");

    // 5) Agent completion deprioritizes vulnerable package symbols.
    assertContains(primReg, "sym.vulnerable");
    assertContains(primReg, "s -= 5");
    assertContains(agent, "!sym.vulnerable");

    // 6) Upgrade to safe version writes dependency file.
    assertContains(depPanel, "Upgrade to Safe Version");
    assertContains(depPanel, "bestFixedVersion");
    assertContains(depPanel, "writeDependenciesForSource");

    printf("step195_test: all assertions passed\n");
    return 0;
}
