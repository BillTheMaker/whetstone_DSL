#pragma once
// Step 520: Completion Overlay UX Hardening

#include <string>
#include <algorithm>

struct OverlayRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

struct CompletionOverlayState {
    bool visible = false;
    bool dismissed = false;
    int selectedIndex = 0;
    std::string lastTriggerToken;
    OverlayRect rect;
};

class CompletionOverlayUXHardening {
public:
    static OverlayRect placeOverlayNearCursor(float cursorX,
                                              float cursorY,
                                              float lineHeight,
                                              float viewportW,
                                              float viewportH,
                                              float overlayW,
                                              float overlayH) {
        OverlayRect r;
        r.w = overlayW;
        r.h = overlayH;
        r.x = cursorX;
        r.y = cursorY + lineHeight;

        if (r.x + r.w > viewportW) r.x = clampMin(viewportW - r.w, 0.0f);
        if (r.y + r.h > viewportH) r.y = clampMin(cursorY - r.h - 4.0f, 0.0f);
        return r;
    }

    static CompletionOverlayState onDismiss(const CompletionOverlayState& in) {
        CompletionOverlayState out = in;
        out.visible = false;
        out.dismissed = true;
        return out;
    }

    static CompletionOverlayState onTrigger(const CompletionOverlayState& in,
                                            const std::string& token,
                                            bool semanticTriggerChanged) {
        CompletionOverlayState out = in;
        if (semanticTriggerChanged || token != in.lastTriggerToken) {
            out.dismissed = false;
        }
        out.lastTriggerToken = token;
        out.visible = !out.dismissed;
        return out;
    }

    static CompletionOverlayState onPointerSelect(const CompletionOverlayState& in,
                                                  int index,
                                                  int itemCount) {
        CompletionOverlayState out = in;
        out.selectedIndex = clampIndex(index, itemCount);
        out.visible = itemCount > 0;
        return out;
    }

    static CompletionOverlayState onKeyNav(const CompletionOverlayState& in,
                                           int delta,
                                           int itemCount) {
        CompletionOverlayState out = in;
        if (itemCount <= 0) {
            out.selectedIndex = 0;
            out.visible = false;
            return out;
        }
        out.selectedIndex = wrapIndex(in.selectedIndex + delta, itemCount);
        out.visible = true;
        return out;
    }

private:
    static float clampMin(float v, float lo) {
        return v < lo ? lo : v;
    }

    static int clampIndex(int idx, int count) {
        if (count <= 0) return 0;
        if (idx < 0) return 0;
        if (idx >= count) return count - 1;
        return idx;
    }

    static int wrapIndex(int idx, int count) {
        if (count <= 0) return 0;
        int m = idx % count;
        if (m < 0) m += count;
        return m;
    }
};
