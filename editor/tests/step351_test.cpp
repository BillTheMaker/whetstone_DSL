// Step 351: Keybinding Registry (12 tests)

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include "KeybindingRegistry.h"

int main() {
    int passed = 0;

    // Test 1: bind and retrieve
    {
        KeybindingRegistry reg;
        reg.registerAction("save-buffer", "Save Buffer", "File");
        reg.bind("save-buffer", KeyCombo::ctrl("S"));
        assert(reg.getBinding("save-buffer").toString() == "Ctrl+S");
        std::cout << "Test 1 PASSED: bind and retrieve\n";
        passed++;
    }

    // Test 2: default bindings loaded
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        assert(reg.getBinding("save-buffer").toString() == "Ctrl+S");
        assert(reg.getBinding("save-all").toString() == "Ctrl+Shift+S");
        assert(reg.getBinding("run-pipeline").toString() == "F5");
        std::cout << "Test 2 PASSED: default bindings loaded\n";
        passed++;
    }

    // Test 3: key combo toString
    {
        KeyCombo c1 = KeyCombo::ctrl("P");
        KeyCombo c2 = KeyCombo::ctrlShift("F");
        KeyCombo c3 = {KeyModifier::Alt | KeyModifier::Shift, "F4"};
        assert(c1.toString() == "Ctrl+P");
        assert(c2.toString() == "Ctrl+Shift+F");
        assert(c3.toString() == "Alt+Shift+F4");
        std::cout << "Test 3 PASSED: key combo toString\n";
        passed++;
    }

    // Test 4: key combo toSymbols (platform-aware string mode)
    {
        uint8_t mods = static_cast<uint8_t>(KeyModifier::Ctrl) |
                       static_cast<uint8_t>(KeyModifier::Shift) |
                       static_cast<uint8_t>(KeyModifier::Super);
        KeyCombo mac = {mods, "P"};
        std::string symbols = mac.toSymbols(true);
        assert(symbols.find("P") != std::string::npos);
        assert(symbols.find("\xE2\x8C\x83") != std::string::npos); // ⌃
        assert(symbols.find("\xE2\x87\xA7") != std::string::npos); // ⇧
        assert(symbols.find("\xE2\x8C\x98") != std::string::npos); // ⌘
        std::cout << "Test 4 PASSED: key combo toSymbols mac-style\n";
        passed++;
    }

    // Test 5: processInput detects active combo
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyCombo::InputState in;
        in.ctrl = true;
        in.key = "S";
        auto action = reg.processInput(in);
        assert(action.has_value());
        assert(action.value() == "save-buffer");
        std::cout << "Test 5 PASSED: processInput detects combo\n";
        passed++;
    }

    // Test 6: processInput no match
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        KeyCombo::InputState in;
        in.ctrl = true;
        in.alt = true;
        in.key = "S";
        auto action = reg.processInput(in);
        assert(!action.has_value());
        std::cout << "Test 6 PASSED: processInput no match\n";
        passed++;
    }

    // Test 7: save/load JSON roundtrip via file
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        reg.bind("save-buffer", KeyCombo::ctrl("W"));

        const std::string path = "/tmp/whetstone_step351_keybindings.json";
        assert(reg.saveToJson(path));

        KeybindingRegistry loaded;
        loaded.loadDefaults();
        assert(loaded.loadFromJsonFile(path));
        assert(loaded.getBinding("save-buffer").toString() == "Ctrl+W");
        std::filesystem::remove(path);
        std::cout << "Test 7 PASSED: save/load JSON roundtrip\n";
        passed++;
    }

    // Test 8: rebind action
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        reg.bind("undo", KeyCombo::ctrl("U"));
        assert(reg.getBinding("undo").toString() == "Ctrl+U");
        std::cout << "Test 8 PASSED: rebind action\n";
        passed++;
    }

    // Test 9: conflict detection
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        auto conflict = reg.findConflict("close-buffer", KeyCombo::ctrl("S"));
        assert(conflict.has_value());
        assert(conflict.value() == "save-buffer");
        auto none = reg.findConflict("close-buffer", KeyCombo::ctrl("Q"));
        assert(!none.has_value());
        std::cout << "Test 9 PASSED: conflict detection\n";
        passed++;
    }

    // Test 10: getSymbols returns displayable string
    {
        KeybindingRegistry reg;
        reg.loadDefaults();
        std::string saveText = reg.getSymbols("save-buffer", false);
        std::string saveMac = reg.getSymbols("save-buffer", true);
        assert(saveText == "Ctrl+S");
        assert(!saveMac.empty());
        std::cout << "Test 10 PASSED: getSymbols returns displayable string\n";
        passed++;
    }

    // Test 11: modifier bitmask correctness
    {
        uint8_t mods = KeyModifier::Ctrl | KeyModifier::Shift;
        assert(hasModifier(mods, KeyModifier::Ctrl));
        assert(hasModifier(mods, KeyModifier::Shift));
        assert(!hasModifier(mods, KeyModifier::Alt));
        std::cout << "Test 11 PASSED: modifier bitmask correctness\n";
        passed++;
    }

    // Test 12: matches() exactness + getAll map
    {
        KeyCombo combo = KeyCombo::ctrlShift("Z");
        KeyCombo::InputState ok{true, false, true, false, "z"};
        KeyCombo::InputState wrongShift{true, false, false, false, "z"};
        assert(combo.matches(ok));
        assert(!combo.matches(wrongShift));

        KeybindingRegistry reg;
        reg.loadDefaults();
        auto all = reg.getAll();
        assert(!all.empty());
        assert(all.find("save-buffer") != all.end());
        std::cout << "Test 12 PASSED: matches exactness + getAll\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
