#pragma once

#include "imgui.h"
#include "ThemeEngine.h"

#include <string>

enum class FileIconType {
    Python,
    Cpp,
    JavaScript,
    TypeScript,
    Rust,
    Go,
    Java,
    Elisp,
    Json,
    Text,
    Folder
};

enum class PanelIconType {
    Explorer,
    Outline,
    Terminal,
    Agents,
    Dependencies,
    Library,
    Settings,
    Search
};

enum class DiagnosticIconType {
    Error,
    Warning,
    Info,
    Hint
};

enum class AnnotationIconType {
    Ownership,
    Borrow,
    Shared,
    Cache,
    Compute
};

struct IconSet {
    static float scaleFromZoom(float fontSize, float baseFontSize) {
        if (baseFontSize <= 0.0f) return 1.0f;
        return fontSize / baseFontSize;
    }

    static ImU32 fileColor(FileIconType type) {
        switch (type) {
        case FileIconType::Python:
            return ThemeEngine::instance().editorColor("icon_python", IM_COL32(53, 114, 165, 255));
        case FileIconType::Cpp:
            return ThemeEngine::instance().editorColor("icon_cpp", IM_COL32(86, 156, 214, 255));
        case FileIconType::JavaScript:
            return ThemeEngine::instance().editorColor("icon_js", IM_COL32(240, 219, 79, 255));
        case FileIconType::TypeScript:
            return ThemeEngine::instance().editorColor("icon_ts", IM_COL32(48, 120, 198, 255));
        case FileIconType::Rust:
            return ThemeEngine::instance().editorColor("icon_rust", IM_COL32(222, 165, 132, 255));
        case FileIconType::Go:
            return ThemeEngine::instance().editorColor("icon_go", IM_COL32(0, 173, 216, 255));
        case FileIconType::Java:
            return ThemeEngine::instance().editorColor("icon_java", IM_COL32(176, 114, 25, 255));
        case FileIconType::Elisp:
            return ThemeEngine::instance().editorColor("icon_elisp", IM_COL32(128, 88, 170, 255));
        case FileIconType::Json:
            return ThemeEngine::instance().editorColor("icon_json", IM_COL32(200, 180, 120, 255));
        case FileIconType::Folder:
            return ThemeEngine::instance().editorColor("icon_folder", IM_COL32(201, 163, 86, 255));
        case FileIconType::Text:
        default:
            return ThemeEngine::instance().editorColor("icon_text", IM_COL32(180, 180, 180, 255));
        }
    }

    static ImU32 panelColor(PanelIconType type) {
        ImU32 base = ThemeEngine::instance().editorColor("icon_panel", IM_COL32(150, 150, 150, 255));
        switch (type) {
        case PanelIconType::Terminal:
            return ThemeEngine::instance().editorColor("icon_terminal", IM_COL32(0, 200, 120, 255));
        case PanelIconType::Agents:
            return ThemeEngine::instance().editorColor("icon_agents", IM_COL32(140, 180, 255, 255));
        case PanelIconType::Dependencies:
            return ThemeEngine::instance().editorColor("icon_deps", IM_COL32(230, 190, 120, 255));
        default:
            return base;
        }
    }

    static ImU32 diagnosticColor(DiagnosticIconType type) {
        switch (type) {
        case DiagnosticIconType::Error:
            return ThemeEngine::instance().editorColor("diag_error", IM_COL32(220, 80, 80, 255));
        case DiagnosticIconType::Warning:
            return ThemeEngine::instance().editorColor("diag_warning", IM_COL32(220, 160, 60, 255));
        case DiagnosticIconType::Info:
            return ThemeEngine::instance().editorColor("diag_info", IM_COL32(90, 160, 220, 255));
        case DiagnosticIconType::Hint:
        default:
            return ThemeEngine::instance().editorColor("diag_hint", IM_COL32(160, 200, 120, 255));
        }
    }

    static ImU32 annotationColor(AnnotationIconType type) {
        switch (type) {
        case AnnotationIconType::Ownership:
            return ThemeEngine::instance().editorColor("icon_ownership", IM_COL32(230, 120, 90, 255));
        case AnnotationIconType::Borrow:
            return ThemeEngine::instance().editorColor("icon_borrow", IM_COL32(120, 180, 230, 255));
        case AnnotationIconType::Shared:
            return ThemeEngine::instance().editorColor("icon_shared", IM_COL32(150, 200, 140, 255));
        case AnnotationIconType::Cache:
            return ThemeEngine::instance().editorColor("icon_cache", IM_COL32(210, 190, 120, 255));
        case AnnotationIconType::Compute:
        default:
            return ThemeEngine::instance().editorColor("icon_compute", IM_COL32(200, 140, 220, 255));
        }
    }

