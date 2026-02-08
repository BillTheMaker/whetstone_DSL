// Step 54 TDD Test: Configurable keybinding profiles
//
// Verifies that KeybindingManager:
// 1. Defaults to VSCode profile
// 2. Can switch between profiles (VSCode, JetBrains, Emacs)
// 3. Different profiles have different bindings for the same actions
// 4. Reverse lookup (key combo -> action) works
// 5. Custom binding overrides work
// 6. All profiles define core actions
// 7. Profile names round-trip correctly

#include <iostream>
#include <string>
#include <cassert>
#include <algorithm>
#include "KeybindingManager.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Default profile is VSCode ---
    {
        KeybindingManager mgr;
        assert(mgr.getProfile() == KeybindingProfile::VSCode &&
               "Default profile should be VSCode");

        std::cout << "Test 1 PASS: Default profile is VSCode" << std::endl;
        ++passed;
    }

    // --- Test 2: VSCode bindings are correct ---
    {
        KeybindingManager mgr;
        KeyCombo save = mgr.getBinding("file.save");
        assert(save.key == 'S' && save.modifiers == WMOD_CTRL &&
               "VSCode: file.save should be Ctrl+S");

        KeyCombo undo = mgr.getBinding("edit.undo");
        assert(undo.key == 'Z' && undo.modifiers == WMOD_CTRL &&
               "VSCode: edit.undo should be Ctrl+Z");

        KeyCombo redo = mgr.getBinding("edit.redo");
        assert(redo.key == 'Y' && redo.modifiers == WMOD_CTRL &&
               "VSCode: edit.redo should be Ctrl+Y");

        std::cout << "Test 2 PASS: VSCode bindings correct" << std::endl;
        ++passed;
    }

    // --- Test 3: JetBrains has different redo ---
    {
        KeybindingManager mgr;
        mgr.setProfile(KeybindingProfile::JetBrains);
        assert(mgr.getProfile() == KeybindingProfile::JetBrains);

        KeyCombo redo = mgr.getBinding("edit.redo");
        assert(redo.key == 'Z' && redo.modifiers == (WMOD_CTRL | WMOD_SHIFT) &&
               "JetBrains: edit.redo should be Ctrl+Shift+Z");

        // But save is still Ctrl+S
        KeyCombo save = mgr.getBinding("file.save");
        assert(save.key == 'S' && save.modifiers == WMOD_CTRL &&
               "JetBrains: file.save should still be Ctrl+S");

        std::cout << "Test 3 PASS: JetBrains profile has different redo" << std::endl;
        ++passed;
    }

    // --- Test 4: Emacs profile loads ---
    {
        KeybindingManager mgr;
        mgr.setProfile(KeybindingProfile::Emacs);
        assert(mgr.getProfile() == KeybindingProfile::Emacs);

        // Emacs uses Ctrl+Y for paste
        KeyCombo paste = mgr.getBinding("edit.paste");
        assert(paste.key == 'Y' && paste.modifiers == WMOD_CTRL &&
               "Emacs: edit.paste should be Ctrl+Y");

        std::cout << "Test 4 PASS: Emacs profile loads with correct bindings" << std::endl;
        ++passed;
    }

    // --- Test 5: Reverse lookup (combo -> action) ---
    {
        KeybindingManager mgr;  // VSCode
        KeyCombo ctrlS = {'S', WMOD_CTRL};
        std::string action = mgr.getAction(ctrlS);
        assert(action == "file.save" && "Ctrl+S should map to file.save");

        KeyCombo unknown = {'Q', WMOD_CTRL | WMOD_ALT | WMOD_SHIFT};
        std::string none = mgr.getAction(unknown);
        assert(none.empty() && "Unknown combo should return empty string");

        std::cout << "Test 5 PASS: Reverse lookup works" << std::endl;
        ++passed;
    }

    // --- Test 6: Custom binding override ---
    {
        KeybindingManager mgr;
        mgr.setBinding("file.save", {'S', WMOD_CTRL | WMOD_ALT});

        KeyCombo save = mgr.getBinding("file.save");
        assert(save.key == 'S' && save.modifiers == (WMOD_CTRL | WMOD_ALT) &&
               "Custom override should change binding");

        std::cout << "Test 6 PASS: Custom binding override" << std::endl;
        ++passed;
    }

    // --- Test 7: All profiles define core actions ---
    {
        std::vector<std::string> coreActions = {
            "file.save", "edit.undo", "edit.redo", "edit.copy", "edit.paste",
            "search.find", "code.comment"
        };

        for (auto profile : KeybindingManager::availableProfiles()) {
            KeybindingManager mgr;
            mgr.setProfile(profile);
            for (const auto& action : coreActions) {
                KeyCombo combo = mgr.getBinding(action);
                assert(combo.key != 0 &&
                       (std::string("Profile ") +
                        KeybindingManager::profileName(profile) +
                        " missing action: " + action).c_str());
            }
        }

        std::cout << "Test 7 PASS: All profiles define core actions" << std::endl;
        ++passed;
    }

    // --- Test 8: Profile name round-trip ---
    {
        for (auto profile : KeybindingManager::availableProfiles()) {
            const char* name = KeybindingManager::profileName(profile);
            KeybindingProfile roundTripped = KeybindingManager::profileFromName(name);
            assert(roundTripped == profile && "Profile name should round-trip");
        }

        // Unknown name defaults to VSCode
        KeybindingProfile def = KeybindingManager::profileFromName("unknown");
        assert(def == KeybindingProfile::VSCode && "Unknown profile should default to VSCode");

        std::cout << "Test 8 PASS: Profile name round-trip" << std::endl;
        ++passed;
    }

    // --- Test 9: KeyCombo toString ---
    {
        KeyCombo ctrlS = {'S', WMOD_CTRL};
        assert(ctrlS.toString() == "Ctrl+S" && "Ctrl+S toString");

        KeyCombo ctrlShiftZ = {'Z', WMOD_CTRL | WMOD_SHIFT};
        assert(ctrlShiftZ.toString() == "Ctrl+Shift+Z" && "Ctrl+Shift+Z toString");

        std::cout << "Test 9 PASS: KeyCombo toString" << std::endl;
        ++passed;
    }

    // --- Test 10: listActions returns all bound actions ---
    {
        KeybindingManager mgr;
        auto actions = mgr.listActions();
        assert(actions.size() > 20 && "VSCode profile should have 20+ actions");

        // Check a few exist
        assert(std::find(actions.begin(), actions.end(), "file.save") != actions.end());
        assert(std::find(actions.begin(), actions.end(), "edit.undo") != actions.end());
        assert(std::find(actions.begin(), actions.end(), "search.find") != actions.end());

        std::cout << "Test 10 PASS: listActions returns all bound actions" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 54 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
