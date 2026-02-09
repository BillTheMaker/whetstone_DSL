// Whetstone Editor — ImGui-based structured code editor
//
// VSCode/JetBrains-inspired layout with docking:
//   - File tree (left)
//   - Editable text area (center) backed by TextEditor + TextASTSync
//   - Syntax-highlighted preview / AST view (bottom)
//   - Toolbar + status bar
//   - Configurable keybinding profiles (VSCode default)

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "TextEditor.h"
#include "TextASTSync.h"
#include "SyntaxHighlighter.h"
#include "KeybindingManager.h"
#include "CodeEditorWidget.h"
#include "EditorMode.h"
#include "FileDialog.h"
#include "FileTree.h"
#include "WelcomeScreen.h"
#include "DragDropHandler.h"
#include "FileWatcher.h"
#include "LSPClient.h"
#include "Pipeline.h"
#include "Diagnostics.h"
#include "SettingsManager.h"
#include "LayoutManager.h"
#include "ASTMutationAPI.h"
#include "MemoryStrategyInference.h"
#include "AnnotationConflict.h"
#include "StrategyDashboard.h"
#include "ast/Generator.h"
#include "ast/Annotation.h"

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cctype>
#include <climits>
#include <filesystem>
#include <map>
#include <memory>
#include <cstdlib>
#include <nlohmann/json.hpp>

// ---------------------------------------------------------------------------
//  Editor application state
// ---------------------------------------------------------------------------
struct BufferState {
    TextEditor        editor;
    TextASTSync       sync;
    CodeEditorWidget  widget;
    CodeEditorWidget  generatedWidget;
    EditorMode        mode;
    EditorMode        generatedMode;
    std::string       language    = "python";
    std::string       generatedLanguage = "python";
    std::string       path        = "(untitled)";
    bool              modified    = false;
    int               cursorLine  = 1;
    int               cursorCol   = 1;
    int               lspVersion  = 1;
    std::string       editBuf;
    std::string       generatedBuf;
    std::vector<HighlightSpan> highlights;
    std::vector<HighlightSpan> generatedHighlights;
    bool              highlightsDirty = true;
    bool              generatedHighlightsDirty = true;
    int               generatedHighlightLine = -1;
    float             splitScrollX = 0.0f;
    float             splitScrollY = 0.0f;
};

struct EditorState {
    KeybindingManager keys;
    BufferManager     buffers;
    std::map<std::string, std::unique_ptr<BufferState>> bufferStates;
    BufferState*      activeBuffer = nullptr;

    // Find/Replace state
    bool              showFind    = false;
    char              findBuf[256]    = {};
    char              replaceBuf[256] = {};
    int               lastFindPos     = 0;

    // Bottom panel
    int               bottomTab   = 0;  // 0=Output, 1=AST, 2=Highlighted
    std::string       outputLog;

    // Custom editor widget state
    bool              showWhitespace = false;
    bool              showMinimap = false;
    bool              showAnnotations = false;
    LayoutPreset      layoutPreset = LayoutPreset::VSCode;
    WelcomeScreen     welcome;
    std::string       workspaceRoot;
    FileTree          fileTree;
    FileNode          fileTreeRoot;
    bool              fileTreeDirty = true;
    FileWatcher       watcher;
    std::shared_ptr<LSPTransport> lspTransport;
    std::shared_ptr<LSPClient> lsp;
    bool              completionPending = false;
    bool              completionVisible = false;
    int               completionSelected = 0;
    double            completionLastChange = 0.0;
    bool              hoverPending = false;
    double            hoverLastMove = 0.0;
    ImVec2            hoverLastMouse = ImVec2(-1.0f, -1.0f);
    Pipeline          pipeline;
    bool              analysisPending = false;
    double            analysisLastChange = 0.0;
    std::vector<EditorDiagnostic> whetstoneDiagnostics;
    SettingsManager   settings;
    bool              showLspSettings = false;
    ASTMutationAPI    mutator;
    std::vector<MemoryStrategyInference::Suggestion> suggestions;
    bool              showSuggestionPopup = false;
    MemoryStrategyInference::Suggestion suggestionPopup;

    BufferState* active() { return activeBuffer; }

    static std::string toFileUri(const std::string& path) {
        std::filesystem::path p(path);
        return "file:///" + p.generic_string();
    }

    static std::string fromFileUri(const std::string& uri) {
        const std::string prefix = "file:///";
        if (uri.rfind(prefix, 0) != 0) return uri;
        std::string path = uri.substr(prefix.size());
        std::replace(path.begin(), path.end(), '/', '\\');
        return path;
    }

    static int byteOffsetForLineCol(const std::string& text, int lineZero, int colZero) {
        if (lineZero < 0) lineZero = 0;
        if (colZero < 0) colZero = 0;
        int line = 0;
        int pos = 0;
        while (pos < (int)text.size() && line < lineZero) {
            if (text[pos] == '\n') ++line;
            ++pos;
        }
        int col = 0;
        while (pos < (int)text.size() && text[pos] != '\n' && col < colZero) {
            ++pos;
            ++col;
        }
        return pos;
    }

    static int wordStartAt(const std::string& text, int pos) {
        pos = std::max(0, std::min(pos, (int)text.size()));
        while (pos > 0) {
            char c = text[pos - 1];
            if (std::isalnum((unsigned char)c) || c == '_') {
                --pos;
            } else {
                break;
            }
        }
        return pos;
    }

    static std::string wordPrefixAt(const std::string& text, int pos) {
        int start = wordStartAt(text, pos);
        return text.substr(start, pos - start);
    }

    void jumpTo(BufferState* buf, int lineZero, int colZero) {
        if (!buf) return;
        int pos = byteOffsetForLineCol(buf->editBuf, lineZero, colZero);
        buf->widget.setCursor(pos);
        buf->cursorLine = lineZero + 1;
        buf->cursorCol = colZero + 1;
    }

    void applyCompletion(BufferState* buf, const std::string& insertText, int replaceStart, int replaceEnd) {
        if (!buf) return;
        replaceStart = std::max(0, std::min(replaceStart, (int)buf->editBuf.size()));
        replaceEnd = std::max(replaceStart, std::min(replaceEnd, (int)buf->editBuf.size()));
        buf->editBuf.replace(replaceStart, replaceEnd - replaceStart, insertText);
        int newPos = replaceStart + (int)insertText.size();
        buf->widget.setCursor(newPos);
        activeBuffer = buf;
        onTextChanged();
    }

    std::string makeUntitledName() const {
        if (!buffers.hasBuffer("(untitled)")) return "(untitled)";
        int i = 1;
        while (true) {
            std::string name = "(untitled-" + std::to_string(i) + ")";
            if (!buffers.hasBuffer(name)) return name;
            ++i;
        }
    }

