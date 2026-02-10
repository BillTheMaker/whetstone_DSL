// Step 244 TDD Test: First-launch UX checks
//
// Verifies:
// 1. New buffers default to Text mode.
// 2. Text mode disables structured-only features.
// 3. Structured mode enables structured-only features.
// 4. Mode toggle behavior persists per-buffer in BufferManager.
// 5. LayoutManager VSCode preset matches DockBuilder expectations.
// 6. Settings defaultBufferMode preference is readable and can drive default mode.

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#include "BufferManager.h"
#include "EditorModePolicy.h"
#include "LayoutManager.h"
#include "SettingsManager.h"

static bool approx(float a, float b) {
    return std::fabs(a - b) < 0.0001f;
}

int main() {
    int passed = 0;
    int failed = 0;

    // 1. New buffers default to Text mode.
    {
        BufferManager bm;
        bm.openBuffer("a.py", "print('x')", "python");
        assert(bm.getBufferMode("a.py") == BufferManager::BufferMode::Text);
        std::cout << "Test 1 PASS: new buffers default to text mode" << std::endl;
        ++passed;
    }

    // 2. Text mode disables structured-only features.
    {
        assert(isTextMode(BufferManager::BufferMode::Text));
        assert(!allowStructuredFeatures(BufferManager::BufferMode::Text));
        std::cout << "Test 2 PASS: text mode disables structured features" << std::endl;
        ++passed;
    }

    // 3. Structured mode enables structured-only features.
    {
        assert(!isTextMode(BufferManager::BufferMode::Structured));
        assert(allowStructuredFeatures(BufferManager::BufferMode::Structured));
        std::cout << "Test 3 PASS: structured mode enables structured features" << std::endl;
        ++passed;
    }

    // 4. Mode toggle behavior persists per-buffer.
    {
        BufferManager bm;
        bm.openBuffer("b.py", "", "python");
        bm.setBufferMode("b.py", BufferManager::BufferMode::Structured);
        assert(bm.getBufferMode("b.py") == BufferManager::BufferMode::Structured);
        bm.switchToBuffer("b.py");
        assert(bm.getBufferMode("b.py") == BufferManager::BufferMode::Structured);
        std::cout << "Test 4 PASS: mode toggle persists per-buffer" << std::endl;
        ++passed;
    }

    // 5. LayoutManager VSCode preset data matches Step 242 DockBuilder expectations.
    {
        LayoutManager layout;
        layout.setPreset(LayoutPreset::VSCode);
        const DockPanel* explorer = layout.findPanel("Explorer");
        const DockPanel* editor = layout.findPanel("Editor");
        const DockPanel* panel = layout.findPanel("Panel");
        assert(explorer && editor && panel);
        assert(explorer->direction == DockDirection::Left);
        assert(editor->direction == DockDirection::Center);
        assert(panel->direction == DockDirection::Bottom);
        assert(approx(explorer->sizeRatio, 0.20f));
        assert(approx(editor->sizeRatio, 0.60f));
        assert(approx(panel->sizeRatio, 0.20f));
        std::cout << "Test 5 PASS: VSCode layout preset matches expected docking data" << std::endl;
        ++passed;
    }

    // 6. Settings defaultBufferMode preference is respected by caller mapping.
    {
        SettingsManager settings;
        assert(settings.getDefaultBufferMode() == "text");

        settings.setDefaultBufferMode("structured");
        assert(settings.getDefaultBufferMode() == "structured");

        auto modeFromSetting = [](const std::string& value) {
            return value == "structured"
                ? BufferManager::BufferMode::Structured
                : BufferManager::BufferMode::Text;
        };

        BufferManager bm;
        bm.openBuffer("c.py", "", "python", modeFromSetting(settings.getDefaultBufferMode()));
        assert(bm.getBufferMode("c.py") == BufferManager::BufferMode::Structured);
        std::cout << "Test 6 PASS: defaultBufferMode setting drives buffer default mode" << std::endl;
        ++passed;
    }

    std::cout << "\n=== Step 244 Results: " << passed << " passed, " << failed
              << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}

