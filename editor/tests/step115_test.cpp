// Step 115 TDD Test: Orchestrator wiring + Emacs config setting
#include "Orchestrator.h"
#include "SettingsManager.h"
#include <iostream>
#include <memory>

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

    {
        Orchestrator orch;
        auto mod = std::make_unique<Module>("m1", "Main", "python");
        orch.setAST(std::move(mod));
        expect(orch.getAST() != nullptr && orch.getAST()->name == "Main", "orchestrator stores AST", passed, failed);
    }

    {
        SettingsManager settings;
        std::string path = settings.getEmacsConfigPath();
        bool ok = !path.empty() && path.find(".emacs.d") != std::string::npos;
        expect(ok, "emacs config default", passed, failed);
        settings.setEmacsConfigPath("C:/custom/emacs");
        expect(settings.getEmacsConfigPath() == "C:/custom/emacs", "emacs config set", passed, failed);
    }

    std::cout << "\n=== Step 115 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