    void createBuffer(const std::string& path, const std::string& content, const std::string& language) {
        if (buffers.hasBuffer(path)) {
            buffers.switchToBuffer(path);
            activeBuffer = bufferStates[path].get();
            return;
        }
        auto state = std::make_unique<BufferState>();
        state->path = path;
        state->language = language;
        state->generatedLanguage = language;
        state->mode.setLanguage(language);
        state->generatedMode.setLanguage(language);
        state->editor.setContent(content, language);
        state->sync.setText(content, language);
        state->sync.syncNow();
        state->editBuf = content;
        state->highlightsDirty = true;
        state->generatedHighlightsDirty = true;
        state->modified = false;
        state->lspVersion = 1;
        buffers.openBuffer(path, content, language);
        activeBuffer = state.get();
        bufferStates[path] = std::move(state);
        if (path.rfind("(untitled", 0) != 0) watcher.watch(path);
        if (lsp && path.rfind("(untitled", 0) != 0) {
            lsp->didOpen(toFileUri(path), language, content, activeBuffer->lspVersion);
        }
    }

    void switchToBuffer(const std::string& path) {
        if (!buffers.hasBuffer(path)) return;
        buffers.switchToBuffer(path);
        activeBuffer = bufferStates[path].get();
    }

    std::filesystem::path configDir() const {
        const char* home = std::getenv("USERPROFILE");
        if (!home) home = std::getenv("HOME");
        std::filesystem::path base = home ? std::filesystem::path(home) : std::filesystem::current_path();
        return base / ".whetstone";
    }

    std::filesystem::path recentFilePath() const {
        return configDir() / "recent.json";
    }

    void loadRecentFiles() {
        std::filesystem::path p = recentFilePath();
        if (!std::filesystem::exists(p)) return;
        std::ifstream in(p.string());
        if (!in.is_open()) return;
        nlohmann::json j;
        in >> j;
        if (!j.is_array()) return;
        for (const auto& item : j) {
            if (!item.contains("path")) continue;
            std::string path = item.value("path", "");
            std::string lang = item.value("language", "");
            if (!path.empty()) welcome.addRecentFile(path, lang);
        }
    }

    void saveRecentFiles() {
        std::filesystem::path p = recentFilePath();
        std::filesystem::create_directories(p.parent_path());
        nlohmann::json j = nlohmann::json::array();
        for (const auto& rf : welcome.getRecentFiles()) {
            j.push_back({{"path", rf.path}, {"language", rf.language}});
        }
        std::ofstream out(p.string(), std::ios::binary);
        out << j.dump(2);
    }

    void init() {
        outputLog = "Whetstone Editor ready.\n";
        workspaceRoot = std::filesystem::current_path().string();
        fileTreeDirty = true;
        loadRecentFiles();
    }

    void setLanguage(const std::string& lang) {
        if (!active()) return;
        std::string prevLang = active()->language;
        active()->language = lang;
        active()->mode.setLanguage(lang);
        if (active()->generatedLanguage == prevLang) {
            active()->generatedLanguage = lang;
            active()->generatedMode.setLanguage(lang);
            active()->generatedHighlightsDirty = true;
        }
        active()->editor.setContent(active()->editBuf, lang);
        active()->sync.setText(active()->editBuf, lang);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
    }

    // Called after editBuf changes (from ImGui input)
    void onTextChanged() {
        if (!active()) return;
        active()->editor.setContent(active()->editBuf, active()->language);
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
        active()->generatedHighlightsDirty = true;
        active()->modified = true;
        if (lsp && active()->path.rfind("(untitled", 0) != 0) {
            active()->lspVersion += 1;
            lsp->didChange(toFileUri(active()->path), active()->editBuf, active()->lspVersion);
        }
    }

    void doUndo() {
        if (!active()) return;
        active()->editor.undo();
        active()->editBuf = active()->editor.getContent();
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
        active()->generatedHighlightsDirty = true;
    }

    void doRedo() {
        if (!active()) return;
        active()->editor.redo();
        active()->editBuf = active()->editor.getContent();
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
        active()->generatedHighlightsDirty = true;
    }

    void doFind() {
        if (!active()) return;
        if (strlen(findBuf) == 0) return;
        int pos = active()->editor.find(findBuf, lastFindPos);
        if (pos >= 0) {
            lastFindPos = pos + 1;
            int line = 1, col = 1;
            for (int i = 0; i < pos && i < (int)active()->editBuf.size(); ++i) {
                if (active()->editBuf[i] == '\n') { ++line; col = 1; } else { ++col; }
            }
            active()->cursorLine = line;
            active()->cursorCol = col;
            outputLog += "Found \"" + std::string(findBuf) + "\" at line " +
                         std::to_string(line) + ", col " + std::to_string(col) + "\n";
        } else {
            lastFindPos = 0;
            outputLog += "\"" + std::string(findBuf) + "\" not found.\n";
        }
    }

    void doReplaceAll() {
        if (!active()) return;
        if (strlen(findBuf) == 0) return;
        int count = active()->editor.replaceAll(findBuf, replaceBuf);
        if (count > 0) {
            active()->editBuf = active()->editor.getContent();
            active()->sync.setText(active()->editBuf, active()->language);
            active()->sync.syncNow();
            active()->highlightsDirty = true;
            active()->generatedHighlightsDirty = true;
            active()->modified = true;
        }
        outputLog += "Replaced " + std::to_string(count) + " occurrence(s).\n";
    }

    void doSave() {
        if (!active()) return;
        if (active()->path.rfind("(untitled", 0) == 0) return;
        std::ofstream out(active()->path);
        if (out.is_open()) {
            out << active()->editBuf;
            out.close();
            active()->modified = false;
            outputLog += "Saved: " + active()->path + "\n";
            if (lsp) lsp->didSave(toFileUri(active()->path), active()->editBuf);
        } else {
            outputLog += "Error saving: " + active()->path + "\n";
        }
    }

    void doOpen(const std::string& path) {
        std::ifstream in(path);
        if (in.is_open()) {
            std::ostringstream ss;
            ss << in.rdbuf();
            std::string content = ss.str();
            std::string language = "python";
            if (path.size() > 3 && path.substr(path.size() - 3) == ".py")
                language = "python";
            else if (path.size() > 4 && path.substr(path.size() - 4) == ".cpp")
                language = "cpp";
            else if (path.size() > 2 && path.substr(path.size() - 2) == ".h")
                language = "cpp";
            else if (path.size() > 3 && path.substr(path.size() - 3) == ".el")
                language = "elisp";
            else if (path.size() > 3 && path.substr(path.size() - 3) == ".js")
                language = "javascript";
            else if (path.size() > 3 && path.substr(path.size() - 3) == ".ts")
                language = "typescript";
            else if (path.size() > 5 && path.substr(path.size() - 5) == ".java")
                language = "java";
            else if (path.size() > 3 && path.substr(path.size() - 3) == ".rs")
                language = "rust";
            else if (path.size() > 3 && path.substr(path.size() - 3) == ".go")
                language = "go";
            createBuffer(path, content, language);
            welcome.addRecentFile(path, language);
            saveRecentFiles();
            outputLog += "Opened: " + path + "\n";
        } else {
            outputLog += "Error opening: " + path + "\n";
        }
    }

