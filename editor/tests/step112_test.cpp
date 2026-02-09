// Step 112 TDD Test: Breadcrumb trail
#include "Breadcrumbs.h"
#include "ast/Statement.h"
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

    Module mod("m1", "Main", "python");
    Function fn("f1", "add");
    Return ret;

    mod.addChild("functions", &fn);
    fn.addChild("body", &ret);

    auto trail = buildBreadcrumbTrail(&ret);
    bool ok = trail.size() == 5 &&
              trail[0].label == "Module" &&
              trail[1].label == "functions" && trail[1].isRole &&
              trail[2].label == "Function:add" &&
              trail[3].label == "body" && trail[3].isRole &&
              trail[4].label == "Return";
    expect(ok, "breadcrumb path with roles", passed, failed);

    std::cout << "\n=== Step 112 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
