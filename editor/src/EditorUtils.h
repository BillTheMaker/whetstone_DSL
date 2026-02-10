#pragma once

#include "EditorState.h"
#include "StringUtils.h"

#include "imgui.h"
#include "SyntaxHighlighter.h"
#include "CodeEditorWidget.h"
#include "FileTree.h"
#include "WelcomeScreen.h"
#include "FileDialog.h"
#include "LSPClient.h"
#include "BufferManager.h"
#include "ast/Serialization.h"
#include "ast/Generator.h"
#include "ast/Annotation.h"

#include <string>
#include <vector>
#include <algorithm>
#include <climits>
#include <memory>

static int countLines(const std::string& text) {
    int lines = 1;
    for (char c : text) {
        if (c == '\n') ++lines;
    }
    return lines;
}

static std::string generateForLanguage(const Module* ast, const std::string& language) {
    if (!ast) return "";
    if (language == "python") {
        PythonGenerator gen;
        return gen.generate(ast);
    }
    if (language == "cpp") {
        CppGenerator gen;
        return gen.generate(ast);
    }
    if (language == "elisp") {
        ElispGenerator gen;
        return gen.generate(ast);
    }
    if (language == "javascript") {
        JavaScriptGenerator gen;
        return gen.generate(ast);
    }
    if (language == "typescript") {
        TypeScriptGenerator gen;
        return gen.generate(ast);
    }
    if (language == "java") {
        JavaGenerator gen;
        return gen.generate(ast);
    }
    if (language == "rust") {
        RustGenerator gen;
        return gen.generate(ast);
    }
    if (language == "go") {
        GoGenerator gen;
        return gen.generate(ast);
    }
    PythonGenerator gen;
    return gen.generate(ast);
}

static std::unique_ptr<Module> cloneModule(const Module* ast) {
    if (!ast) return nullptr;
    json j = toJson(ast);
    ASTNode* node = fromJson(j);
    if (!node || node->conceptType != "Module") return nullptr;
    return std::unique_ptr<Module>(static_cast<Module*>(node));
}

static bool isAnnotationNode(const ASTNode* node) {
    if (!node) return false;
    if (node->conceptType.find("Annotation") != std::string::npos) return true;
    if (node->conceptType == "DerefStrategy") return true;
    if (node->conceptType == "OptimizationLock") return true;
    if (node->conceptType == "LangSpecific") return true;
    return false;
}

static int countAnnotationNodes(const ASTNode* node) {
    if (!node) return 0;
    int count = isAnnotationNode(node) ? 1 : 0;
    for (auto* child : node->allChildren()) {
        count += countAnnotationNodes(child);
    }
    return count;
}

static std::string findOptimizationBlockReason(const ASTNode* node) {
    if (!node) return "";
    for (auto* anno : node->getChildren("annotations")) {
        if (anno->conceptType == "OwnerAnnotation") {
            auto* oa = static_cast<const OwnerAnnotation*>(anno);
            if (oa->strategy == "Single") {
                return "@Owner(Single) blocks optimization (aliasing risk)";
            }
        }
        if (anno->conceptType == "DeallocateAnnotation") {
            auto* da = static_cast<const DeallocateAnnotation*>(anno);
            if (da->strategy == "Explicit") {
                return "@Deallocate(Explicit) blocks optimization (reorder risk)";
            }
        }
        if (anno->conceptType == "AllocateAnnotation") {
            auto* aa = static_cast<const AllocateAnnotation*>(anno);
            if (aa->strategy == "Static") {
                return "@Allocate(Static) blocks optimization (dynamic alloc risk)";
            }
        }
    }
    for (auto* child : node->allChildren()) {
        std::string found = findOptimizationBlockReason(child);
        if (!found.empty()) return found;
    }
    return "";
}

static std::string findOptimizationLockWarning(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "OptimizationLock") {
        auto* lock = static_cast<const OptimizationLock*>(node);
        if (lock->lockLevel == "warning") {
            return "OptimizationLock: " + lock->lockReason +
                   " (locked by " + lock->lockedBy + ")";
        }
    }
    for (auto* child : node->allChildren()) {
        std::string found = findOptimizationLockWarning(child);
        if (!found.empty()) return found;
    }
    return "";
}

static ImVec4 transformColorForName(const std::string& name) {
    if (name == "constant-fold") return ImVec4(0.45f, 0.85f, 0.45f, 1.0f);
    if (name == "dead-code-elim") return ImVec4(0.90f, 0.45f, 0.45f, 1.0f);
    return ImVec4(0.75f, 0.75f, 0.75f, 1.0f);
}


