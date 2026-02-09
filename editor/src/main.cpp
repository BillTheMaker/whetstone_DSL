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
#include "ast/Generator.h"

#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>

// ---------------------------------------------------------------------------
//  Editor application state
// ---------------------------------------------------------------------------
struct EditorState {
    TextEditor        editor;
    TextASTSync       sync;
    KeybindingManager keys;
    EditorMode        mode;
    std::string       language    = "python";
    std::string       filePath    = "(untitled)";
    bool              modified    = false;
    int               cursorLine  = 1;
    int               cursorCol   = 1;

    // Edit buffer for ImGui InputTextMultiline — kept in sync with TextEditor
    std::string       editBuf;
    bool              bufDirty    = true;  // true = need to push editBuf → editor

    // Find/Replace state
    bool              showFind    = false;
    char              findBuf[256]    = {};
    char              replaceBuf[256] = {};
    int               lastFindPos     = 0;

    // Bottom panel
    int               bottomTab   = 0;  // 0=Output, 1=AST, 2=Highlighted
    std::string       outputLog;

    // Highlight cache (recomputed on text change)
    std::vector<HighlightSpan> highlights;
    bool              highlightsDirty = true;

    // Custom editor widget state
    CodeEditorWidget  codeWidget;
    bool              showWhitespace = false;
    bool              showMinimap = false;

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

