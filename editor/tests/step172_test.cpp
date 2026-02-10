// Step 172: Bundled theme pack checks.

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

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
    struct ThemeFile {
        const char* file;
        const char* name;
    };
    const ThemeFile themes[] = {
        {"themes/whetstone_dark.json", "Whetstone Dark"},
        {"themes/whetstone_light.json", "Whetstone Light"},
        {"themes/monokai_pro.json", "Monokai Pro"},
        {"themes/solarized_dark.json", "Solarized Dark"},
        {"themes/solarized_light.json", "Solarized Light"},
        {"themes/dracula.json", "Dracula"},
        {"themes/nord.json", "Nord"},
    };

    for (const auto& theme : themes) {
        assert(fs::exists(theme.file));
        const std::string content = readFile(theme.file);
        assert(!content.empty());
        const std::string needle = std::string("\"name\": \"") + theme.name + "\"";
        assertContains(content, needle);
    }

    printf("step172_test: all assertions passed\n");
    return 0;
}
