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
#include "ast/Generator.h"

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>

// ---------------------------------------------------------------------------
//  Editor application state
// ---------------------------------------------------------------------------
struct BufferState {
    TextEditor        editor;
    TextASTSync       sync;
    CodeEditorWidget  widget;
    EditorMode        mode;
    std::string       language    = "python";
    std::string       path        = "(untitled)";
    bool              modified    = false;
    int               cursorLine  = 1;
    int               cursorCol   = 1;
    std::string       editBuf;
    std::vector<HighlightSpan> highlights;
    bool              highlightsDirty = true;
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
    std::string       workspaceRoot;
    FileTree          fileTree;
    FileNode          fileTreeRoot;
    bool              fileTreeDirty = true;

    BufferState* active() { return activeBuffer; }

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
        state->mode.setLanguage(language);
        state->editor.setContent(content, language);
        state->sync.setText(content, language);
        state->sync.syncNow();
        state->editBuf = content;
        state->highlightsDirty = true;
        state->modified = false;
        buffers.openBuffer(path, content, language);
        activeBuffer = state.get();
        bufferStates[path] = std::move(state);
    }

    void switchToBuffer(const std::string& path) {
        if (!buffers.hasBuffer(path)) return;
        buffers.switchToBuffer(path);
        activeBuffer = bufferStates[path].get();
    }

    void init() {
        std::string defaultContent =
            "def add(x, y):\n"
            "    \"\"\"Add two numbers.\"\"\"\n"
            "    result = x + y\n"
            "    return result\n"
            "\n"
            "def multiply(a, b):\n"
            "    return a * b\n"
            "\n"
            "# Calculate the sum of a list\n"
            "def calculate_sum(items):\n"
            "    total = 0\n"
            "    for item in items:\n"
            "        total = total + item\n"
            "    return total\n";

        createBuffer(makeUntitledName(), defaultContent, "python");
        outputLog = "Whetstone Editor ready.\n";
        workspaceRoot = std::filesystem::current_path().string();
        fileTreeDirty = true;
    }

    void setLanguage(const std::string& lang) {
        if (!active()) return;
        active()->language = lang;
        active()->mode.setLanguage(lang);
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
        active()->modified = true;
    }

    void doUndo() {
        if (!active()) return;
        active()->editor.undo();
        active()->editBuf = active()->editor.getContent();
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
    }

    void doRedo() {
        if (!active()) return;
        active()->editor.redo();
        active()->editBuf = active()->editor.getContent();
        active()->sync.setText(active()->editBuf, active()->language);
        active()->sync.syncNow();
        active()->highlightsDirty = true;
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
            outputLog += "Opened: " + path + "\n";
        } else {
            outputLog += "Error opening: " + path + "\n";
        }
    }

    void updateHighlights() {
        if (!active()) return;
        if (!active()->highlightsDirty) return;
        active()->highlights = SyntaxHighlighter::highlight(active()->editBuf, active()->language);
        active()->highlightsDirty = false;
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
        //  Editor (center) — editable text area
        // ---------------------------------------------------------------
        ImGui::Begin("Editor");
        ImGui::PushFont(monoFont);

        // Tab bar for the file
        if (ImGui::BeginTabBar("EditorTabs")) {
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
                    CodeEditorOptions opts;
                    opts.showWhitespace = state.showWhitespace;
                    opts.mode = &buf->mode;
                    opts.enableFolding = true;
                    opts.showMinimap = state.showMinimap;

                    CodeEditorResult res = buf->widget.render("##editor",
                        buf->editBuf, buf->highlights, opts, avail, monoFont);
                    state.updateCursorPos(res.cursorByte);
                    if (res.changed) {
                        state.onTextChanged();
                    }

                    ImGui::EndTabItem();
                }
                if (!open) {
                    state.buffers.closeBuffer(path);
                    state.bufferStates.erase(path);
                    if (state.buffers.bufferCount() > 0) {
                        state.switchToBuffer(state.buffers.getOpenBuffers().front());
                    } else {
                        state.activeBuffer = nullptr;
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
                if (ast) {
                    std::string generated;
                    if (state.active()->language == "python") {
                        PythonGenerator gen;
                        generated = gen.generate(ast);
                    } else if (state.active()->language == "cpp") {
                        CppGenerator gen;
                        generated = gen.generate(ast);
                    } else if (state.active()->language == "elisp") {
                        ElispGenerator gen;
                        generated = gen.generate(ast);
                    }
                    ImGui::TextUnformatted(generated.c_str());
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