// ---------------------------------------------------------------------------
//  ImGui InputTextMultiline with std::string resize callback
// ---------------------------------------------------------------------------
struct InputTextCallbackData {
    std::string* str;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* userData = static_cast<InputTextCallbackData*>(data->UserData);
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        userData->str->resize(data->BufTextLen);
        data->Buf = userData->str->data();
    }
    return 0;
}

static bool InputTextMultilineStr(const char* label, std::string* str,
                                   const ImVec2& size, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallbackData cbData{str};
    // Ensure buffer has room
    if (str->capacity() < str->size() + 256)
        str->reserve(str->size() + 4096);
    return ImGui::InputTextMultiline(label, str->data(), str->capacity() + 1,
                                      size, flags, InputTextCallback, &cbData);
}

static bool InputTextStr(const char* label, std::string* str, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    InputTextCallbackData cbData{str};
    if (str->capacity() < str->size() + 64)
        str->reserve(str->size() + 256);
    return ImGui::InputText(label, str->data(), str->capacity() + 1,
                            flags, InputTextCallback, &cbData);
}

static bool annotationInfo(const ASTNode* anno, ImU32& color, std::string& label) {
    if (!anno) return false;
    if (anno->conceptType == "ReclaimAnnotation") {
        auto* a = static_cast<const ReclaimAnnotation*>(anno);
        if (a->strategy == "Tracing") {
            color = IM_COL32(80, 140, 220, 255);
            label = "@Reclaim(Tracing)";
            return true;
        }
    } else if (anno->conceptType == "OwnerAnnotation") {
        auto* a = static_cast<const OwnerAnnotation*>(anno);
        if (a->strategy == "Single") {
            color = IM_COL32(220, 160, 60, 255);
            label = "@Owner(Single)";
            return true;
        }
    } else if (anno->conceptType == "DeallocateAnnotation") {
        auto* a = static_cast<const DeallocateAnnotation*>(anno);
        if (a->strategy == "Explicit") {
            color = IM_COL32(220, 80, 80, 255);
            label = "@Deallocate(Explicit)";
            return true;
        }
    } else if (anno->conceptType == "LifetimeAnnotation") {
        auto* a = static_cast<const LifetimeAnnotation*>(anno);
        if (a->strategy == "RAII") {
            color = IM_COL32(80, 180, 100, 255);
            label = "@Lifetime(RAII)";
            return true;
        }
    } else if (anno->conceptType == "AllocateAnnotation") {
        auto* a = static_cast<const AllocateAnnotation*>(anno);
        if (a->strategy == "Static") {
            color = IM_COL32(160, 160, 160, 255);
            label = "@Allocate(Static)";
            return true;
        }
    }
    return false;
}

static std::string annotationLabel(const ASTNode* anno) {
    if (!anno) return "";
    if (anno->conceptType == "ReclaimAnnotation") {
        auto* a = static_cast<const ReclaimAnnotation*>(anno);
        return "@Reclaim(" + a->strategy + ")";
    }
    if (anno->conceptType == "OwnerAnnotation") {
        auto* a = static_cast<const OwnerAnnotation*>(anno);
        return "@Owner(" + a->strategy + ")";
    }
    if (anno->conceptType == "DeallocateAnnotation") {
        auto* a = static_cast<const DeallocateAnnotation*>(anno);
        return "@Deallocate(" + a->strategy + ")";
    }
    if (anno->conceptType == "LifetimeAnnotation") {
        auto* a = static_cast<const LifetimeAnnotation*>(anno);
        return "@Lifetime(" + a->strategy + ")";
    }
    if (anno->conceptType == "AllocateAnnotation") {
        auto* a = static_cast<const AllocateAnnotation*>(anno);
        return "@Allocate(" + a->strategy + ")";
    }
    return anno->conceptType;
}

static std::string nodeDisplayName(const ASTNode* node) {
    if (!node) return "";
    if (node->conceptType == "Function") return static_cast<const Function*>(node)->name;
    if (node->conceptType == "Variable") return static_cast<const Variable*>(node)->name;
    return node->conceptType;
}

static std::string nextAnnotationId() {
    static int counter = 0;
    return "anno_ui_" + std::to_string(counter++);
}

