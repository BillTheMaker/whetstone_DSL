#pragma once

#include <string>
#include <algorithm>

struct UIAnimationState {
    std::string activeTabPath;
    double tabSwitchTime = 0.0;

    void recordTabSwitch(const std::string& path, double nowSeconds) {
        if (path == activeTabPath) return;
        activeTabPath = path;
        tabSwitchTime = nowSeconds;
    }

    float tabFadeAlpha(const std::string& path,
                       double nowSeconds,
                       bool reduceMotion,
                       float durationSeconds = 0.15f) const {
        if (reduceMotion) return 1.0f;
        if (path.empty() || path != activeTabPath) return 1.0f;
        double elapsed = nowSeconds - tabSwitchTime;
        float t = (durationSeconds <= 0.0f) ? 1.0f : (float)(elapsed / durationSeconds);
        return std::max(0.0f, std::min(1.0f, t));
    }
};
