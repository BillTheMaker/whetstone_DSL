// Sprint 6 guard: enforce architecture file size limits
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
    const int maxMainLines = 1500;

    namespace fs = std::filesystem;
    fs::path root = fs::current_path();
    fs::path srcDir = root / "editor" / "src";
    fs::path mainPath = srcDir / "main.cpp";

    int mainLines = countLines(mainPath.string());
    expect(mainLines >= 0, "main.cpp readable", passed, failed);
    if (mainLines >= 0) {
        expect(mainLines <= maxMainLines, "main.cpp line limit", passed, failed);
    }

    int headerCount = 0;
    int overs = 0;
    std::vector<std::string> allowlist = {
        (srcDir / "ast" / "CppGenerator.h").string(),
        (srcDir / "ast" / "JavaGenerator.h").string(),
        (srcDir / "ast" / "JavaScriptGenerator.h").string(),
        (srcDir / "ast" / "Parser.h").string(),
        (srcDir / "CodeEditorWidget.h").string(),
        (srcDir / "EditorState.h").string(),
        (srcDir / "EditorUtils.h").string(),
        (srcDir / "panels" / "BottomPanel.h").string(),
        (srcDir / "panels" / "EditorPanel.h").string(),
        (srcDir / "SyntaxHighlighter.h").string()
    };
    for (const auto& entry : fs::recursive_directory_iterator(srcDir)) {
        if (!entry.is_regular_file()) continue;
        auto path = entry.path();
        if (path.extension() != ".h") continue;
        ++headerCount;
        int lines = countLines(path.string());
        if (lines < 0) {
            expect(false, "read header " + path.string(), passed, failed);
            continue;
        }
        if (lines > maxHeaderLines) {
            bool allowed = std::find(allowlist.begin(), allowlist.end(),
                                     path.string()) != allowlist.end();
            if (allowed) {
                std::cout << "Allowlisted oversize header (" << lines << "): "
                          << path.string() << "\n";
                continue;
            }
            ++overs;
            std::cout << "Header too long (" << lines << "): " << path.string() << "\n";
        }
    }

    expect(headerCount > 0, "header scan", passed, failed);
    expect(overs == 0, "header line limits", passed, failed);

    std::cout << "\n=== File Limits Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