static Annotation* createAnnotationNode(const std::string& type, const std::string& strategy) {
    if (type == "ReclaimAnnotation") {
        auto* a = new ReclaimAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "OwnerAnnotation") {
        auto* a = new OwnerAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "DeallocateAnnotation") {
        auto* a = new DeallocateAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "LifetimeAnnotation") {
        auto* a = new LifetimeAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    if (type == "AllocateAnnotation") {
        auto* a = new AllocateAnnotation(nextAnnotationId(), strategy);
        return a;
    }
    return nullptr;
}

static int spanScore(const ASTNode* node) {
    if (!node || !node->hasSpan()) return INT_MAX;
    int lines = node->spanEndLine - node->spanStartLine;
    int cols = node->spanEndCol - node->spanStartCol;
    return lines * 10000 + cols;
}

static bool spanContains(const ASTNode* node, int line, int col) {
    if (!node || !node->hasSpan()) return false;
    if (line < node->spanStartLine || line > node->spanEndLine) return false;
    if (line == node->spanStartLine && col < node->spanStartCol) return false;
    if (line == node->spanEndLine && col > node->spanEndCol) return false;
    return true;
}

static ASTNode* findNodeAtPosition(ASTNode* node, int line, int col) {
    if (!node) return nullptr;
    ASTNode* best = nullptr;
    if (spanContains(node, line, col)) best = node;
    for (auto* child : node->allChildren()) {
        ASTNode* cand = findNodeAtPosition(child, line, col);
        if (cand) {
            if (!best || spanScore(cand) < spanScore(best)) best = cand;
        }
    }
    return best;
}

static ASTNode* findAnnotationTarget(ASTNode* node, int line) {
    if (!node) return nullptr;
    ASTNode* best = nullptr;
    if (node->hasSpan() && line >= node->spanStartLine && line <= node->spanEndLine) {
        if (node->conceptType == "Function" || node->conceptType == "Variable") {
            best = node;
        }
    }
    for (auto* child : node->allChildren()) {
        ASTNode* cand = findAnnotationTarget(child, line);
        if (cand) {
            if (!best || spanScore(cand) < spanScore(best)) best = cand;
        }
    }
    return best;
}

static ASTNode* findNodeById(ASTNode* node, const std::string& id) {
    if (!node) return nullptr;
    if (node->id == id) return node;
    for (auto* child : node->allChildren()) {
        if (auto* found = findNodeById(child, id)) return found;
    }
    return nullptr;
}

static void collectAnnotationMarkers(const ASTNode* node, std::vector<AnnotationMarker>& out) {
    if (!node) return;
    if (node->hasSpan()) {
        for (const auto* anno : node->getChildren("annotations")) {
            ImU32 color = 0;
            std::string label;
            if (annotationInfo(anno, color, label)) {
                AnnotationMarker marker;
                marker.line = node->spanStartLine;
                marker.color = color;
                marker.message = label;
                out.push_back(std::move(marker));
            }
        }
    }
    for (auto* child : node->allChildren()) {
        collectAnnotationMarkers(child, out);
    }
}

struct OutlineItem {
    std::string name;
    std::string kind;
    int line = -1;
    int col = -1;
    std::vector<OutlineItem> children;
};

static OutlineItem makeOutlineItem(const std::string& name,
                                   const std::string& kind,
                                   const ASTNode* node) {
    OutlineItem item;
    item.name = name;
    item.kind = kind;
    if (node && node->hasSpan()) {
        item.line = node->spanStartLine;
        item.col = node->spanStartCol;
    }
    return item;
}

static void collectOutlineFromFunction(const Function* fn, std::vector<OutlineItem>& out) {
    OutlineItem item = makeOutlineItem(fn->name, "Function", fn);
    for (auto* p : fn->getChildren("parameters")) {
        if (p->conceptType == "Parameter") {
            auto* param = static_cast<const Parameter*>(p);
            item.children.push_back(makeOutlineItem(param->name, "Parameter", param));
        }
    }
    for (auto* child : fn->getChildren("body")) {
        if (child->conceptType == "Variable") {
            auto* var = static_cast<const Variable*>(child);
            item.children.push_back(makeOutlineItem(var->name, "Variable", var));
        }
    }
    out.push_back(std::move(item));
}

static void collectOutlineFromAST(const Module* mod, std::vector<OutlineItem>& out) {
    if (!mod) return;
    for (auto* child : mod->getChildren("variables")) {
        if (child->conceptType == "Variable") {
            auto* var = static_cast<const Variable*>(child);
            out.push_back(makeOutlineItem(var->name, "Variable", var));
        }
    }
    for (auto* child : mod->getChildren("functions")) {
        if (child->conceptType == "Function") {
            auto* fn = static_cast<const Function*>(child);
            collectOutlineFromFunction(fn, out);
        }
    }
}

// trimCopy is in StringUtils.h

static std::string toLowerCopy(const std::string& input) {
    std::string out = input;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

static bool containsCaseInsensitive(const std::string& text, const std::string& filter) {
    if (filter.empty()) return true;
    std::string hay = toLowerCopy(text);
    return hay.find(filter) != std::string::npos;
}

static bool outlineMatchesFilter(const OutlineItem& item, const std::string& filter) {
    if (filter.empty()) return true;
    if (containsCaseInsensitive(item.name, filter)) return true;
    for (const auto& child : item.children) {
        if (outlineMatchesFilter(child, filter)) return true;
    }
    return false;
}

static const char* lspSymbolKindLabel(int kind) {
    switch (kind) {
        case 1: return "File";
        case 2: return "Module";
        case 5: return "Class";
        case 6: return "Method";
        case 12: return "Function";
        case 13: return "Variable";
        case 14: return "Constant";
        case 19: return "String";
        case 23: return "Struct";
        default: return "Symbol";
    }
}

static bool lspSymbolMatchesFilter(const LSPClient::DocumentSymbol& sym, const std::string& filter) {
    if (filter.empty()) return true;
    if (containsCaseInsensitive(sym.name, filter)) return true;
    for (const auto& child : sym.children) {
        if (lspSymbolMatchesFilter(child, filter)) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
//  Theme setup — VSCode Dark-inspired
// ---------------------------------------------------------------------------
static void SetupVSCodeDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window
    colors[ImGuiCol_WindowBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Borders
    colors[ImGuiCol_Border]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_BorderShadow]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame (input fields, checkboxes)
    colors[ImGuiCol_FrameBg]            = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);

    // Title bar
    colors[ImGuiCol_TitleBg]            = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);

    // Menu bar
    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button]             = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    // Headers (collapsing headers, tree nodes)
    colors[ImGuiCol_Header]             = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    // Separator
    colors[ImGuiCol_Separator]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]   = ImVec4(0.28f, 0.56f, 0.89f, 1.00f);
    colors[ImGuiCol_SeparatorActive]    = ImVec4(0.28f, 0.56f, 0.89f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabHovered]        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabActive]         = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused]      = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);

    // Docking
    colors[ImGuiCol_DockingPreview]    = ImVec4(0.28f, 0.56f, 0.89f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg]    = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);

    // Text
    colors[ImGuiCol_Text]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TextDisabled]      = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Selection / highlight
    colors[ImGuiCol_TextSelectedBg]    = ImVec4(0.17f, 0.40f, 0.64f, 0.60f);

    // Style tweaks
    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 0.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowPadding     = ImVec2(8, 8);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(6, 4);
}

