// Step 135 TDD Test: Library-aware completion ranking
#include "CompletionUtils.h"
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

    std::vector<LSPClient::CompletionItem> lsp = {
        {"foo", 0, "", "foo", ""},
        {"bar", 0, "", "bar", ""}
    };
    std::vector<PrimitiveSymbol> prims = {
        {"bar", "function", "import", "lib"},
        {"baz", "function", "import", "lib"}
    };

    auto built = buildLibraryAwareCompletions(lsp, prims, "");
    expect(built.preferredNames.count("bar") == 1, "preferred contains bar", passed, failed);
    expect(built.preferredNames.count("baz") == 1, "preferred contains baz", passed, failed);
    expect(built.items.size() == 3, "completion list includes primitive", passed, failed);
    expect(built.items[0].label == "bar" || built.items[0].label == "baz",
           "preferred items sorted first", passed, failed);

    auto builtPref = buildLibraryAwareCompletions(lsp, prims, "ba");
    bool hasBaz = false;
    for (const auto& item : builtPref.items) {
        if (item.label == "baz") hasBaz = true;
    }
    expect(hasBaz, "prefix adds matching primitive", passed, failed);

    std::cout << "\n=== Step 135 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
