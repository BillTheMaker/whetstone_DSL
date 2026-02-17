#pragma once
// Step 511: Interaction State Model
// Shared token-driven interaction state logic for hover/focus/active/pressed/
// disabled with deterministic style selection and contrast validation.

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>

enum class InteractionState {
    Idle,
    Hover,
    Focus,
    Active,
    Pressed,
    Disabled
};

struct RGBA {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct InteractionTokens {
    RGBA surface;
    RGBA surfaceHover;
    RGBA surfaceFocus;
    RGBA surfaceActive;
    RGBA surfacePressed;
    RGBA surfaceDisabled;
    RGBA text;
    RGBA textDisabled;
    RGBA border;
    RGBA focusRing;
};

struct InteractionSnapshot {
    bool hovered = false;
    bool focused = false;
    bool active = false;
    bool pressed = false;
    bool disabled = false;
};

struct StyleResolution {
    InteractionState state = InteractionState::Idle;
    RGBA surface;
    RGBA text;
    RGBA border;
    bool showFocusRing = false;
    float contrastRatio = 1.0f;
    bool contrastPassAA = false;
};

class InteractionStateModel {
public:
    static InteractionTokens defaultDarkTokens() {
        return {
            {0.10f, 0.10f, 0.10f, 1.0f},
            {0.15f, 0.15f, 0.15f, 1.0f},
            {0.12f, 0.15f, 0.20f, 1.0f},
            {0.16f, 0.16f, 0.18f, 1.0f},
            {0.20f, 0.20f, 0.22f, 1.0f},
            {0.12f, 0.12f, 0.12f, 0.6f},
            {0.90f, 0.90f, 0.90f, 1.0f},
            {0.55f, 0.55f, 0.55f, 1.0f},
            {0.30f, 0.30f, 0.30f, 1.0f},
            {0.18f, 0.56f, 0.95f, 1.0f}
        };
    }

    static InteractionState resolveState(const InteractionSnapshot& s) {
        if (s.disabled) return InteractionState::Disabled;
        if (s.pressed) return InteractionState::Pressed;
        if (s.active) return InteractionState::Active;
        if (s.focused) return InteractionState::Focus;
        if (s.hovered) return InteractionState::Hover;
        return InteractionState::Idle;
    }

    static StyleResolution resolveStyle(const InteractionSnapshot& snapshot,
                                        const InteractionTokens& tokens) {
        StyleResolution out;
        out.state = resolveState(snapshot);
        out.border = tokens.border;

        switch (out.state) {
            case InteractionState::Idle:
                out.surface = tokens.surface;
                out.text = tokens.text;
                break;
            case InteractionState::Hover:
                out.surface = tokens.surfaceHover;
                out.text = tokens.text;
                break;
            case InteractionState::Focus:
                out.surface = tokens.surfaceFocus;
                out.text = tokens.text;
                out.showFocusRing = true;
                break;
            case InteractionState::Active:
                out.surface = tokens.surfaceActive;
                out.text = tokens.text;
                break;
            case InteractionState::Pressed:
                out.surface = tokens.surfacePressed;
                out.text = tokens.text;
                break;
            case InteractionState::Disabled:
                out.surface = tokens.surfaceDisabled;
                out.text = tokens.textDisabled;
                break;
        }

        out.contrastRatio = contrast(out.text, out.surface);
        out.contrastPassAA = out.contrastRatio >= 4.5f;
        return out;
    }

    static std::map<std::string, StyleResolution> resolveForAllStates(
            const InteractionTokens& tokens) {
        std::map<std::string, StyleResolution> out;
        out["idle"] = resolveStyle({}, tokens);
        out["hover"] = resolveStyle({true, false, false, false, false}, tokens);
        out["focus"] = resolveStyle({false, true, false, false, false}, tokens);
        out["active"] = resolveStyle({true, true, true, false, false}, tokens);
        out["pressed"] = resolveStyle({true, true, true, true, false}, tokens);
        out["disabled"] = resolveStyle({false, false, false, false, true}, tokens);
        return out;
    }

    static std::vector<std::string> contrastFailures(const InteractionTokens& tokens) {
        std::vector<std::string> failures;
        auto all = resolveForAllStates(tokens);
        for (const auto& [name, style] : all) {
            if (!style.contrastPassAA) {
                failures.push_back(name + ":" + std::to_string(style.contrastRatio));
            }
        }
        return failures;
    }

private:
    static float channel(float c) {
        if (c <= 0.03928f) return c / 12.92f;
        return std::pow((c + 0.055f) / 1.055f, 2.4f);
    }

    static float luminance(const RGBA& c) {
        float r = channel(std::max(0.0f, std::min(1.0f, c.r)));
        float g = channel(std::max(0.0f, std::min(1.0f, c.g)));
        float b = channel(std::max(0.0f, std::min(1.0f, c.b)));
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    static float contrast(const RGBA& fg, const RGBA& bg) {
        float l1 = luminance(fg);
        float l2 = luminance(bg);
        if (l1 < l2) std::swap(l1, l2);
        return (l1 + 0.05f) / (l2 + 0.05f);
    }
};
