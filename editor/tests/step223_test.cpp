// Step 223: Evaluation runner CLI wiring.

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
    const std::string mainSrc = readFile("src/eval_main.cpp");
    assertContains(mainSrc, "whetstone_eval");
    assertContains(mainSrc, "--tasks");
    assertContains(mainSrc, "EvalRunner::loadTasksFromDir");
    printf("step223_test: all assertions passed\n");
    return 0;
}
