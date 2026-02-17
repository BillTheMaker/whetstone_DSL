#pragma once
// Step 516: Signature Visual Identity Pack
// Encodes black-surface gradient treatment, blade-edge accents, and
// electric-blue logic link styling.

#include <string>
#include <vector>
#include <algorithm>

struct GradientStop {
    float t = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct EdgeAccent {
    std::string edge; // left/right/top/bottom
    float thickness = 1.0f;
    float intensity = 0.0f;
    bool active = false;
};

struct LogicLinkStyle {
    float width = 1.0f;
    float glow = 0.0f;
    float r = 0.15f;
    float g = 0.56f;
    float b = 0.95f;
    float alpha = 1.0f;
};

class SignatureVisualIdentity {
public:
    static std::vector<GradientStop> blackSurfaceGradient(bool elevated) {
        if (elevated) {
            return {
                {0.0f, 0.02f, 0.02f, 0.02f, 1.0f},
                {0.45f, 0.08f, 0.08f, 0.08f, 1.0f},
                {1.0f, 0.14f, 0.14f, 0.14f, 1.0f}
            };
        }
        return {
            {0.0f, 0.01f, 0.01f, 0.01f, 1.0f},
            {0.60f, 0.05f, 0.05f, 0.05f, 1.0f},
            {1.0f, 0.09f, 0.09f, 0.09f, 1.0f}
        };
    }

    static EdgeAccent bladeEdge(const std::string& edge, bool active, float emphasis) {
        EdgeAccent a;
        a.edge = edge;
        a.active = active;
        a.thickness = active ? 2.0f : 1.0f;
        a.intensity = active ? std::max(0.2f, std::min(1.0f, emphasis)) : 0.12f;
        return a;
    }

    static LogicLinkStyle electricBlueLink(bool highlighted, float distanceNorm) {
        LogicLinkStyle s;
        s.width = highlighted ? 2.4f : 1.4f;
        s.glow = highlighted ? 0.80f : 0.35f;
        float fade = std::max(0.35f, 1.0f - std::max(0.0f, std::min(1.0f, distanceNorm)) * 0.45f);
        s.alpha = fade;
        return s;
    }

    static bool isElectricBlue(const LogicLinkStyle& s) {
        return s.b > s.r && s.b > s.g && s.g > s.r;
    }

    static float revealDurationMs(bool reducedMotion) {
        return reducedMotion ? 80.0f : 220.0f;
    }

    static float staggerDelayMs(int index, bool reducedMotion) {
        if (reducedMotion) return 0.0f;
        return std::max(0, index) * 24.0f;
    }
};
