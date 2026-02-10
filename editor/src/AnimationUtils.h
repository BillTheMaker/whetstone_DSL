#pragma once

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

namespace AnimationUtils {
inline float clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float easeOutCubic(float t) {
    t = clamp01(t);
    float inv = 1.0f - t;
    return 1.0f - inv * inv * inv;
}

inline float easeInOut(float t) {
    t = clamp01(t);
    return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

inline float approach(float current, float target, float delta) {
    if (current < target) return std::min(target, current + delta);
    return std::max(target, current - delta);
}

inline float blinkAlpha(double nowSeconds, float rateHz, bool reduceMotion) {
    if (reduceMotion || rateHz <= 0.0f) return 1.0f;
    const float phase = std::fmod((float)nowSeconds * rateHz, 1.0f);
    const float wave = 0.5f + 0.5f * std::sin(phase * 6.2831853f);
    return clamp01(wave);
}

inline float pulseAlpha(double nowSeconds, double startSeconds, float duration, bool reduceMotion) {
    if (reduceMotion) return 1.0f;
    if (startSeconds <= 0.0) return 0.0f;
    float t = (float)((nowSeconds - startSeconds) / std::max(0.001f, duration));
    if (t < 0.0f || t > 1.0f) return 0.0f;
    float eased = easeOutCubic(t);
    return 1.0f - eased;
}

struct TooltipState {
    double hoverStart = -1.0;
    double lastUpdate = 0.0;
    float alpha = 0.0f;
    bool hovered = false;
};

inline std::unordered_map<std::string, TooltipState>& tooltipStates() {
    static std::unordered_map<std::string, TooltipState> states;
    return states;
}

inline float tooltipAlpha(const std::string& id,
                          bool hovered,
                          double nowSeconds,
                          bool reduceMotion,
                          float delay = 0.45f,
                          float fadeIn = 0.12f,
                          float fadeOut = 0.12f) {
    if (reduceMotion) return hovered ? 1.0f : 0.0f;
    auto& state = tooltipStates()[id];
    if (state.lastUpdate <= 0.0) state.lastUpdate = nowSeconds;
    const float dt = (float)(nowSeconds - state.lastUpdate);
    state.lastUpdate = nowSeconds;

    if (hovered) {
        if (!state.hovered) {
            state.hovered = true;
            state.hoverStart = nowSeconds;
        }
        float target = (nowSeconds - state.hoverStart) >= delay ? 1.0f : 0.0f;
        float step = (target > state.alpha)
            ? (dt / std::max(0.001f, fadeIn))
            : (dt / std::max(0.001f, fadeOut));
        state.alpha = approach(state.alpha, target, step);
    } else {
        if (state.hovered) {
            state.hovered = false;
        }
        float step = dt / std::max(0.001f, fadeOut);
        state.alpha = approach(state.alpha, 0.0f, step);
    }
    return clamp01(state.alpha);
}

struct PanelState {
    bool visible = false;
    double toggleTime = 0.0;
    bool animating = false;
};

inline std::unordered_map<std::string, PanelState>& panelStates() {
    static std::unordered_map<std::string, PanelState> states;
    return states;
}

inline bool panelTransition(const std::string& id,
                            bool targetVisible,
                            double nowSeconds,
                            bool reduceMotion,
                            float duration,
                            float slideDistance,
                            float& outAlpha,
                            float& outOffset) {
    auto& state = panelStates()[id];
    if (state.visible != targetVisible) {
        state.visible = targetVisible;
        state.toggleTime = nowSeconds;
        state.animating = true;
    }
    if (reduceMotion) {
        outAlpha = targetVisible ? 1.0f : 0.0f;
        outOffset = 0.0f;
        state.animating = false;
        return targetVisible;
    }
    float t = (float)((nowSeconds - state.toggleTime) / std::max(0.001f, duration));
    if (t >= 1.0f) {
        t = 1.0f;
        state.animating = false;
    }
    if (state.visible) {
        outAlpha = easeOutCubic(t);
        outOffset = (1.0f - outAlpha) * slideDistance;
    } else {
        outAlpha = 1.0f - easeOutCubic(t);
        outOffset = (1.0f - outAlpha) * slideDistance;
    }
    return state.visible || state.animating;
}
} // namespace AnimationUtils
