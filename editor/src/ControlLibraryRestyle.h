#pragma once
// Step 517: Control Library Restyle
// Shared control style semantics for depth, pressed state clarity, and DPI
// consistency across core widgets.

#include <string>
#include <algorithm>

struct ControlStyle {
    float cornerRadius = 0.0f;
    float borderWidth = 1.0f;
    float depthIdle = 0.0f;
    float depthPressed = 0.0f;
    float labelContrast = 1.0f;
    float iconContrast = 1.0f;
    float strokePx = 1.0f;
};

enum class ControlKind {
    Button,
    Toggle,
    Tab,
    Menu,
    Dropdown,
    Checkbox
};

class ControlLibraryRestyle {
public:
    static ControlStyle styleFor(ControlKind kind, float dpiScale, bool pressed) {
        float s = std::max(0.75f, std::min(3.0f, dpiScale));
        ControlStyle c;
        c.cornerRadius = baseRadius(kind) * s;
        c.borderWidth = 1.0f * s;
        c.depthIdle = baseDepth(kind) * s;
        c.depthPressed = (baseDepth(kind) + 1.4f) * s;
        c.labelContrast = 6.0f;
        c.iconContrast = 4.0f;
        c.strokePx = std::max(1.0f, 1.2f * s);
        if (pressed) {
            c.depthIdle = c.depthPressed;
        }
        return c;
    }

    static bool hasClearPressedSignal(const ControlStyle& idle,
                                      const ControlStyle& pressed) {
        return pressed.depthIdle > idle.depthIdle + 0.6f;
    }

    static bool validContrast(const ControlStyle& c) {
        return c.labelContrast >= 4.5f && c.iconContrast >= 3.0f;
    }

    static bool dpiStable(const ControlStyle& lo, const ControlStyle& hi) {
        return hi.cornerRadius > lo.cornerRadius && hi.strokePx >= lo.strokePx;
    }

private:
    static float baseRadius(ControlKind kind) {
        switch (kind) {
            case ControlKind::Button: return 4.0f;
            case ControlKind::Toggle: return 7.0f;
            case ControlKind::Tab: return 3.0f;
            case ControlKind::Menu: return 2.0f;
            case ControlKind::Dropdown: return 4.0f;
            case ControlKind::Checkbox: return 2.0f;
        }
        return 3.0f;
    }

    static float baseDepth(ControlKind kind) {
        switch (kind) {
            case ControlKind::Button: return 1.2f;
            case ControlKind::Toggle: return 1.6f;
            case ControlKind::Tab: return 1.0f;
            case ControlKind::Menu: return 0.8f;
            case ControlKind::Dropdown: return 1.1f;
            case ControlKind::Checkbox: return 0.9f;
        }
        return 1.0f;
    }
};