        editor.setContent(defaultContent, language);
        mode.setLanguage(language);
        sync.setText(defaultContent, language);
        sync.syncNow();
        editBuf = defaultContent;
        highlightsDirty = true;
        outputLog = "Whetstone Editor ready.\n";
    }

    void setLanguage(const std::string& lang) {
        language = lang;
        mode.setLanguage(lang);
        editor.setContent(editBuf, lang);
        sync.setText(editBuf, lang);
        sync.syncNow();
        highlightsDirty = true;
    }

    // Called after editBuf changes (from ImGui input)
    void onTextChanged() {
        editor.setContent(editBuf, language);
        sync.setText(editBuf, language);
        sync.syncNow();
        highlightsDirty = true;
        modified = true;
    }

    void doUndo() {
        editor.undo();
        editBuf = editor.getContent();
        sync.setText(editBuf, language);
        sync.syncNow();
        highlightsDirty = true;
    }

    void doRedo() {
        editor.redo();
        editBuf = editor.getContent();
        sync.setText(editBuf, language);
        sync.syncNow();
        highlightsDirty = true;
    }

    void doFind() {
        if (strlen(findBuf) == 0) return;
        int pos = editor.find(findBuf, lastFindPos);
        if (pos >= 0) {
            lastFindPos = pos + 1;
            // Calculate line/col from position
            int line = 1, col = 1;
            for (int i = 0; i < pos && i < (int)editBuf.size(); ++i) {
                if (editBuf[i] == '\n') { ++line; col = 1; } else { ++col; }
            }
            cursorLine = line;
            cursorCol = col;
            outputLog += "Found \"" + std::string(findBuf) + "\" at line " +
                         std::to_string(line) + ", col " + std::to_string(col) + "\n";
        } else {
            lastFindPos = 0;
            outputLog += "\"" + std::string(findBuf) + "\" not found.\n";
        }
    }

    void doReplaceAll() {
        if (strlen(findBuf) == 0) return;
        int count = editor.replaceAll(findBuf, replaceBuf);
        if (count > 0) {
            editBuf = editor.getContent();
            sync.setText(editBuf, language);
            sync.syncNow();
            highlightsDirty = true;
            modified = true;
        }
        outputLog += "Replaced " + std::to_string(count) + " occurrence(s).\n";
    }

    void doSave() {
        if (filePath == "(untitled)") return;
        std::ofstream out(filePath);
        if (out.is_open()) {
            out << editBuf;
            out.close();
            modified = false;
            outputLog += "Saved: " + filePath + "\n";
        } else {
            outputLog += "Error saving: " + filePath + "\n";
        }
    }

    void doOpen(const std::string& path) {
        std::ifstream in(path);
        if (in.is_open()) {
            std::ostringstream ss;
            ss << in.rdbuf();
            editBuf = ss.str();
            filePath = path;
            // Detect language from extension
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
            editor.setContent(editBuf, language);
            sync.setText(editBuf, language);
            sync.syncNow();
            highlightsDirty = true;
            modified = false;
            outputLog += "Opened: " + path + "\n";
        } else {
            outputLog += "Error opening: " + path + "\n";
        }
    }

    void updateHighlights() {
        if (!highlightsDirty) return;
        highlights = SyntaxHighlighter::highlight(editBuf, language);
        highlightsDirty = false;
    }

    void updateCursorPos(int bytePos) {
        cursorLine = 1;
        cursorCol = 1;
        for (int i = 0; i < bytePos && i < (int)editBuf.size(); ++i) {
            if (editBuf[i] == '\n') { ++cursorLine; cursorCol = 1; }
            else { ++cursorCol; }
        }
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

    // File open dialog state
    static char openPathBuf[512] = {};

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
                        state.editBuf.clear();
                        state.onTextChanged();
                        state.filePath = "(untitled)";
                        state.modified = false;
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
                    state.editBuf.clear();
                    state.onTextChanged();
                    state.filePath = "(untitled)";
                    state.modified = false;
                }
                if (ImGui::MenuItem("Open...", state.keys.getBinding("file.open").toString().c_str()))
                {
                    ImGui::OpenPopup("OpenFilePopup");
                }
                if (ImGui::MenuItem("Save", state.keys.getBinding("file.save").toString().c_str()))
                {
                    state.doSave();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) done = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", state.keys.getBinding("edit.undo").toString().c_str(),
                                    false, state.editor.canUndo()))
                    state.doUndo();
                if (ImGui::MenuItem("Redo", state.keys.getBinding("edit.redo").toString().c_str(),
                                    false, state.editor.canRedo()))
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
                if (ImGui::MenuItem("Python", nullptr, state.language == "python"))
                    state.setLanguage("python");
                if (ImGui::MenuItem("C++", nullptr, state.language == "cpp"))
                    state.setLanguage("cpp");
                if (ImGui::MenuItem("Elisp", nullptr, state.language == "elisp"))
                    state.setLanguage("elisp");
                if (ImGui::MenuItem("JavaScript", nullptr, state.language == "javascript"))
                    state.setLanguage("javascript");
                if (ImGui::MenuItem("TypeScript", nullptr, state.language == "typescript"))
                    state.setLanguage("typescript");
                if (ImGui::MenuItem("Java", nullptr, state.language == "java"))
                    state.setLanguage("java");
                if (ImGui::MenuItem("Rust", nullptr, state.language == "rust"))
                    state.setLanguage("rust");
                if (ImGui::MenuItem("Go", nullptr, state.language == "go"))
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

        // Open file popup
        if (ImGui::BeginPopupModal("OpenFilePopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter file path:");
            ImGui::InputText("##path", openPathBuf, sizeof(openPathBuf));
            if (ImGui::Button("Open", ImVec2(120, 0))) {
                state.doOpen(openPathBuf);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        // ---------------------------------------------------------------
        //  File Explorer (left panel)
        // ---------------------------------------------------------------
        ImGui::Begin("Explorer");
        ImGui::PushFont(uiFont);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "OPEN EDITORS");
        ImGui::Separator();
        // Show current file as a selectable
        {
            std::string label = state.filePath;
            if (state.modified) label += " *";
            ImGui::Selectable(label.c_str(), true);
        }
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "FILES");
        ImGui::Separator();
        // Placeholder files
        const char* files[] = {"Calculator.py", "ConditionalExample.py", "main.cpp", "test.el"};
        for (auto f : files) {
            if (ImGui::Selectable(f)) {
                state.doOpen(f);
            }
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
            std::string tabLabel = state.filePath;
            if (state.modified) tabLabel += " *";
            if (ImGui::BeginTabItem(tabLabel.c_str())) {
                // Editable text area fills available space
                ImVec2 avail = ImGui::GetContentRegionAvail();
                avail.y -= 4; // small margin

                state.updateHighlights();
                CodeEditorOptions opts;
                opts.showWhitespace = state.showWhitespace;
                opts.mode = &state.mode;
                opts.enableFolding = true;
                opts.showMinimap = state.showMinimap;

                CodeEditorResult res = state.codeWidget.render("##editor",
                    state.editBuf, state.highlights, opts, avail, monoFont);
                state.updateCursorPos(res.cursorByte);
                if (res.changed) {
                    state.onTextChanged();
                }

                ImGui::EndTabItem();
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
                Module* ast = state.sync.getAST();
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
                RenderHighlightedText(state.editBuf, state.highlights);
                ImGui::EndChild();
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Generated code preview
            if (ImGui::BeginTabItem("Generated")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##genScroll", ImVec2(0, 0), false);
                Module* ast = state.sync.getAST();
                if (ast) {
                    std::string generated;
                    if (state.language == "python") {
                        PythonGenerator gen;
                        generated = gen.generate(ast);
                    } else if (state.language == "cpp") {
                        CppGenerator gen;
                        generated = gen.generate(ast);
                    } else if (state.language == "elisp") {
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
            ImGui::Text("Ln %d, Col %d", state.cursorLine, state.cursorCol);
            ImGui::SameLine(0, 30);

            // Language
            ImGui::Text("%s", state.language.c_str());
            ImGui::SameLine(0, 30);

            // Keybinding profile
            ImGui::Text("Keys: %s", KeybindingManager::profileName(state.keys.getProfile()));
            ImGui::SameLine(0, 30);

            // Modified indicator
            if (state.modified)
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
