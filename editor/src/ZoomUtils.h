#pragma once
// Step 119: Zoom utilities

inline int clampFontSize(int size, int minSize = 12, int maxSize = 24) {
    if (size < minSize) return minSize;
    if (size > maxSize) return maxSize;
    return size;
}

inline int zoomPercent(int fontSize, float baseFontSize) {
    if (baseFontSize <= 0.0f) return 100;
    float pct = (fontSize / baseFontSize) * 100.0f;
    return (int)(pct + 0.5f);
}