    void reloadBuffer(const std::string& path) {
        auto it = bufferStates.find(path);
        if (it == bufferStates.end()) return;
        std::ifstream in(path);
        if (!in.is_open()) return;
        std::ostringstream ss;
        ss << in.rdbuf();
        auto* buf = it->second.get();
        buf->editBuf = ss.str();
        buf->editor.setContent(buf->editBuf, buf->language);
        buf->sync.setText(buf->editBuf, buf->language);
        buf->sync.syncNow();
        buf->highlightsDirty = true;
        buf->generatedHighlightsDirty = true;
        buf->modified = false;
        outputLog += "Reloaded: " + path + "\n";
    }

    void handleFileChanges() {
        auto changed = watcher.poll();
        for (const auto& path : changed) {
            auto it = bufferStates.find(path);
            if (it == bufferStates.end()) continue;
            if (it->second->modified) {
                outputLog += "File changed on disk (dirty): " + path + "\n";
            } else {
                reloadBuffer(path);
            }
        }
    }

    void updateHighlights() {
        if (!active()) return;
        if (!active()->highlightsDirty) return;
        active()->highlights = SyntaxHighlighter::highlight(active()->editBuf, active()->language);
        active()->highlightsDirty = false;
    }

    void updateGenerated() {
        if (!active()) return;
        Module* ast = active()->sync.getAST();
        std::string generated;
        if (ast) {
            if (active()->generatedLanguage == "python") {
                PythonGenerator gen;
                generated = gen.generate(ast);
            } else if (active()->generatedLanguage == "cpp") {
                CppGenerator gen;
                generated = gen.generate(ast);
            } else if (active()->generatedLanguage == "elisp") {
                ElispGenerator gen;
                generated = gen.generate(ast);
            } else {
                PythonGenerator gen;
                generated = gen.generate(ast);
            }
        }
        if (generated != active()->generatedBuf) {
            active()->generatedBuf = generated;
            active()->generatedHighlightsDirty = true;
        }
        if (active()->generatedHighlightsDirty) {
            active()->generatedHighlights =
                SyntaxHighlighter::highlight(active()->generatedBuf, active()->generatedLanguage);
            active()->generatedHighlightsDirty = false;
        }
    }

    void updateCursorPos(int bytePos) {
        if (!active()) return;
        active()->cursorLine = 1;
        active()->cursorCol = 1;
        for (int i = 0; i < bytePos && i < (int)active()->editBuf.size(); ++i) {
            if (active()->editBuf[i] == '\n') { ++active()->cursorLine; active()->cursorCol = 1; }
            else { ++active()->cursorCol; }
        }
    }

    void refreshFileTree() {
        if (!fileTreeDirty) return;
        fileTreeRoot = fileTree.build(workspaceRoot);
        fileTreeDirty = false;
    }
};

struct NullLSPTransport : public LSPTransport {
    void send(const std::string& msg) override { (void)msg; }
    bool receive(std::string& out) override { (void)out; return false; }
    bool isOpen() const override { return false; }
    void close() override {}
};

static int countLines(const std::string& text) {
    int lines = 1;
    for (char c : text) {
        if (c == '\n') ++lines;
    }
    return lines;
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
            lastDialogPath = path;
        }
    }

    ImGui::Separator();
    ImGui::Text("Recent Files");
    for (const auto& rf : welcome.getRecentFiles()) {
        if (ImGui::Selectable(rf.displayName.c_str())) {
            state.doOpen(rf.path);
        }
    }

    ImGui::Separator();
    ImGui::Text("Tip");
    ImGui::TextWrapped("%s", welcome.getTip(0).c_str());
}

