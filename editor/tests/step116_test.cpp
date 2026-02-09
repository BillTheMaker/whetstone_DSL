// Step 116 TDD Test: Project save/load serialization
#include "ProjectManager.h"
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

    ProjectFile project;
    project.workspaceRoot = "C:/repo";
    project.activePath = "C:/repo/main.py";
    project.buffers.push_back({"C:/repo/main.py", "python", "structured"});
    project.buffers.push_back({"C:/repo/util.py", "python", "text"});
    project.ast = std::make_unique<Module>("m1", "Main", "python");

    auto j = projectToJson(project);
    ProjectFile loaded = projectFromJson(j);

    bool okRoot = loaded.workspaceRoot == "C:/repo";
    bool okActive = loaded.activePath == "C:/repo/main.py";
    bool okBuffers = loaded.buffers.size() == 2 && loaded.buffers[1].mode == "text";
    bool okAst = loaded.ast && loaded.ast->name == "Main" && loaded.ast->targetLanguage == "python";

    expect(okRoot, "workspace root", passed, failed);
    expect(okActive, "active path", passed, failed);
    expect(okBuffers, "buffer list", passed, failed);
    expect(okAst, "ast roundtrip", passed, failed);

    std::cout << "\n=== Step 116 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
