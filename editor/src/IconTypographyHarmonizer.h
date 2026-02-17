#pragma once
// Step 518: Icon + Typography Contrast Harmonization

#include <string>
#include <vector>
#include <algorithm>

struct TypographyProfile {
    float bodySizePx = 14.0f;
    float headingSizePx = 20.0f;
    float lineHeight = 1.4f;
    float weightBody = 450.0f;
    float weightHeading = 640.0f;
    float textContrast = 1.0f;
};

struct IconProfile {
    float strokePx = 1.5f;
    float contrast = 1.0f;
    float opticalSize = 16.0f;
};

class IconTypographyHarmonizer {
public:
    static TypographyProfile darkSurfaceTypography(float density) {
        float d = std::max(0.8f, std::min(1.4f, density));
        TypographyProfile t;
        t.bodySizePx = 14.0f * d;
        t.headingSizePx = 20.0f * d;
        t.lineHeight = 1.38f;
        t.weightBody = 460.0f;
        t.weightHeading = 650.0f;
        t.textContrast = 6.4f;
        return t;
    }

    static IconProfile iconProfileFor(float density, bool emphasized) {
        float d = std::max(0.8f, std::min(1.4f, density));
        IconProfile i;
        i.strokePx = emphasized ? 1.9f * d : 1.5f * d;
        i.contrast = emphasized ? 4.5f : 3.4f;
        i.opticalSize = 16.0f * d;
        return i;
    }

    static bool harmonized(const TypographyProfile& t, const IconProfile& i) {
        return t.textContrast >= 4.5f && i.contrast >= 3.0f &&
               i.strokePx >= 1.0f && t.headingSizePx > t.bodySizePx;
    }

    static float rhythmScore(const TypographyProfile& t,
                             const std::vector<float>& panelDensities) {
        if (panelDensities.empty()) return 0.0f;
        float spread = *std::max_element(panelDensities.begin(), panelDensities.end()) -
                       *std::min_element(panelDensities.begin(), panelDensities.end());
        float base = 1.0f - std::min(1.0f, spread / 12.0f);
        float typeBonus = (t.lineHeight >= 1.3f && t.lineHeight <= 1.6f) ? 0.2f : 0.0f;
        return std::max(0.0f, std::min(1.0f, base + typeBonus));
    }
};
