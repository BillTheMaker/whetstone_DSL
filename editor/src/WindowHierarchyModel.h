#pragma once
// Step 512: Window Chrome + Hierarchy Pass
// Headless hierarchy model for surface classification and deterministic
// elevation/layering semantics between work surfaces, tool surfaces, and overlays.

#include <string>
#include <vector>
#include <algorithm>

struct SurfaceStyle {
    float borderAlpha = 0.0f;
    float shadowAlpha = 0.0f;
    float edgeHighlightAlpha = 0.0f;
    int zIndex = 0;
};

enum class SurfaceRole {
    Work,
    Tool,
    Overlay,
    Modal,
    Notification
};

struct SurfaceNode {
    std::string id;
    SurfaceRole role = SurfaceRole::Work;
    bool modal = false;
    bool interactive = true;
    int depth = 0;
    SurfaceStyle style;
};

struct HierarchyReport {
    bool valid = true;
    std::vector<SurfaceNode> ordered;
    std::vector<std::string> violations;
};

class WindowHierarchyModel {
public:
    static SurfaceStyle styleForRole(SurfaceRole role, int depth) {
        SurfaceStyle s;
        switch (role) {
            case SurfaceRole::Work:
                s.borderAlpha = 0.28f;
                s.shadowAlpha = 0.05f;
                s.edgeHighlightAlpha = 0.10f;
                s.zIndex = 100 + depth;
                break;
            case SurfaceRole::Tool:
                s.borderAlpha = 0.34f;
                s.shadowAlpha = 0.10f;
                s.edgeHighlightAlpha = 0.16f;
                s.zIndex = 200 + depth;
                break;
            case SurfaceRole::Overlay:
                s.borderAlpha = 0.40f;
                s.shadowAlpha = 0.20f;
                s.edgeHighlightAlpha = 0.24f;
                s.zIndex = 300 + depth;
                break;
            case SurfaceRole::Modal:
                s.borderAlpha = 0.46f;
                s.shadowAlpha = 0.28f;
                s.edgeHighlightAlpha = 0.30f;
                s.zIndex = 400 + depth;
                break;
            case SurfaceRole::Notification:
                s.borderAlpha = 0.42f;
                s.shadowAlpha = 0.18f;
                s.edgeHighlightAlpha = 0.26f;
                s.zIndex = 350 + depth;
                break;
        }
        return s;
    }

    static HierarchyReport validateAndOrder(const std::vector<SurfaceNode>& input) {
        HierarchyReport out;
        out.ordered = input;

        for (auto& n : out.ordered) {
            n.style = styleForRole(n.role, n.depth);
        }

        std::sort(out.ordered.begin(), out.ordered.end(),
                  [](const SurfaceNode& a, const SurfaceNode& b) {
                      if (a.style.zIndex != b.style.zIndex) return a.style.zIndex < b.style.zIndex;
                      return a.id < b.id;
                  });

        bool seenModal = false;
        for (size_t i = 0; i < out.ordered.size(); ++i) {
            const auto& n = out.ordered[i];
            if (n.role == SurfaceRole::Modal) {
                seenModal = true;
                if (!n.modal) {
                    out.valid = false;
                    out.violations.push_back("modal-flag-missing:" + n.id);
                }
            }
            if (seenModal && n.role == SurfaceRole::Work) {
                out.valid = false;
                out.violations.push_back("work-above-modal:" + n.id);
            }
        }

        for (const auto& n : out.ordered) {
            if (n.role == SurfaceRole::Overlay && n.style.zIndex < 300) {
                out.valid = false;
                out.violations.push_back("overlay-z-too-low:" + n.id);
            }
            if (n.role == SurfaceRole::Tool && n.style.zIndex < 200) {
                out.valid = false;
                out.violations.push_back("tool-z-too-low:" + n.id);
            }
        }

        return out;
    }

    static std::vector<SurfaceNode> normalize(const std::vector<SurfaceNode>& input) {
        std::vector<SurfaceNode> out = input;
        for (auto& n : out) {
            if (n.role == SurfaceRole::Modal) n.modal = true;
            n.style = styleForRole(n.role, n.depth);
        }
        std::sort(out.begin(), out.end(),
                  [](const SurfaceNode& a, const SurfaceNode& b) {
                      return a.style.zIndex < b.style.zIndex;
                  });
        return out;
    }

    static bool blocksInteraction(const SurfaceNode& top, const SurfaceNode& bottom) {
        if (!top.interactive) return false;
        if (top.role == SurfaceRole::Modal && top.style.zIndex > bottom.style.zIndex) return true;
        if (top.role == SurfaceRole::Overlay && top.style.zIndex > bottom.style.zIndex) return true;
        return false;
    }
};