static void SetupVSCodeLightTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]           = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_ChildBg]            = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_PopupBg]            = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);

    colors[ImGuiCol_Border]             = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_FrameBg]            = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]     = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_FrameBgActive]      = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);

    colors[ImGuiCol_TitleBg]            = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TitleBgActive]      = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);

    colors[ImGuiCol_MenuBarBg]          = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
    colors[ImGuiCol_Button]             = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_ButtonHovered]      = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_ButtonActive]       = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    colors[ImGuiCol_Header]             = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_HeaderHovered]      = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    colors[ImGuiCol_HeaderActive]       = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    colors[ImGuiCol_Text]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_TextDisabled]       = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.40f, 0.60f, 0.90f, 0.45f);

    style.WindowRounding    = 0.0f;
    style.FrameRounding     = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 2.0f;
    style.TabRounding       = 0.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.WindowPadding     = ImVec2(8, 8);
    style.FramePadding      = ImVec2(6, 3);
    style.ItemSpacing       = ImVec2(6, 4);
}

// ---------------------------------------------------------------------------
//  Syntax highlight color map
// ---------------------------------------------------------------------------
static ImVec4 tokenColor(TokenCategory cat) {
    switch (cat) {
        case TokenCategory::Keyword:     return ImVec4(0.77f, 0.49f, 0.86f, 1.0f); // purple
        case TokenCategory::String:      return ImVec4(0.81f, 0.54f, 0.37f, 1.0f); // orange
        case TokenCategory::Comment:     return ImVec4(0.42f, 0.52f, 0.35f, 1.0f); // green
        case TokenCategory::Number:      return ImVec4(0.71f, 0.80f, 0.55f, 1.0f); // light green
        case TokenCategory::Function:    return ImVec4(0.86f, 0.86f, 0.55f, 1.0f); // yellow
        case TokenCategory::Parameter:   return ImVec4(0.60f, 0.78f, 0.90f, 1.0f); // light blue
        case TokenCategory::Type:        return ImVec4(0.30f, 0.70f, 0.68f, 1.0f); // teal
        case TokenCategory::Operator:    return ImVec4(0.86f, 0.86f, 0.86f, 1.0f); // white
        case TokenCategory::Punctuation: return ImVec4(0.60f, 0.60f, 0.60f, 1.0f); // gray
        case TokenCategory::Builtin:     return ImVec4(0.30f, 0.70f, 0.90f, 1.0f); // blue
        case TokenCategory::Identifier:  return ImVec4(0.60f, 0.78f, 0.90f, 1.0f); // light blue
        default:                         return ImVec4(0.86f, 0.86f, 0.86f, 1.0f); // white
    }
}

