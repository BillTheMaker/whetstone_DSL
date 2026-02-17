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

        if (r.x + r.w > viewportW) r.x = std::max(0.0f, viewportW - r.w);
        if (r.y + r.h > viewportH) r.y = std::max(0.0f, cursorY - r.h - 4.0f);
        if (r.y < 0.0f) r.y = 0.0f;
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
        out.selectedIndex = std::max(0, std::min(itemCount - 1, index));
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
        int idx = in.selectedIndex + delta;
        while (idx < 0) idx += itemCount;
        while (idx >= itemCount) idx -= itemCount;
        out.selectedIndex = idx;
        out.visible = true;
        return out;
    }
};
