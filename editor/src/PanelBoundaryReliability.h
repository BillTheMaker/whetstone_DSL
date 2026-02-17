#pragma once
// Step 510: Panel Boundaries, Resizers, and Hit-Target Corrections
// Headless geometry model for deterministic resize hit-testing and split-bar
// correction without GUI dependencies.

#include <string>
#include <vector>
#include <algorithm>

struct PanelRect {
    std::string id;
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct SplitBar {
    std::string id;
    bool vertical = true; // vertical split bar between left/right panes
    float pos = 0.0f;     // x when vertical, y when horizontal
    float thickness = 6.0f;
    std::string firstPanel;
    std::string secondPanel;
};

struct HitTargetResult {
    bool hit = false;
    bool resizeHit = false;
    std::string targetId;
    std::string reason;
};

struct BoundaryFixResult {
    std::vector<PanelRect> panels;
    std::vector<SplitBar> bars;
    std::vector<std::string> notes;
};

class PanelBoundaryReliability {
public:
    static HitTargetResult resolveClick(float mouseX,
                                        float mouseY,
                                        const std::vector<PanelRect>& panels,
                                        const std::vector<SplitBar>& bars,
                                        float resizePriorityMargin = 2.0f) {
        HitTargetResult out;

        for (const auto& bar : bars) {
            float half = bar.thickness * 0.5f + resizePriorityMargin;
            bool inBand = bar.vertical
                ? (mouseX >= bar.pos - half && mouseX <= bar.pos + half)
                : (mouseY >= bar.pos - half && mouseY <= bar.pos + half);
            if (inBand) {
                out.hit = true;
                out.resizeHit = true;
                out.targetId = bar.id;
                out.reason = "resize-priority";
                return out;
            }
        }

        for (const auto& p : panels) {
            if (contains(p, mouseX, mouseY)) {
                out.hit = true;
                out.resizeHit = false;
                out.targetId = p.id;
                out.reason = "panel-content";
                return out;
            }
        }

        out.reason = "none";
        return out;
    }

    static BoundaryFixResult normalizeGeometry(const std::vector<PanelRect>& inputPanels,
                                               const std::vector<SplitBar>& inputBars,
                                               float minPanelSize = 120.0f,
                                               float minBarThickness = 5.0f,
                                               float maxBarThickness = 14.0f) {
        BoundaryFixResult out;
        out.panels = inputPanels;
        out.bars = inputBars;

        for (auto& p : out.panels) {
            if (p.w < minPanelSize) {
                p.w = minPanelSize;
                out.notes.push_back("panel-width-clamped:" + p.id);
            }
            if (p.h < minPanelSize) {
                p.h = minPanelSize;
                out.notes.push_back("panel-height-clamped:" + p.id);
            }
        }

        for (auto& b : out.bars) {
            float old = b.thickness;
            b.thickness = std::max(minBarThickness,
                                   std::min(maxBarThickness, b.thickness));
            if (b.thickness != old) {
                out.notes.push_back("bar-thickness-clamped:" + b.id);
            }
        }

        return out;
    }

    static SplitBar keyboardResize(const SplitBar& bar,
                                   float delta,
                                   float minPos,
                                   float maxPos,
                                   bool fineStep) {
        SplitBar out = bar;
        float scale = fineStep ? 0.5f : 1.0f;
        out.pos = std::max(minPos, std::min(maxPos, bar.pos + delta * scale));
        return out;
    }

    static std::vector<std::string> navigationOrderByBarProximity(
            const std::vector<PanelRect>& panels,
            const SplitBar& bar) {
        std::vector<std::pair<float, std::string>> ranked;
        for (const auto& p : panels) {
            float center = bar.vertical ? (p.x + p.w * 0.5f) : (p.y + p.h * 0.5f);
            float dist = std::abs(center - bar.pos);
            ranked.push_back({dist, p.id});
        }
        std::sort(ranked.begin(), ranked.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        std::vector<std::string> ids;
        for (const auto& r : ranked) ids.push_back(r.second);
        return ids;
    }

private:
    static bool contains(const PanelRect& p, float mx, float my) {
        return mx >= p.x && mx <= p.x + p.w && my >= p.y && my <= p.y + p.h;
    }
};
