#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

static int countLines(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return -1;
    int lines = 0;
    std::string line;
    while (std::getline(in, line)) {
        ++lines;
    }
    return lines;
}

int main() {
    int passed = 0;
    int failed = 0;

    const int maxHeaderLines = 600;

    namespace fs = std::filesystem;
    fs::path root = fs::current_path();
    fs::path srcDir = root / "editor" / "src";
    if (!fs::exists(srcDir)) {
        srcDir = root / "src";
    }

    std::vector<fs::path> headers = {
        srcDir / "CodeEditorWidget.h",
        srcDir / "CodeEditorRendering.h",
        srcDir / "CodeEditorRenderHelpers.h",
        srcDir / "CodeEditorSelection.h",
        srcDir / "SyntaxHighlighter.h",
        srcDir / "SyntaxLanguages.h",
        srcDir / "SyntaxHighlighterPython.h",
        srcDir / "SyntaxHighlighterCpp.h",
        srcDir / "SyntaxHighlighterJavaScript.h",
        srcDir / "SyntaxHighlighterJava.h",
        srcDir / "SyntaxHighlighterRust.h",
        srcDir / "SyntaxHighlighterGo.h",
        srcDir / "SyntaxHighlighterElisp.h",
        srcDir / "SyntaxHighlighterOrg.h",
        srcDir / "ast" / "Parser.h",
        srcDir / "ast" / "PythonParser.h",
        srcDir / "ast" / "CppParser.h",
        srcDir / "ast" / "ElispParser.h",
        srcDir / "ast" / "JavaScriptParser.h",
        srcDir / "ast" / "TypeScriptParser.h",
        srcDir / "ast" / "JavaParser.h",
        srcDir / "ast" / "RustParser.h",
        srcDir / "ast" / "GoParser.h",
        srcDir / "ast" / "CppGenerator.h",
        srcDir / "ast" / "CppGeneratorTypes.h",
    };

    for (const auto& path : headers) {
        expect(fs::exists(path), "exists " + path.string(), passed, failed);
        int lines = countLines(path.string());
        expect(lines > 0, "read " + path.string(), passed, failed);
        if (lines > 0) {
            expect(lines <= maxHeaderLines,
                   "line limit " + path.string() + " (" + std::to_string(lines) + ")",
                   passed, failed);
        }
    }

    std::cout << "\n=== Step 168 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