// ---------------------------------------------------------------------------
//  Render syntax-highlighted text (read-only preview)
// ---------------------------------------------------------------------------
static void RenderHighlightedText(const std::string& text,
                                   const std::vector<HighlightSpan>& spans) {
    if (text.empty()) return;

    // Build a color for each byte position
    // Default = plain text color
    std::vector<TokenCategory> charCats(text.size(), TokenCategory::Plain);
    for (const auto& span : spans) {
        for (uint32_t i = span.start; i < span.end && i < charCats.size(); ++i) {
            charCats[i] = span.category;
        }
    }

    // Render line by line, span by span
    size_t pos = 0;
    int lineNum = 1;
    while (pos < text.size()) {
        // Find end of line
        size_t eol = text.find('\n', pos);
        if (eol == std::string::npos) eol = text.size();

        // Line number
        ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.45f, 1.0f), "%4d ", lineNum);
        ImGui::SameLine(0.0f, 0.0f);

        // Render spans within this line
        size_t linePos = pos;
        while (linePos < eol) {
            // Find the extent of the current color
            TokenCategory cat = charCats[linePos];
            size_t spanEnd = linePos + 1;
            while (spanEnd < eol && charCats[spanEnd] == cat) ++spanEnd;

            std::string chunk = text.substr(linePos, spanEnd - linePos);
            ImGui::TextColored(tokenColor(cat), "%s", chunk.c_str());
            if (spanEnd < eol) ImGui::SameLine(0.0f, 0.0f);

            linePos = spanEnd;
        }

        if (linePos == pos) {
            // Empty line
            ImGui::TextUnformatted("");
        }

        pos = eol + 1;
        ++lineNum;
    }
}

// ---------------------------------------------------------------------------
//  File tree rendering
// ---------------------------------------------------------------------------
static void RenderFileTree(const FileNode& node, EditorState& state) {
    if (!node.isDir) {
        if (ImGui::Selectable(node.name.c_str())) {
            state.doOpen(node.path);
        }
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (ImGui::TreeNodeEx(node.name.c_str(), flags)) {
        for (const auto& child : node.children) {
            RenderFileTree(child, state);
        }
        ImGui::TreePop();
    }
}

// ---------------------------------------------------------------------------
//  Welcome screen rendering
// ---------------------------------------------------------------------------
static void RenderWelcome(WelcomeScreen& welcome, EditorState& state, std::string& lastDialogPath) {
    ImGui::Text("%s", welcome.getTitle().c_str());
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", welcome.getSubtitle().c_str());
    ImGui::Separator();

    ImGui::Text("Quick Actions");
    if (ImGui::Button("New File")) {
        std::string lang = state.active() ? state.active()->language : "python";
        state.createBuffer(state.makeUntitledName(), "", lang);
    }
    ImGui::SameLine();
    if (ImGui::Button("Open File")) {
        auto path = FileDialog::openFile({"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go"}, lastDialogPath});
        if (!path.empty()) {
            lastDialogPath = path;
            state.doOpen(path);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Folder")) {
        auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
        if (!path.empty()) {
            state.workspaceRoot = path;
            state.fileTreeDirty = true;
            state.search.projectSearch.setRoot(state.workspaceRoot);
            lastDialogPath = path;
        }
    }

    ImGui::Separator();
    ImGui::Text("Recent Files");
    for (const auto& rf : welcome.getRecentFiles()) {
        if (ImGui::Selectable(rf.displayName.c_str())) {
            state.doOpen(rf.path, bufferModeFromString(rf.mode));
        }
    }

    ImGui::Separator();
    ImGui::Text("Tip");
    ImGui::TextWrapped("%s", welcome.getTip(0).c_str());
}