    static void drawFileIcon(ImDrawList* draw, ImVec2 center, float size,
                             FileIconType type, float scale = 1.0f) {
        float s = size * scale;
        ImU32 color = fileColor(type);
        ImVec2 min(center.x - s * 0.5f, center.y - s * 0.6f);
        ImVec2 max(center.x + s * 0.5f, center.y + s * 0.6f);
        draw->AddRectFilled(min, max, color, 3.0f);
        ImVec2 foldA(max.x - s * 0.25f, min.y);
        ImVec2 foldB(max.x, min.y + s * 0.25f);
        draw->AddTriangleFilled(foldA, ImVec2(max.x, min.y), foldB,
                                IM_COL32(255, 255, 255, 60));
    }

    static void drawFolderIcon(ImDrawList* draw, ImVec2 center, float size, float scale = 1.0f) {
        float s = size * scale;
        ImU32 color = fileColor(FileIconType::Folder);
        ImVec2 min(center.x - s * 0.6f, center.y - s * 0.35f);
        ImVec2 max(center.x + s * 0.6f, center.y + s * 0.35f);
        draw->AddRectFilled(min, max, color, 3.0f);
        ImVec2 tabMin(min.x + s * 0.1f, min.y - s * 0.2f);
        ImVec2 tabMax(min.x + s * 0.5f, min.y);
        draw->AddRectFilled(tabMin, tabMax, color, 3.0f);
    }

    static void drawPanelIcon(ImDrawList* draw, ImVec2 center, float size,
                              PanelIconType type, float scale = 1.0f) {
        float s = size * scale;
        ImU32 color = panelColor(type);
        switch (type) {
        case PanelIconType::Explorer: {
            ImVec2 min(center.x - s * 0.5f, center.y - s * 0.5f);
            ImVec2 max(center.x + s * 0.5f, center.y + s * 0.5f);
            draw->AddRect(min, max, color, 2.0f, 0, 2.0f);
            draw->AddLine(ImVec2(min.x, center.y), ImVec2(max.x, center.y), color, 2.0f);
            break;
        }
        case PanelIconType::Outline: {
            draw->AddCircleFilled(center, s * 0.45f, color);
            break;
        }
        case PanelIconType::Terminal: {
            ImVec2 min(center.x - s * 0.5f, center.y - s * 0.35f);
            ImVec2 max(center.x + s * 0.5f, center.y + s * 0.35f);
            draw->AddRect(min, max, color, 2.0f, 0, 2.0f);
            draw->AddLine(ImVec2(min.x + s * 0.2f, center.y),
                          ImVec2(min.x + s * 0.35f, center.y + s * 0.15f), color, 2.0f);
            break;
        }
        default:
            draw->AddCircleFilled(center, s * 0.3f, color);
            break;
        }
    }

    static void drawDiagnosticIcon(ImDrawList* draw, ImVec2 center, float size,
                                   DiagnosticIconType type, float scale = 1.0f) {
        float s = size * scale;
        ImU32 color = diagnosticColor(type);
        if (type == DiagnosticIconType::Warning) {
            ImVec2 a(center.x, center.y - s * 0.5f);
            ImVec2 b(center.x - s * 0.5f, center.y + s * 0.4f);
            ImVec2 c(center.x + s * 0.5f, center.y + s * 0.4f);
            draw->AddTriangleFilled(a, b, c, color);
        } else if (type == DiagnosticIconType::Info) {
            draw->AddCircleFilled(center, s * 0.45f, color);
            draw->AddLine(ImVec2(center.x, center.y - s * 0.15f),
                          ImVec2(center.x, center.y + s * 0.2f), IM_COL32(30, 30, 30, 200), 2.0f);
        } else {
            draw->AddCircleFilled(center, s * 0.45f, color);
        }
    }

    static void drawAnnotationIcon(ImDrawList* draw, ImVec2 center, float size,
                                   AnnotationIconType type, float scale = 1.0f) {
        float s = size * scale;
        ImU32 color = annotationColor(type);
        switch (type) {
        case AnnotationIconType::Ownership:
            draw->AddRectFilled(ImVec2(center.x - s * 0.35f, center.y - s * 0.35f),
                                ImVec2(center.x + s * 0.35f, center.y + s * 0.35f),
                                color, 2.0f);
            break;
        case AnnotationIconType::Borrow:
            draw->AddCircleFilled(center, s * 0.35f, color);
            break;
        case AnnotationIconType::Shared:
            draw->AddTriangleFilled(ImVec2(center.x, center.y - s * 0.45f),
                                    ImVec2(center.x - s * 0.4f, center.y + s * 0.35f),
                                    ImVec2(center.x + s * 0.4f, center.y + s * 0.35f),
                                    color);
            break;
        case AnnotationIconType::Cache:
            draw->AddRect(ImVec2(center.x - s * 0.4f, center.y - s * 0.4f),
                          ImVec2(center.x + s * 0.4f, center.y + s * 0.4f),
                          color, 1.0f, 0, 2.0f);
            break;
        case AnnotationIconType::Compute:
        default:
            draw->AddCircleFilled(center, s * 0.15f, color);
            draw->AddCircleFilled(ImVec2(center.x - s * 0.35f, center.y), s * 0.15f, color);
            draw->AddCircleFilled(ImVec2(center.x + s * 0.35f, center.y), s * 0.15f, color);
            break;
        }
    }
};
