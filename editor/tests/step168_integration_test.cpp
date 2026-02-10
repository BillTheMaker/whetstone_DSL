#include <iostream>
#include <string>

#include "ast/Parser.h"
#include "SyntaxHighlighter.h"

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

    const std::string py = "def f(x):\n    return x + 1\n";
    auto result = TreeSitterParser::parsePythonWithDiagnostics(py);
    expect(result.module != nullptr, "parsePythonWithDiagnostics returns module", passed, failed);
    expect(!result.hasErrors(), "parsePythonWithDiagnostics no errors", passed, failed);

    auto spans = SyntaxHighlighter::highlight("def f():\n  return 1\n", "python");
    expect(!spans.empty(), "syntax highlighter returns spans", passed, failed);
    expect(std::string(SyntaxHighlighter::categoryName(TokenCategory::Keyword)) == "keyword",
           "categoryName keyword", passed, failed);

    std::cout << "\n=== Step 168 Integration Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
