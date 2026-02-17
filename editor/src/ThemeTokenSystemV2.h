#pragma once
// Step 515: Theme Token System v2
// Semantic token set with derivation support and contrast checks.

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <cmath>

struct ThemeColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct ThemeTokensV2 {
    ThemeColor bgBase;
    ThemeColor bgLayer1;
    ThemeColor bgLayer2;
    ThemeColor textPrimary;
    ThemeColor textSecondary;
    ThemeColor accentPrimary;
    ThemeColor accentDanger;
    float radiusSmall = 2.0f;
    float radiusMedium = 6.0f;
    float borderThin = 1.0f;
    float borderThick = 2.0f;
    float elevationLow = 2.0f;
    float elevationHigh = 8.0f;
};

class ThemeTokenSystemV2 {
public:
    static ThemeTokensV2 baseBlackStone() {
        return {
            {0.05f, 0.05f, 0.05f, 1.0f},
            {0.10f, 0.10f, 0.10f, 1.0f},
            {0.16f, 0.16f, 0.16f, 1.0f},
            {0.92f, 0.92f, 0.92f, 1.0f},
            {0.70f, 0.70f, 0.70f, 1.0f},
            {0.15f, 0.56f, 0.95f, 1.0f},
            {0.86f, 0.30f, 0.30f, 1.0f},
            2.0f, 6.0f, 1.0f, 2.0f, 2.0f, 8.0f
        };
    }

    static ThemeTokensV2 deriveContrastVariant(const ThemeTokensV2& in,
                                                float textBoost,
                                                float layerLift) {
        ThemeTokensV2 out = in;
        out.textPrimary = lighten(out.textPrimary, textBoost);
        out.textSecondary = lighten(out.textSecondary, textBoost * 0.7f);
        out.bgLayer1 = lighten(out.bgLayer1, layerLift);
        out.bgLayer2 = lighten(out.bgLayer2, layerLift);
        return out;
    }

    static std::map<std::string, float> spacingScale(float base = 4.0f) {
        return {
            {"xs", base},
            {"sm", base * 2.0f},
            {"md", base * 3.0f},
            {"lg", base * 4.0f},
            {"xl", base * 6.0f}
        };
    }

    static bool hasHardcodedPurple(const ThemeTokensV2& t) {
        // Reject old default purple bias in primary accents.
        return (t.accentPrimary.r > 0.40f && t.accentPrimary.b > 0.70f);
    }

    static float contrastTextVsBg(const ThemeColor& text, const ThemeColor& bg) {
        float l1 = luminance(text);
        float l2 = luminance(bg);
        if (l1 < l2) std::swap(l1, l2);
        return (l1 + 0.05f) / (l2 + 0.05f);
    }

    static bool validateAA(const ThemeTokensV2& t) {
        return contrastTextVsBg(t.textPrimary, t.bgBase) >= 4.5f &&
               contrastTextVsBg(t.textSecondary, t.bgLayer1) >= 3.0f;
    }

private:
    static ThemeColor lighten(ThemeColor c, float delta) {
        c.r = clamp01(c.r + delta);
        c.g = clamp01(c.g + delta);
        c.b = clamp01(c.b + delta);
        return c;
    }

    static float clamp01(float v) {
        return std::max(0.0f, std::min(1.0f, v));
    }

    static float linear(float c) {
        if (c <= 0.03928f) return c / 12.92f;
        return std::pow((c + 0.055f) / 1.055f, 2.4f);
    }

    static float luminance(const ThemeColor& c) {
        return 0.2126f * linear(c.r) + 0.7152f * linear(c.g) + 0.0722f * linear(c.b);
    }
};