// ---------------------------------------------------------------------------
//  Main
// ---------------------------------------------------------------------------
int main(int, char**) {
    // SDL init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL Error: %s\n", SDL_GetError());
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    auto winFlags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                       SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
    SDL_Window* window = SDL_CreateWindow("Whetstone Editor",
                                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           1440, 900, winFlags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Load a monospace font (Consolas on Windows, fallback to default)
    ImFont* monoFont = nullptr;
    monoFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 15.0f);
    if (!monoFont) {
        monoFont = io.Fonts->AddFontDefault();
    }
    // Also keep default font for UI elements
    ImFont* uiFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 15.0f);
    if (!uiFont) {
        uiFont = io.Fonts->AddFontDefault();
    }

    SetupVSCodeDarkTheme();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Editor state
    EditorState state;
    state.init();
    state.lspTransport = std::make_shared<NullLSPTransport>();
    state.lsp = std::make_shared<LSPClient>(state.lspTransport);

    // File dialog defaults
    std::string lastDialogPath;

    bool done = false;
        while (!done) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_DROPFILE) {
                char* droppedFile = event.drop.file;
                if (droppedFile) {
                    DragDropHandler::handleDrop(droppedFile,
                        [&](const std::string& p) { state.doOpen(p); },
                        [&](const std::string& p) {
                            state.workspaceRoot = p;
                            state.fileTreeDirty = true;
                        });
                    SDL_free(droppedFile);
                }
            }

            // Handle keyboard shortcuts via KeybindingManager
            if (event.type == SDL_KEYDOWN && !io.WantTextInput) {
                int key = 0;
                auto sym = event.key.keysym.sym;
                if (sym >= SDLK_a && sym <= SDLK_z) key = 'A' + (sym - SDLK_a);
                else if (sym >= SDLK_0 && sym <= SDLK_9) key = '0' + (sym - SDLK_0);
                else if (sym == SDLK_EQUALS) key = '=';
                else if (sym == SDLK_MINUS) key = '-';
                else if (sym == SDLK_SLASH) key = '/';
                else if (sym == SDLK_SEMICOLON) key = ';';
                else if (sym == SDLK_PERIOD) key = '.';
                else if (sym == SDLK_BACKQUOTE) key = '`';

                int mods = WMOD_NONE;
                auto sdlMod = event.key.keysym.mod;
                if (sdlMod & KMOD_CTRL)  mods |= WMOD_CTRL;
                if (sdlMod & KMOD_SHIFT) mods |= WMOD_SHIFT;
                if (sdlMod & KMOD_ALT)   mods |= WMOD_ALT;

                if (key != 0 && mods != WMOD_NONE) {
                    KeyCombo combo{key, mods};
                    std::string action = state.keys.getAction(combo);
                    if (action == "edit.undo")       state.doUndo();
                    else if (action == "edit.redo")   state.doRedo();
                    else if (action == "search.find") state.showFind = !state.showFind;
                    else if (action == "file.save")   state.doSave();
                    else if (action == "file.new") {
                        std::string lang = state.active() ? state.active()->language : "python";
                        state.createBuffer(state.makeUntitledName(), "", lang);
                    }
                }
            }
        }

        // File watcher polling
        state.handleFileChanges();

        // Start frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Full-window dockspace
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags dockFlags = ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("##DockHost", nullptr, dockFlags);
        ImGui::PopStyleVar(3);
        ImGuiID dockId = ImGui::GetID("WhetstoneDS");
        ImGui::DockSpace(dockId, ImVec2(0, 0), ImGuiDockNodeFlags_None);
        ImGui::End();

        // ---------------------------------------------------------------
        //  Menu bar (as a window docked to the top)
        // ---------------------------------------------------------------
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", state.keys.getBinding("file.new").toString().c_str()))
                {
                    std::string lang = state.active() ? state.active()->language : "python";
                    state.createBuffer(state.makeUntitledName(), "", lang);
                }
                if (ImGui::MenuItem("Open...", state.keys.getBinding("file.open").toString().c_str()))
                {
                    auto path = FileDialog::openFile({"Open File", {"*.py","*.cpp","*.h","*.el","*.js","*.ts","*.java","*.rs","*.go"}, lastDialogPath});
                    if (!path.empty()) {
                        lastDialogPath = path;
                        state.doOpen(path);
                    }
                }
                if (ImGui::MenuItem("Open Folder...")) {
                    auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
                    if (!path.empty()) {
                        state.workspaceRoot = path;
                        state.fileTreeDirty = true;
                        lastDialogPath = path;
                    }
                }
                if (ImGui::MenuItem("Save", state.keys.getBinding("file.save").toString().c_str()))
                {
                    if (!state.active()) {
                        // no-op
                    } else if (state.active()->path.rfind("(untitled", 0) == 0) {
                        auto path = FileDialog::saveFile({"Save File", {"*.*"}, lastDialogPath});
                        if (!path.empty()) {
                            state.active()->path = path;
                            lastDialogPath = path;
                            state.doSave();
                        }
                    } else {
                        state.doSave();
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) done = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", state.keys.getBinding("edit.undo").toString().c_str(),
                                    false, state.active() ? state.active()->editor.canUndo() : false))
                    state.doUndo();
                if (ImGui::MenuItem("Redo", state.keys.getBinding("edit.redo").toString().c_str(),
                                    false, state.active() ? state.active()->editor.canRedo() : false))
                    state.doRedo();
                ImGui::Separator();
                if (ImGui::MenuItem("Find/Replace", state.keys.getBinding("search.find").toString().c_str()))
                    state.showFind = !state.showFind;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Whitespace", nullptr, &state.showWhitespace);
                ImGui::MenuItem("Show Minimap", nullptr, &state.showMinimap);
                ImGui::MenuItem("Show Annotations", nullptr, &state.showAnnotations);
                ImGui::MenuItem("LSP Servers...", nullptr, &state.showLspSettings);
                if (ImGui::BeginMenu("Layout")) {
                    if (ImGui::MenuItem("VSCode", nullptr, state.layoutPreset == LayoutPreset::VSCode))
                        state.layoutPreset = LayoutPreset::VSCode;
                    if (ImGui::MenuItem("Emacs", nullptr, state.layoutPreset == LayoutPreset::Emacs))
                        state.layoutPreset = LayoutPreset::Emacs;
                    if (ImGui::MenuItem("JetBrains", nullptr, state.layoutPreset == LayoutPreset::JetBrains))
                        state.layoutPreset = LayoutPreset::JetBrains;
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Language")) {
                if (ImGui::MenuItem("Python", nullptr, state.active() && state.active()->language == "python"))
                    state.setLanguage("python");
                if (ImGui::MenuItem("C++", nullptr, state.active() && state.active()->language == "cpp"))
                    state.setLanguage("cpp");
                if (ImGui::MenuItem("Elisp", nullptr, state.active() && state.active()->language == "elisp"))
                    state.setLanguage("elisp");
                if (ImGui::MenuItem("JavaScript", nullptr, state.active() && state.active()->language == "javascript"))
                    state.setLanguage("javascript");
                if (ImGui::MenuItem("TypeScript", nullptr, state.active() && state.active()->language == "typescript"))
                    state.setLanguage("typescript");
                if (ImGui::MenuItem("Java", nullptr, state.active() && state.active()->language == "java"))
                    state.setLanguage("java");
                if (ImGui::MenuItem("Rust", nullptr, state.active() && state.active()->language == "rust"))
                    state.setLanguage("rust");
                if (ImGui::MenuItem("Go", nullptr, state.active() && state.active()->language == "go"))
                    state.setLanguage("go");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Keybindings")) {
                for (auto p : KeybindingManager::availableProfiles()) {
                    if (ImGui::MenuItem(KeybindingManager::profileName(p), nullptr,
                                        state.keys.getProfile() == p))
                        state.keys.setProfile(p);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Native dialogs replace manual path popup (Step 84)

        // ---------------------------------------------------------------
        //  File Explorer (left panel)
        // ---------------------------------------------------------------
        ImGui::Begin("Explorer");
        ImGui::PushFont(uiFont);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "OPEN EDITORS");
        ImGui::Separator();
        // Show open buffers
        for (const auto& path : state.buffers.getOpenBuffers()) {
            auto* buf = state.bufferStates[path].get();
            std::string label = path;
            if (buf && buf->modified) label += " *";
            bool selected = state.active() && state.active()->path == path;
            if (ImGui::Selectable(label.c_str(), selected)) {
                state.switchToBuffer(path);
            }
        }
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "FILES");
        ImGui::Separator();
        state.refreshFileTree();
        if (state.fileTreeRoot.path.empty()) {
            ImGui::TextDisabled("(no workspace)");
        } else {
            RenderFileTree(state.fileTreeRoot, state);
        }
        ImGui::PopFont();
        ImGui::End();

        // ---------------------------------------------------------------
        //  Find / Replace bar (floating at top of editor)
        // ---------------------------------------------------------------
        if (state.showFind) {
            ImGui::Begin("Find & Replace", &state.showFind, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::PushFont(uiFont);
            ImGui::SetNextItemWidth(300);
            if (ImGui::InputText("Find", state.findBuf, sizeof(state.findBuf),
                                 ImGuiInputTextFlags_EnterReturnsTrue)) {
                state.doFind();
            }
            ImGui::SameLine();
            if (ImGui::Button("Find Next")) state.doFind();

            ImGui::SetNextItemWidth(300);
            ImGui::InputText("Replace", state.replaceBuf, sizeof(state.replaceBuf));
            ImGui::SameLine();
            if (ImGui::Button("Replace All")) state.doReplaceAll();
            ImGui::PopFont();
            ImGui::End();
        }

        // ---------------------------------------------------------------
        //  LSP Server Settings
        // ---------------------------------------------------------------
        if (state.showLspSettings) {
            ImGui::Begin("LSP Servers", &state.showLspSettings);
            ImGui::PushFont(uiFont);
            if (ImGui::Button("Auto-Detect")) {
                state.settings.autoDetect();
            }
            ImGui::Separator();
            for (auto& cfg : state.settings.getLSPServersMutable()) {
                ImGui::PushID(cfg.language.c_str());
                ImGui::Checkbox("Enabled", &cfg.enabled);
                ImGui::SameLine();
                ImGui::Text("%s", cfg.language.c_str());
                ImGui::SetNextItemWidth(320);
                InputTextStr("Path", &cfg.path);
                ImGui::SetNextItemWidth(320);
                if (InputTextStr("Args", &cfg.argsLine)) {
                    state.settings.syncArgs(cfg);
                }
                ImGui::Separator();
                ImGui::PopID();
            }
            ImGui::PopFont();
            ImGui::End();
        }

        // ---------------------------------------------------------------
        //  Editor (center) — editable text area
        // ---------------------------------------------------------------
        ImGui::Begin("Editor");
        ImGui::PushFont(monoFont);

        // Tab bar for the file
        if (ImGui::BeginTabBar("EditorTabs")) {
            if (state.buffers.bufferCount() == 0) {
                if (ImGui::BeginTabItem("Welcome")) {
                    RenderWelcome(state.welcome, state, lastDialogPath);
                    ImGui::EndTabItem();
                }
            } else {
                auto openBuffers = state.buffers.getOpenBuffers();
                for (const auto& path : openBuffers) {
                    auto* buf = state.bufferStates[path].get();
                    if (!buf) continue;
                    std::string tabLabel = path;
                    if (buf->modified) tabLabel += " *";
                    bool open = true;
                    if (ImGui::BeginTabItem(tabLabel.c_str(), &open)) {
                        if (!state.active() || state.active()->path != path) {
                            state.switchToBuffer(path);
                        }
                        // Editable text area fills available space
                        ImVec2 avail = ImGui::GetContentRegionAvail();
                        avail.y -= 4; // small margin

                        state.updateHighlights();
                        state.updateGenerated();
                        std::vector<int> errorLines;
                        std::vector<int> warningLines;
                        std::vector<DiagnosticRange> diagRanges;
                        std::vector<AnnotationMarker> annoMarkers;
                        std::vector<SuggestionMarker> suggestionMarkers;
                        std::vector<AnnotationConflictMarker> conflictMarkers;
                        std::string activeUri = EditorState::toFileUri(buf->path);
                        if (state.lsp) {
                            auto lspDiags = state.lsp->getDiagnosticsForUri(activeUri);
                            for (const auto& d : lspDiags) {
                                if (d.severity == 1) errorLines.push_back(d.range.start.line);
                                else if (d.severity == 2) warningLines.push_back(d.range.start.line);
                                DiagnosticRange dr;
                                dr.startLine = d.range.start.line;
                                dr.startCol = d.range.start.character;
                                dr.endLine = d.range.end.line;
                                dr.endCol = d.range.end.character;
                                dr.severity = d.severity;
                                dr.message = d.message;
                                diagRanges.push_back(std::move(dr));
                            }
                        }
                        for (const auto& d : state.whetstoneDiagnostics) {
                            if (d.uri != activeUri) continue;
                            if (d.severity == 1) errorLines.push_back(d.line);
                            else if (d.severity == 2) warningLines.push_back(d.line);
                            DiagnosticRange dr;
                            dr.startLine = d.line;
                            dr.startCol = d.character;
                            dr.endLine = d.line;
                            dr.endCol = d.character + 1;
                            dr.severity = d.severity;
                            dr.message = d.message;
                            diagRanges.push_back(std::move(dr));
                        }
                        if (state.active()) {
                            Module* ast = state.active()->sync.getAST();
                            if (ast) {
                                collectAnnotationMarkers(ast, annoMarkers);
                                std::vector<AnnotationConflict> conflicts;
                                collectAnnotationConflicts(ast, conflicts);
                                for (const auto& c : conflicts) {
                                    AnnotationConflictMarker m;
                                    m.childLine = c.childLine;
                                    m.parentLine = c.parentLine;
                                    m.message = c.message;
                                    m.childAnnoId = c.childAnnoId;
                                    m.parentAnnoId = c.parentAnnoId;
                                    m.parentStrategy = c.parentStrategy;
                                    conflictMarkers.push_back(std::move(m));
                                }
                                for (const auto& s : state.suggestions) {
                                    if (s.confidence <= 0.5) continue;
                                    ASTNode* target = findNodeById(ast, s.nodeId);
                                    if (!target || !target->hasSpan()) continue;
                                    SuggestionMarker sm;
                                    sm.line = target->spanStartLine;
                                    sm.confidence = s.confidence;
                                    sm.reason = s.reason;
                                    sm.annotationType = s.annotationType;
                                    sm.strategy = s.strategy;
                                    sm.nodeId = s.nodeId;
                                    sm.label = "@" + s.annotationType.substr(0, s.annotationType.find("Annotation")) +
                                               "(" + s.strategy + ")";
                                    suggestionMarkers.push_back(std::move(sm));
                                }
                            }
                        }

                        CodeEditorOptions opts;
                        opts.showWhitespace = state.showWhitespace;
                        opts.mode = &buf->mode;
                        opts.enableFolding = true;
                        opts.showMinimap = state.showMinimap;
                        opts.showAnnotations = state.showAnnotations;
                        if (state.layoutPreset == LayoutPreset::Emacs) opts.annotationLayout = 1;
                        else if (state.layoutPreset == LayoutPreset::JetBrains) opts.annotationLayout = 2;
                        else opts.annotationLayout = 0;
                        opts.errorLines = &errorLines;
                        opts.warningLines = &warningLines;
                        opts.diagnostics = &diagRanges;
                        opts.annotations = &annoMarkers;
                        opts.suggestions = &suggestionMarkers;
                        opts.conflicts = &conflictMarkers;
                        opts.syncScrollX = &buf->splitScrollX;
                        opts.syncScrollY = &buf->splitScrollY;
                        opts.scrollMaster = true;

                        ImGui::BeginTable("##editorSplit", 2,
                                          ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
                        ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.55f);
                        ImGui::TableSetupColumn("Generated", ImGuiTableColumnFlags_WidthStretch, 0.45f);
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        CodeEditorResult res = buf->widget.render("##editor",
                            buf->editBuf, buf->highlights, opts, ImVec2(0, avail.y), monoFont);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::BeginGroup();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted("Generated");
                        ImGui::SameLine();
                        const char* targetLabels[] = {"Python", "C++", "Elisp"};
                        const char* targetValues[] = {"python", "cpp", "elisp"};
                        int targetIndex = 0;
                        for (int i = 0; i < IM_ARRAYSIZE(targetValues); ++i) {
                            if (buf->generatedLanguage == targetValues[i]) {
                                targetIndex = i;
                                break;
                            }
                        }
                        ImGui::SetNextItemWidth(120.0f);
                        if (ImGui::Combo("##targetLang", &targetIndex,
                                         targetLabels, IM_ARRAYSIZE(targetLabels))) {
                            buf->generatedLanguage = targetValues[targetIndex];
                            buf->generatedMode.setLanguage(buf->generatedLanguage);
                            buf->generatedHighlightsDirty = true;
                            state.updateGenerated();
                        }
                        ImVec2 genAvail = ImGui::GetContentRegionAvail();
                        CodeEditorOptions genOpts;
                        genOpts.readOnly = true;
                        genOpts.showWhitespace = state.showWhitespace;
                        genOpts.mode = &buf->generatedMode;
                        genOpts.showCurrentLine = false;
                        genOpts.highlightLine = buf->generatedHighlightLine;
                        genOpts.syncScrollX = &buf->splitScrollX;
                        genOpts.syncScrollY = &buf->splitScrollY;
                        genOpts.scrollMaster = false;
                        buf->generatedWidget.render("##generated",
                            buf->generatedBuf, buf->generatedHighlights, genOpts,
                            ImVec2(0, genAvail.y), monoFont);
                        ImGui::EndGroup();
                        ImGui::EndTable();

                        state.updateCursorPos(res.cursorByte);
                        if (res.lineClicked) {
                            Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                            if (ast) {
                                int genLines = countLines(buf->generatedBuf);
                                int targetLine = std::max(0, std::min(res.clickedLine, genLines - 1));
                                buf->generatedHighlightLine = targetLine;
                            } else {
                                buf->generatedHighlightLine = -1;
                            }
                        }
                        if (res.changed) {
                            state.onTextChanged();
                            state.completionPending = true;
                            state.completionLastChange = ImGui::GetTime();
                            state.completionVisible = false;
                            state.completionSelected = 0;
                            if (state.lsp) state.lsp->clearCompletionItems();
                            if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                int cursor = state.active()->widget.getCursor();
                                if (cursor > 0 && state.active()->editBuf[cursor - 1] == '(') {
                                    int lineZero = std::max(0, state.active()->cursorLine - 1);
                                    int colZero = std::max(0, state.active()->cursorCol - 1);
                                    state.lsp->requestSignatureHelp(EditorState::toFileUri(state.active()->path),
                                                                    lineZero, colZero);
                                } else if (cursor > 0 && state.active()->editBuf[cursor - 1] == ')') {
                                    state.lsp->clearSignatureHelp();
                                }
                            }
                            state.analysisPending = true;
                            state.analysisLastChange = ImGui::GetTime();
                        }

                        if (res.suggestionClicked) {
                            state.suggestionPopup.annotationType = res.clickedSuggestion.annotationType;
                            state.suggestionPopup.strategy = res.clickedSuggestion.strategy;
                            state.suggestionPopup.reason = res.clickedSuggestion.reason;
                            state.suggestionPopup.confidence = res.clickedSuggestion.confidence;
                            state.suggestionPopup.nodeId = res.clickedSuggestion.nodeId;
                            state.showSuggestionPopup = true;
                            ImGui::OpenPopup("SuggestionPopup");
                        }

                        double now = ImGui::GetTime();
                        if (state.completionPending && (now - state.completionLastChange) > 0.2) {
                            if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                int lineZero = std::max(0, state.active()->cursorLine - 1);
                                int colZero = std::max(0, state.active()->cursorCol - 1);
                                state.lsp->requestCompletion(EditorState::toFileUri(state.active()->path),
                                                             lineZero, colZero);
                            }
                            state.completionPending = false;
                        }

                        if (state.analysisPending && (now - state.analysisLastChange) > 0.5) {
                            if (state.active()) {
                                auto result = state.pipeline.run(state.active()->editBuf,
                                                                 state.active()->language,
                                                                 state.active()->language);
                                if (result.success) {
                                    state.whetstoneDiagnostics =
                                        collectWhetstoneDiagnostics(result.validationDiags,
                                                                    result.violations,
                                                                    EditorState::toFileUri(state.active()->path));
                                } else {
                                    state.whetstoneDiagnostics.clear();
                                }
                                Module* ast = state.active()->sync.getAST();
                                if (ast) {
                                    MemoryStrategyInference inf;
                                    state.suggestions = inf.inferAnnotations(ast);
                                } else {
                                    state.suggestions.clear();
                                }
                            }
                            state.analysisPending = false;
                        }

                        if (state.lsp && state.active()) {
                            auto items = state.lsp->getCompletionItems();
                            int cursor = state.active()->widget.getCursor();
                            std::string prefix = EditorState::wordPrefixAt(state.active()->editBuf, cursor);
                            std::vector<LSPClient::CompletionItem> filtered;
                            filtered.reserve(items.size());
                            for (const auto& item : items) {
                                const std::string& key = item.filterText.empty() ? item.label : item.filterText;
                                if (prefix.empty() || key.rfind(prefix, 0) == 0) filtered.push_back(item);
                            }

                            state.completionVisible = !filtered.empty();
                            if (state.completionVisible) {
                                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x + 20.0f,
                                                                 ImGui::GetWindowPos().y + 60.0f));
                                ImGui::BeginChild("##completionPopup", ImVec2(300, 150), true);
                                int maxIndex = std::max(0, (int)filtered.size() - 1);
                                if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                                    state.completionSelected = std::min(state.completionSelected + 1, maxIndex);
                                }
                                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                                    state.completionSelected = std::max(state.completionSelected - 1, 0);
                                }
                                bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Tab);
                                bool dismiss = ImGui::IsKeyPressed(ImGuiKey_Escape);

                                if (dismiss) {
                                    state.lsp->clearCompletionItems();
                                    state.completionVisible = false;
                                }

                                for (int i = 0; i < (int)filtered.size(); ++i) {
                                    std::string itemLabel = filtered[i].label;
                                    if (filtered[i].kind != 0) {
                                        itemLabel = "[" + std::to_string(filtered[i].kind) + "] " + itemLabel;
                                    }
                                    bool selected = (i == state.completionSelected);
                                    if (ImGui::Selectable(itemLabel.c_str(), selected)) {
                                        state.completionSelected = i;
                                        accept = true;
                                    }
                                }

                                if (accept && !filtered.empty()) {
                                    const auto& item = filtered[state.completionSelected];
                                    int start = EditorState::wordStartAt(state.active()->editBuf, cursor);
                                    state.applyCompletion(state.active(), item.insertText, start, cursor);
                                    state.lsp->clearCompletionItems();
                                    state.completionVisible = false;
                                }
                                ImGui::EndChild();
                            }
                        }

                        if (state.showSuggestionPopup) {
                            if (ImGui::BeginPopupModal("SuggestionPopup", &state.showSuggestionPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
                                ImGui::Text("%s(%s)", state.suggestionPopup.annotationType.c_str(),
                                            state.suggestionPopup.strategy.c_str());
                                ImGui::Separator();
                                ImGui::TextWrapped("%s", state.suggestionPopup.reason.c_str());
                                ImGui::Text("Confidence: %.2f", state.suggestionPopup.confidence);
                                if (ImGui::Button("Apply")) {
                                    Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                                    if (ast) {
                                        ASTNode* target = findNodeById(ast, state.suggestionPopup.nodeId);
                                        if (target) {
                                            auto* anno = createAnnotationNode(state.suggestionPopup.annotationType,
                                                                             state.suggestionPopup.strategy);
                                            if (anno) {
                                                if (target->hasSpan()) {
                                                    anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                                  target->spanEndLine, target->spanEndCol);
                                                }
                                                state.mutator.setRoot(ast);
                                                auto res2 = state.mutator.insertNode(target->id, "annotations", anno);
                                                if (!res2.error.empty()) state.outputLog += res2.error + "\n";
                                            }
                                        }
                                    }
                                    state.showSuggestionPopup = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::SameLine();
                                if (ImGui::Button("Cancel")) {
                                    state.showSuggestionPopup = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::EndPopup();
                            }
                        }

                        if (state.lsp && state.active()) {
                            if (ImGui::IsWindowHovered()) {
                                ImVec2 mouse = ImGui::GetMousePos();
                                if (mouse.x != state.hoverLastMouse.x || mouse.y != state.hoverLastMouse.y) {
                                    state.hoverLastMouse = mouse;
                                    state.hoverLastMove = ImGui::GetTime();
                                    state.hoverPending = true;
                                    state.lsp->clearHover();
                                }
                                if (state.hoverPending && (ImGui::GetTime() - state.hoverLastMove) > 0.5) {
                                    if (state.active()->path.rfind("(untitled", 0) != 0) {
                                        int lineZero = std::max(0, state.active()->cursorLine - 1);
                                        int colZero = std::max(0, state.active()->cursorCol - 1);
                                        state.lsp->requestHover(EditorState::toFileUri(state.active()->path),
                                                               lineZero, colZero);
                                    }
                                    state.hoverPending = false;
                                }
                                std::string hover = state.lsp->getHoverContents();
                                if (!hover.empty()) {
                                    ImGui::BeginTooltip();
                                    ImGui::TextUnformatted(hover.c_str());
                                    ImGui::EndTooltip();
                                }
                            }

                            auto sig = state.lsp->getSignatureHelp();
                            if (!sig.label.empty()) {
                                ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x + 20.0f,
                                                                 ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 80.0f));
                                ImGui::BeginChild("##signatureHelp", ImVec2(420, 60), true);
                                ImGui::TextUnformatted(sig.label.c_str());
                                ImGui::EndChild();
                            }
                        }

                        if (ImGui::BeginPopupContextWindow("AnnotationContext", ImGuiPopupFlags_MouseButtonRight)) {
                            Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                            int lineZero = state.active() ? std::max(0, state.active()->cursorLine - 1) : 0;
                            ASTNode* target = ast ? findAnnotationTarget(ast, lineZero) : nullptr;
                            if (!target) {
                                ImGui::TextDisabled("No annotatable symbol on this line.");
                            } else {
                                ImGui::Text("Target: %s", nodeDisplayName(target).c_str());
                                ImGui::Separator();

                                if (ImGui::BeginMenu("Add Annotation")) {
                                    if (ImGui::MenuItem("@Reclaim(Tracing)")) {
                                        auto* anno = createAnnotationNode("ReclaimAnnotation", "Tracing");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    if (ImGui::MenuItem("@Owner(Single)")) {
                                        auto* anno = createAnnotationNode("OwnerAnnotation", "Single");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    if (ImGui::MenuItem("@Deallocate(Explicit)")) {
                                        auto* anno = createAnnotationNode("DeallocateAnnotation", "Explicit");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    if (ImGui::MenuItem("@Lifetime(RAII)")) {
                                        auto* anno = createAnnotationNode("LifetimeAnnotation", "RAII");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    if (ImGui::MenuItem("@Allocate(Static)")) {
                                        auto* anno = createAnnotationNode("AllocateAnnotation", "Static");
                                        if (anno) {
                                            anno->setSpan(target->spanStartLine, target->spanStartCol,
                                                          target->spanEndLine, target->spanEndCol);
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.insertNode(target->id, "annotations", anno);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                const auto& annos = target->getChildren("annotations");
                                if (ImGui::BeginMenu("Edit Annotation", !annos.empty())) {
                                    for (auto* anno : annos) {
                                        std::string label = annotationLabel(anno);
                                        if (ImGui::BeginMenu(label.c_str())) {
                                            if (anno->conceptType == "ReclaimAnnotation") {
                                                if (ImGui::MenuItem("Tracing")) {
                                                    state.mutator.setRoot(ast);
                                                    state.mutator.setProperty(anno->id, "strategy", "Tracing");
                                                }
                                            } else if (anno->conceptType == "OwnerAnnotation") {
                                                if (ImGui::MenuItem("Single")) {
                                                    state.mutator.setRoot(ast);
                                                    state.mutator.setProperty(anno->id, "strategy", "Single");
                                                }
                                            } else if (anno->conceptType == "DeallocateAnnotation") {
                                                if (ImGui::MenuItem("Explicit")) {
                                                    state.mutator.setRoot(ast);
                                                    state.mutator.setProperty(anno->id, "strategy", "Explicit");
                                                }
                                            } else if (anno->conceptType == "LifetimeAnnotation") {
                                                if (ImGui::MenuItem("RAII")) {
                                                    state.mutator.setRoot(ast);
                                                    state.mutator.setProperty(anno->id, "strategy", "RAII");
                                                }
                                            } else if (anno->conceptType == "AllocateAnnotation") {
                                                if (ImGui::MenuItem("Static")) {
                                                    state.mutator.setRoot(ast);
                                                    state.mutator.setProperty(anno->id, "strategy", "Static");
                                                }
                                            }
                                            ImGui::EndMenu();
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                if (ImGui::BeginMenu("Remove Annotation", !annos.empty())) {
                                    for (auto* anno : annos) {
                                        std::string label = annotationLabel(anno);
                                        if (ImGui::MenuItem(label.c_str())) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.deleteNode(anno->id);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    ImGui::EndMenu();
                                }

                                std::vector<AnnotationConflict> conflicts;
                                collectAnnotationConflicts(ast, conflicts);
                                bool conflictOnLine = false;
                                AnnotationConflict lineConflict;
                                for (const auto& c : conflicts) {
                                    if (c.childLine == lineZero || c.parentLine == lineZero) {
                                        lineConflict = c;
                                        conflictOnLine = true;
                                        break;
                                    }
                                }
                                if (ImGui::BeginMenu("Resolve Conflict", conflictOnLine)) {
                                    if (conflictOnLine) {
                                        if (ImGui::MenuItem("Remove child annotation")) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.deleteNode(lineConflict.childAnnoId);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                        if (ImGui::MenuItem("Match parent strategy")) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.setProperty(lineConflict.childAnnoId,
                                                                                "strategy",
                                                                                lineConflict.parentStrategy);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                        }
                                    }
                                    ImGui::EndMenu();
                                }
                            }
                            ImGui::EndPopup();
                        }

                        ImGui::EndTabItem();
                    }
                    if (!open) {
                        state.buffers.closeBuffer(path);
                        state.bufferStates.erase(path);
                        if (path.rfind("(untitled", 0) != 0) state.watcher.unwatch(path);
                        if (state.buffers.bufferCount() > 0) {
                            state.switchToBuffer(state.buffers.getOpenBuffers().front());
                        } else {
                            state.activeBuffer = nullptr;
                        }
                    }
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::PopFont();
        ImGui::End();

        // ---------------------------------------------------------------
        //  Bottom panel — Output / AST / Highlighted Preview
        // ---------------------------------------------------------------
        ImGui::Begin("Panel");

        if (ImGui::BeginTabBar("PanelTabs")) {
            // Output log
            if (ImGui::BeginTabItem("Output")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##outputScroll", ImVec2(0, 0), false);
                ImGui::TextUnformatted(state.outputLog.c_str());
                // Auto-scroll to bottom
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20)
                    ImGui::SetScrollHereY(1.0f);
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Problems (LSP diagnostics)
            if (ImGui::BeginTabItem("Problems")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##problemsScroll", ImVec2(0, 0), false);
                auto diags = state.lsp ? state.lsp->getDiagnostics() : std::vector<LSPClient::Diagnostic>{};
                const auto& whetDiags = state.whetstoneDiagnostics;
                if (diags.empty() && whetDiags.empty()) {
                    ImGui::TextDisabled("(no diagnostics)");
                } else {
                    for (const auto& d : diags) {
                        const char* sev = "Unknown";
                        if (d.severity == 1) sev = "Error";
                        else if (d.severity == 2) sev = "Warning";
                        else if (d.severity == 3) sev = "Info";
                        else if (d.severity == 4) sev = "Hint";

                        std::string path = EditorState::fromFileUri(d.uri);
                        std::string label = "[" + std::string(sev) + "] " + d.message +
                                            " (" + path + ":" + std::to_string(d.range.start.line + 1) +
                                            ":" + std::to_string(d.range.start.character + 1) + ")";
                        if (ImGui::Selectable(label.c_str())) {
                            if (!path.empty()) {
                                if (state.buffers.hasBuffer(path)) state.switchToBuffer(path);
                                else state.doOpen(path);
                                state.jumpTo(state.active(), d.range.start.line, d.range.start.character);
                            }
                        }
                    }
                    for (const auto& d : whetDiags) {
                        const char* sev = "Unknown";
                        if (d.severity == 1) sev = "Error";
                        else if (d.severity == 2) sev = "Warning";
                        else if (d.severity == 3) sev = "Info";

                        std::string path = EditorState::fromFileUri(d.uri);
                        std::string label = "[" + std::string(sev) + "] " + d.message +
                                            " (" + path + ":" + std::to_string(d.line + 1) +
                                            ":" + std::to_string(d.character + 1) + ")";
                        if (ImGui::Selectable(label.c_str())) {
                            if (!path.empty()) {
                                if (state.buffers.hasBuffer(path)) state.switchToBuffer(path);
                                else state.doOpen(path);
                                state.jumpTo(state.active(), d.line, d.character);
                            }
                        }
                    }
                }
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // AST view
            if (ImGui::BeginTabItem("AST")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##astScroll", ImVec2(0, 0), false);
                Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                if (ast) {
                    // Show basic AST info
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f),
                                       "Module: %s  [%s]",
                                       ast->name.c_str(), ast->targetLanguage.c_str());
                    ImGui::Separator();
                    auto functions = ast->getChildren("functions");
                    for (size_t i = 0; i < functions.size(); ++i) {
                        auto* fn = static_cast<Function*>(functions[i]);
                        ImGui::TextColored(ImVec4(0.86f, 0.86f, 0.55f, 1.0f),
                                           "  Function: %s", fn->name.c_str());
                        auto params = fn->getChildren("parameters");
                        for (auto* p : params) {
                            auto* param = static_cast<Parameter*>(p);
                            ImGui::TextColored(ImVec4(0.6f, 0.78f, 0.9f, 1.0f),
                                               "    param: %s", param->name.c_str());
                        }
                        auto body = fn->getChildren("body");
                        ImGui::Text("    body: %d statement(s)", (int)body.size());
                        auto annos = fn->getChildren("annotations");
                        for (auto* a : annos) {
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                                               "    @%s", a->conceptType.c_str());
                        }
                    }
                } else {
                    ImGui::TextDisabled("(no AST — enter some code)");
                }
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Syntax-highlighted preview
            if (ImGui::BeginTabItem("Highlighted")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##hlScroll", ImVec2(0, 0), false);
                state.updateHighlights();
                if (state.active())
                    RenderHighlightedText(state.active()->editBuf, state.active()->highlights);
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Generated code preview
            if (ImGui::BeginTabItem("Generated")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##genScroll", ImVec2(0, 0), false);
                Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                state.updateGenerated();
                if (ast && state.active()) {
                    ImGui::TextUnformatted(state.active()->generatedBuf.c_str());
                } else {
                    ImGui::TextDisabled("(no AST)");
                }
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();

        // ---------------------------------------------------------------
        //  Memory Strategies dashboard
        // ---------------------------------------------------------------
        ImGui::Begin("Memory Strategies");
        ImGui::PushFont(uiFont);
        Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
        if (!ast) {
            ImGui::TextDisabled("(no AST)");
        } else {
            std::vector<AnnotationEntry> entries;
            collectAnnotationEntries(ast, entries);
            std::map<std::string, int> counts;
            for (const auto& e : entries) counts[e.label]++;

            ImGui::Text("Annotations");
            ImGui::Separator();
            for (const auto& [label, count] : counts) {
                ImGui::Text("%s: %d", label.c_str(), count);
            }
            ImGui::Spacing();

            ImGui::Text("Details");
            ImGui::Separator();
            for (const auto& e : entries) {
                std::string lineInfo = e.line >= 0 ? ("L" + std::to_string(e.line + 1)) : "-";
                std::string row = e.label + " — " + e.nodeName + " (" + lineInfo + ")";
                if (ImGui::Selectable(row.c_str())) {
                    if (state.active()) state.jumpTo(state.active(), e.line, 0);
                }
            }

            ImGui::Spacing();
            ImGui::Text("Suggestions");
            ImGui::Separator();
            for (const auto& s : state.suggestions) {
                if (s.confidence < 0.5) continue;
                std::string row = s.annotationType + "(" + s.strategy + ") — " +
                                  s.nodeId + " (" + std::to_string(s.confidence) + ")";
                ImGui::TextUnformatted(row.c_str());
            }

            if (ImGui::Button("Export JSON")) {
                auto j = buildAnnotationSummaryJson(entries);
                state.outputLog += "Annotation summary:\\n" + j.dump(2) + "\\n";
            }
        }
        ImGui::PopFont();
        ImGui::End();

        // ---------------------------------------------------------------
        //  Status bar
        // ---------------------------------------------------------------
        {
            ImGuiWindowFlags sbFlags = ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoSavedSettings;
            float sbHeight = ImGui::GetFrameHeight() + 2;
            ImVec2 sbPos(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - sbHeight);
            ImVec2 sbSize(viewport->WorkSize.x, sbHeight);
            ImGui::SetNextWindowPos(sbPos);
            ImGui::SetNextWindowSize(sbSize);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.00f, 0.47f, 0.84f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
            ImGui::Begin("##StatusBar", nullptr, sbFlags);
            ImGui::PushFont(uiFont);

            // Left side: line/col
            if (state.active())
                ImGui::Text("Ln %d, Col %d", state.active()->cursorLine, state.active()->cursorCol);
            else
                ImGui::Text("Ln -, Col -");
            ImGui::SameLine(0, 30);

            // Language
            if (state.active())
                ImGui::Text("%s", state.active()->language.c_str());
            else
                ImGui::Text("-");
            ImGui::SameLine(0, 30);

            // Keybinding profile
            ImGui::Text("Keys: %s", KeybindingManager::profileName(state.keys.getProfile()));
            ImGui::SameLine(0, 30);

            // Modified indicator
            if (state.active() && state.active()->modified)
                ImGui::Text("Modified");
            else
                ImGui::Text("Saved");

            ImGui::SameLine(0, 30);
            ImGui::Text("UTF-8");

            ImGui::PopFont();
            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        // Render
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.12f, 0.12f, 0.12f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
