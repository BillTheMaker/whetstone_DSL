// Step 193: Semantic tags for libraries.

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
    const std::string tags = readFile("src/SemanticTags.h");
    const std::string ext = readFile("src/ast/ExternalModule.h");
    const std::string sig = readFile("src/ast/TypeSignature.h");
    const std::string indexer = readFile("src/LibraryIndexer.h");

    assertContains(tags, "@serialize");
    assertContains(tags, "semantic_tags.json");
    assertContains(ext, "semanticTags");
    assertContains(sig, "semanticTags");
    assertContains(indexer, "tagsForSymbol");

    printf("step193_test: all assertions passed\n");
    return 0;
}
