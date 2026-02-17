#pragma once
// Step 513: Accessibility Baseline Pass
// Keyboard traversal, focus-ring visibility, and contrast checks for baseline
// accessibility requirements in a headless validation model.

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>

struct AccessibilityNode {
    std::string id;
    bool focusable = true;
    bool visible = true;
    bool enabled = true;
    int tabOrder = 0;
};

struct FocusRingStyle {
    float thickness = 0.0f;
    float alpha = 0.0f;
    float contrast = 1.0f;
};

struct AccessibilityReport {
    bool pass = true;
    std::vector<std::string> failures;
    std::vector<std::string> traversalOrder;
};

class AccessibilityBaseline {
public:
    static std::vector<std::string> keyboardTraversalOrder(
            const std::vector<AccessibilityNode>& nodes) {
        std::vector<AccessibilityNode> filtered;
        for (const auto& n : nodes) {
            if (n.visible && n.enabled && n.focusable) filtered.push_back(n);
        }
        std::sort(filtered.begin(), filtered.end(),
                  [](const auto& a, const auto& b) { return a.tabOrder < b.tabOrder; });
        std::vector<std::string> out;
        for (const auto& n : filtered) out.push_back(n.id);
        return out;
    }

    static FocusRingStyle resolveFocusRing(bool focused,
                                           bool highContrast,
                                           float bgLuminance,
                                           float ringLuminance) {
        FocusRingStyle s;
        if (!focused) return s;
        s.thickness = highContrast ? 3.0f : 2.0f;
        s.alpha = highContrast ? 1.0f : 0.9f;
        s.contrast = contrast(bgLuminance, ringLuminance);
        return s;
    }

    static bool focusRingVisible(const FocusRingStyle& s) {
        return s.thickness >= 2.0f && s.alpha >= 0.85f && s.contrast >= 3.0f;
    }

    static AccessibilityReport validate(const std::vector<AccessibilityNode>& nodes,
                                        const FocusRingStyle& ring,
                                        float textBgContrast,
                                        float iconBgContrast) {
        AccessibilityReport r;
        r.traversalOrder = keyboardTraversalOrder(nodes);

        if (r.traversalOrder.empty()) {
            r.pass = false;
            r.failures.push_back("no-focusable-controls");
        }

        if (!focusRingVisible(ring)) {
            r.pass = false;
            r.failures.push_back("focus-ring-not-visible");
        }

        if (textBgContrast < 4.5f) {
            r.pass = false;
            r.failures.push_back("text-contrast-low");
        }

        if (iconBgContrast < 3.0f) {
            r.pass = false;
            r.failures.push_back("icon-contrast-low");
        }

        return r;
    }

private:
    static float contrast(float a, float b) {
        float l1 = std::max(a, b);
        float l2 = std::min(a, b);
        return (l1 + 0.05f) / (l2 + 0.05f);
    }
};
