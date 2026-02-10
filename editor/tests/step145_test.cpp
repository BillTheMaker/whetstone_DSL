// Step 145 TDD Test: Org-mode parsing and source blocks
#include "OrgMode.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    std::string text =
        "* Title\n"
        "\n"
        "Some paragraph text.\n"
        "\n"
        "#+begin_src python\n"
        "print('hi')\n"
        "#+end_src\n";

    auto blocks = parseOrgBlocks(text);
    expect(blocks.size() == 3, "parsed block count", passed, failed);
    expect(blocks[0].type == OrgBlockType::Heading, "heading block", passed, failed);
    expect(blocks[0].title == "Title", "heading title", passed, failed);
    expect(blocks[1].type == OrgBlockType::Paragraph, "paragraph block", passed, failed);
    expect(blocks[2].type == OrgBlockType::SourceBlock, "source block", passed, failed);
    expect(blocks[2].language == "python", "source language", passed, failed);
    expect(blocks[2].code.find("print") != std::string::npos, "source code", passed, failed);

    std::string rebuilt = buildOrgText(blocks);
    expect(rebuilt.find("#+begin_src python") != std::string::npos, "build org text", passed, failed);

    OrgDocumentState state;
    updateOrgDocument(state, text);
    expect(!state.blocks.empty(), "doc blocks stored", passed, failed);
    expect(state.editors.size() == 1, "source editors count", passed, failed);
    expect(state.modes.size() == 1, "source modes count", passed, failed);

    std::cout << "\n=== Step 145 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
