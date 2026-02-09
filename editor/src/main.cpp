// Whetstone Editor — ImGui-based structured code editor
//
// VSCode/JetBrains-inspired layout with docking:
//   - File tree (left)
//   - Editable text area (center) backed by TextEditor + TextASTSync
//   - Syntax-highlighted preview / AST view (bottom)
//   - Toolbar + status bar
//   - Configurable keybinding profiles (VSCode default)

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "EditorState.h"
#include "EditorUtils.h"

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
    const float baseFontSize = 15.0f;
    ImFont* monoFont = nullptr;
    monoFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", baseFontSize);
    if (!monoFont) {
        monoFont = io.Fonts->AddFontDefault();
    }
    // Also keep default font for UI elements
    ImFont* uiFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", baseFontSize);
    if (!uiFont) {
        uiFont = io.Fonts->AddFontDefault();
    }

    SetupVSCodeDarkTheme();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Editor state
    EditorState state;
    state.init();
    if (state.settings.getTheme() == "Light") {
        SetupVSCodeLightTheme();
    } else {
        SetupVSCodeDarkTheme();
    }
    io.FontGlobalScale = state.settings.getFontSize() / baseFontSize;
    SessionData session;
    if (state.loadSession(session)) {
        if (!session.imguiIni.empty()) {
            ImGui::LoadIniSettingsFromMemory(session.imguiIni.c_str(),
                                             session.imguiIni.size());
        }
        state.applySession(session);
    }
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
                                state.projectSearch.setRoot(state.workspaceRoot);
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

                if ((sdlMod & KMOD_CTRL) && (sdlMod & KMOD_SHIFT) && sym == SDLK_p) {
                    state.showCommandPalette = true;
                    state.commandQuery[0] = '\0';
                    state.commandSelected = 0;
                }

                auto applyFontSize = [&](int newSize) {
                    newSize = clampFontSize(newSize);
                    state.settings.setFontSize(newSize);
                    io.FontGlobalScale = newSize / baseFontSize;
                    state.saveSettingsToDisk();
                };

                if ((sdlMod & KMOD_CTRL) && sym == SDLK_0) {
                    applyFontSize((int)baseFontSize);
                }

                if (key != 0 && mods != WMOD_NONE) {
                    KeyCombo combo{key, mods};
                    std::string action = state.keys.getAction(combo);
                    if (action == "edit.undo")       state.doUndo();
                    else if (action == "edit.redo")   state.doRedo();
                    else if (action == "search.find") state.showFind = !state.showFind;
                    else if (action == "search.findInFiles") state.showProjectSearch = !state.showProjectSearch;
                    else if (action == "nav.goToLine") {
                        state.showGoToLine = true;
                        state.goToLineBuf[0] = '\0';
                        state.goToLineError = false;
                    }
                    else if (action == "view.toggleTerminal") {
                        state.showTerminalPanel = !state.showTerminalPanel;
                    }
                    else if (action == "file.save")   state.doSave();
                    else if (action == "file.new") {
                        std::string lang = state.active() ? state.active()->language : "python";
                        state.createBuffer(state.makeUntitledName(), "", lang);
                    }
                    else if (action == "build.run")    state.runActiveFile(false);
                    else if (action == "build.build")  state.runActiveFile(true);
                    else if (action == "view.zoomIn")  applyFontSize(state.settings.getFontSize() + 1);
                    else if (action == "view.zoomOut") applyFontSize(state.settings.getFontSize() - 1);
                }
            }
        }

        // File watcher polling
        state.handleFileChanges();
        if (state.settings.getAutoSaveSeconds() > 0 && state.active() && state.active()->modified) {
            double now = ImGui::GetTime();
            if ((now - state.lastAutoSave) >= state.settings.getAutoSaveSeconds()) {
                state.doSave();
                state.lastAutoSave = now;
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
                if (ImGui::MenuItem("Open Project...")) {
                    auto path = FileDialog::openFile({"Open Project", {"*.whetstone"}, lastDialogPath});
                    if (!path.empty()) {
                        if (!state.loadProject(path)) {
                            state.outputLog += "Failed to open project: " + path + "\n";
                        } else {
                            lastDialogPath = path;
                        }
                    }
                }
                if (ImGui::MenuItem("Open Folder...")) {
                    auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
                    if (!path.empty()) {
                        state.workspaceRoot = path;
                        state.fileTreeDirty = true;
                        state.projectSearch.setRoot(state.workspaceRoot);
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
                if (ImGui::MenuItem("Save Project...")) {
                    auto path = FileDialog::saveFile({"Save Project", {"*.whetstone"}, lastDialogPath});
                    if (!path.empty()) {
                        if (!state.saveProject(path)) {
                            state.outputLog += "Failed to save project: " + path + "\n";
                        } else {
                            lastDialogPath = path;
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) done = true;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", state.keys.getBinding("edit.undo").toString().c_str(),
                                    false, state.active() ? state.active()->orchestrator.getUndoDepth() > 0 : false))
                    state.doUndo();
                if (ImGui::MenuItem("Redo", state.keys.getBinding("edit.redo").toString().c_str(),
                                    false, state.active() ? state.active()->orchestrator.getRedoDepth() > 0 : false))
                    state.doRedo();
                ImGui::Separator();
                if (ImGui::MenuItem("Find/Replace", state.keys.getBinding("search.find").toString().c_str()))
                    state.showFind = !state.showFind;
                if (ImGui::MenuItem("Find in Files", state.keys.getBinding("search.findInFiles").toString().c_str()))
                    state.showProjectSearch = !state.showProjectSearch;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Whitespace", nullptr, &state.showWhitespace);
                ImGui::MenuItem("Show Minimap", nullptr, &state.showMinimap);
                ImGui::MenuItem("Show Annotations", nullptr, &state.showAnnotations);
                ImGui::MenuItem("Show Outline", nullptr, &state.showOutline);
                ImGui::MenuItem("Terminal", state.keys.getBinding("view.toggleTerminal").toString().c_str(),
                                &state.showTerminalPanel);
                ImGui::MenuItem("Settings", nullptr, &state.showSettingsPanel);
                ImGui::MenuItem("LSP Servers...", nullptr, &state.showLspSettings);
                if (state.active()) {
                    bool textMode = state.active()->bufferMode == BufferManager::BufferMode::Text;
                    if (ImGui::MenuItem("Text-Editor Mode", nullptr, textMode)) {
                        state.active()->bufferMode = textMode ?
                            BufferManager::BufferMode::Structured :
                            BufferManager::BufferMode::Text;
                        state.buffers.setBufferMode(state.active()->path, state.active()->bufferMode);
                        if (state.active()->bufferMode == BufferManager::BufferMode::Text) {
                            state.suggestions.clear();
                            state.whetstoneDiagnostics.clear();
                            state.analysisPending = false;
                        } else {
                            state.onTextChanged();
                        }
                        if (state.active()->path.rfind("(untitled", 0) != 0) {
                            state.welcome.addRecentFile(state.active()->path,
                                                        state.active()->language,
                                                        bufferModeToString(state.active()->bufferMode));
                            state.saveRecentFiles();
                        }
                    }
                }
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
            if (ImGui::BeginMenu("Navigate")) {
                if (ImGui::MenuItem("Go to Line...", state.keys.getBinding("nav.goToLine").toString().c_str())) {
                    state.showGoToLine = true;
                    state.goToLineBuf[0] = '\0';
                    state.goToLineError = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Build")) {
                if (ImGui::MenuItem("Run", state.keys.getBinding("build.run").toString().c_str())) {
                    state.runActiveFile(false);
                }
                if (ImGui::MenuItem("Build", state.keys.getBinding("build.build").toString().c_str())) {
                    state.runActiveFile(true);
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
            if (ImGui::BeginMenu("Refactor")) {
                bool canRefactor = state.isStructured() && state.active() && !state.active()->readOnly;
                if (!canRefactor) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Rename Variable")) {
                    state.refactorAction = 1;
                    state.showRefactorPopup = true;
                    state.refactorNameA[0] = '\0';
                    state.refactorNameB[0] = '\0';
                    ImGui::OpenPopup("RefactorPopup");
                }
                if (ImGui::MenuItem("Extract Function")) {
                    state.refactorAction = 2;
                    state.showRefactorPopup = true;
                    state.refactorNameA[0] = '\0';
                    state.refactorNameB[0] = '\0';
                    ImGui::OpenPopup("RefactorPopup");
                }
                if (ImGui::MenuItem("Inline Variable")) {
                    state.refactorAction = 3;
                    state.showRefactorPopup = true;
                    state.refactorNameA[0] = '\0';
                    state.refactorNameB[0] = '\0';
                    ImGui::OpenPopup("RefactorPopup");
                }
                if (!canRefactor) ImGui::EndDisabled();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Keybindings")) {
                for (auto p : KeybindingManager::availableProfiles()) {
                    if (ImGui::MenuItem(KeybindingManager::profileName(p), nullptr,
                                        state.keys.getProfile() == p)) {
                        state.keys.setProfile(p);
                        state.registerCommands();
                    }
                }
                ImGui::EndMenu();
            }

            const bool canProject = state.activeAST() != nullptr;
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(12.0f, 0.0f));
            ImGui::SameLine();
            if (!canProject) ImGui::BeginDisabled();
            if (ImGui::Button("Project to...")) {
                ImGui::OpenPopup("ProjectToPopup");
            }
            if (ImGui::BeginPopup("ProjectToPopup")) {
                if (ImGui::MenuItem("Python")) state.projectToLanguage("python");
                if (ImGui::MenuItem("C++")) state.projectToLanguage("cpp");
                if (ImGui::MenuItem("Elisp")) state.projectToLanguage("elisp");
                ImGui::EndPopup();
            }
            if (!canProject) ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::Dummy(ImVec2(12.0f, 0.0f));
            ImGui::SameLine();
            bool canRunFile = state.active() && !state.active()->readOnly;
            if (!canRunFile) ImGui::BeginDisabled();
            if (ImGui::Button("Run >")) {
                state.runActiveFile(false);
            }
            ImGui::SameLine();
            if (ImGui::Button("Build")) {
                state.runActiveFile(true);
            }
            if (!canRunFile) ImGui::EndDisabled();
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
        //  Symbol Outline
        // ---------------------------------------------------------------
        if (state.showOutline) {
            ImGui::Begin("Outline", &state.showOutline);
            ImGui::PushFont(uiFont);
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::InputText("Filter", state.outlineFilter, sizeof(state.outlineFilter));
            ImGui::Separator();

            std::string filter = toLowerCopy(state.outlineFilter);
            if (!state.active()) {
                ImGui::TextDisabled("(no file)");
            } else {
                std::vector<LSPClient::DocumentSymbol> lspSymbols =
                    state.lsp ? state.lsp->getDocumentSymbols() : std::vector<LSPClient::DocumentSymbol>{};

                if (!lspSymbols.empty()) {
                    int outlineId = 0;
                    std::function<void(const LSPClient::DocumentSymbol&)> renderLsp;
                    renderLsp = [&](const LSPClient::DocumentSymbol& sym) {
                        if (!lspSymbolMatchesFilter(sym, filter)) return;
                        ImGui::PushID(outlineId++);
                        std::string label = std::string(lspSymbolKindLabel(sym.kind)) + ": " + sym.name;
                        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
                        if (sym.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                        bool open = ImGui::TreeNodeEx(label.c_str(), flags);
                        if (ImGui::IsItemClicked()) {
                            state.jumpTo(state.active(), sym.selectionRange.start.line,
                                         sym.selectionRange.start.character);
                        }
                        if (!sym.children.empty() && open) {
                            for (const auto& child : sym.children) {
                                renderLsp(child);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    };
                    for (const auto& sym : lspSymbols) renderLsp(sym);
                } else if (state.isStructured()) {
                    std::vector<OutlineItem> outline;
                    collectOutlineFromAST(state.activeAST(), outline);
                    if (outline.empty()) {
                        ImGui::TextDisabled("(no symbols)");
                    } else {
                        int outlineId = 0;
                        std::function<void(const OutlineItem&)> renderItem;
                        renderItem = [&](const OutlineItem& item) {
                            if (!outlineMatchesFilter(item, filter)) return;
                            ImGui::PushID(outlineId++);
                            std::string label = item.kind + ": " + item.name;
                            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
                            if (item.children.empty()) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            bool open = ImGui::TreeNodeEx(label.c_str(), flags);
                            if (ImGui::IsItemClicked() && item.line >= 0) {
                                state.jumpTo(state.active(), item.line, item.col);
                            }
                            if (!item.children.empty() && open) {
                                for (const auto& child : item.children) {
                                    renderItem(child);
                                }
                                ImGui::TreePop();
                            }
                            ImGui::PopID();
                        };
                        for (const auto& item : outline) renderItem(item);
                    }
                } else {
                    ImGui::TextDisabled("(no symbols)");
                }
            }
            ImGui::PopFont();
            ImGui::End();
        }

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
        //  Project Search (Ctrl+Shift+F)
        // ---------------------------------------------------------------
        if (state.showProjectSearch) {
            ImGui::Begin("Search", &state.showProjectSearch);
            ImGui::PushFont(uiFont);
            bool doSearch = false;
            ImGui::SetNextItemWidth(420);
            if (ImGui::InputText("Query", state.searchQuery, sizeof(state.searchQuery),
                                 ImGuiInputTextFlags_EnterReturnsTrue)) {
                doSearch = true;
            }
            ImGui::SetNextItemWidth(420);
            ImGui::InputText("Include (glob)", state.searchInclude, sizeof(state.searchInclude));
            ImGui::SetNextItemWidth(420);
            ImGui::InputText("Exclude (glob)", state.searchExclude, sizeof(state.searchExclude));
            ImGui::Checkbox("Regex", &state.searchUseRegex);
            ImGui::SameLine();
            if (ImGui::Button("Search")) doSearch = true;
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                state.searchQuery[0] = '\0';
                state.searchResults.clear();
            }

            if (doSearch) {
                state.searchResults = state.projectSearch.search(
                    state.searchQuery,
                    state.searchInclude,
                    state.searchExclude,
                    state.searchUseRegex);
            }

            ImGui::Separator();
            ImGui::BeginChild("##searchResults", ImVec2(0, 0), false);
            if (state.searchResults.empty()) {
                ImGui::TextDisabled("(no results)");
            } else {
                for (const auto& fileRes : state.searchResults) {
                    std::string label = fileRes.path + " (" + std::to_string(fileRes.matches.size()) + ")";
                    if (ImGui::TreeNode(label.c_str())) {
                        for (const auto& match : fileRes.matches) {
                            std::string lineLabel = std::to_string(match.line + 1) + ":" +
                                std::to_string(match.col + 1) + "  " + match.lineText;
                            if (ImGui::Selectable(lineLabel.c_str())) {
                                if (state.buffers.hasBuffer(fileRes.path)) state.switchToBuffer(fileRes.path);
                                else state.doOpen(fileRes.path);
                                state.jumpTo(state.active(), match.line, match.col);
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::PopFont();
            ImGui::End();
        }

        // ---------------------------------------------------------------
        //  Settings Panel
        // ---------------------------------------------------------------
        if (state.showSettingsPanel) {
            ImGui::Begin("Settings", &state.showSettingsPanel);
            ImGui::PushFont(uiFont);
            bool settingsChanged = false;
            ImGuiIO& io = ImGui::GetIO();

            int fontSize = state.settings.getFontSize();
            if (ImGui::SliderInt("Font Size", &fontSize, 12, 24)) {
                state.settings.setFontSize(fontSize);
                io.FontGlobalScale = fontSize / baseFontSize;
                settingsChanged = true;
            }

            int tabSize = state.settings.getTabSize();
            const int tabSizes[] = {2, 4, 8};
            int tabIndex = 1;
            for (int i = 0; i < 3; ++i) {
                if (tabSize == tabSizes[i]) tabIndex = i;
            }
            const char* tabLabels[] = {"2", "4", "8"};
            if (ImGui::Combo("Tab Size", &tabIndex, tabLabels, 3)) {
                state.settings.setTabSize(tabSizes[tabIndex]);
                state.applyTabSizeToBuffers(tabSizes[tabIndex]);
                settingsChanged = true;
            }

            const char* themeLabels[] = {"Dark", "Light"};
            int themeIndex = state.settings.getTheme() == "Light" ? 1 : 0;
            if (ImGui::Combo("Theme", &themeIndex, themeLabels, 2)) {
                state.settings.setTheme(themeLabels[themeIndex]);
                if (themeIndex == 1) SetupVSCodeLightTheme();
                else SetupVSCodeDarkTheme();
                settingsChanged = true;
            }

            int autoSave = state.settings.getAutoSaveSeconds();
            if (ImGui::InputInt("Auto-save (sec)", &autoSave)) {
                autoSave = std::max(0, autoSave);
                state.settings.setAutoSaveSeconds(autoSave);
                settingsChanged = true;
            }

            bool showMinimap = state.showMinimap;
            if (ImGui::Checkbox("Show Minimap", &showMinimap)) {
                state.showMinimap = showMinimap;
                settingsChanged = true;
            }
            bool showLineNumbers = state.showLineNumbers;
            if (ImGui::Checkbox("Show Line Numbers", &showLineNumbers)) {
                state.showLineNumbers = showLineNumbers;
                settingsChanged = true;
            }

            LayoutPreset preset = state.layoutPreset;
            int presetIndex = 0;
            if (preset == LayoutPreset::Emacs) presetIndex = 1;
            else if (preset == LayoutPreset::JetBrains) presetIndex = 2;
            const char* presetLabels[] = {"VSCode", "Emacs", "JetBrains"};
            if (ImGui::Combo("Layout Preset", &presetIndex, presetLabels, 3)) {
                state.layoutPreset = (presetIndex == 1) ? LayoutPreset::Emacs :
                                    (presetIndex == 2) ? LayoutPreset::JetBrains : LayoutPreset::VSCode;
                settingsChanged = true;
            }

            int keyProfileIndex = 0;
            if (state.keys.getProfile() == KeybindingProfile::JetBrains) keyProfileIndex = 1;
            else if (state.keys.getProfile() == KeybindingProfile::Emacs) keyProfileIndex = 2;
            const char* keyProfiles[] = {"VSCode", "JetBrains", "Emacs"};
            if (ImGui::Combo("Keybindings", &keyProfileIndex, keyProfiles, 3)) {
                KeybindingProfile profile = KeybindingProfile::VSCode;
                if (keyProfileIndex == 1) profile = KeybindingProfile::JetBrains;
                else if (keyProfileIndex == 2) profile = KeybindingProfile::Emacs;
                state.keys.setProfile(profile);
                state.registerCommands();
                settingsChanged = true;
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Emacs Config");
            ImGui::SetNextItemWidth(320);
            if (InputTextStr("Config Path", &state.settings.getEmacsConfigPathMutable())) {
                settingsChanged = true;
            }

            if (ImGui::CollapsingHeader("LSP Servers", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto& cfg : state.settings.getLSPServersMutable()) {
                    ImGui::PushID(cfg.language.c_str());
                    ImGui::Checkbox("Enabled", &cfg.enabled);
                    ImGui::SameLine();
                    ImGui::Text("%s", cfg.language.c_str());
                    ImGui::SetNextItemWidth(320);
                    if (InputTextStr("Path", &cfg.path)) settingsChanged = true;
                    ImGui::SetNextItemWidth(320);
                    if (InputTextStr("Args", &cfg.argsLine)) {
                        state.settings.syncArgs(cfg);
                        settingsChanged = true;
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }
            }

            if (settingsChanged) {
                state.saveSettingsToDisk();
            }
            ImGui::PopFont();
            ImGui::End();
        }

        // ---------------------------------------------------------------
        //  Go To Line (Ctrl+G)
        // ---------------------------------------------------------------
        if (state.showGoToLine) {
            ImGui::OpenPopup("GoToLine");
        }
        if (ImGui::BeginPopupModal("GoToLine", &state.showGoToLine, ImGuiWindowFlags_AlwaysAutoResize)) {
            int totalLines = state.active() ? countLines(state.active()->editBuf) : 0;
            ImGui::Text("Enter line or :line:col");
            ImGui::TextDisabled("Total lines: %d", totalLines);
            ImGui::SetNextItemWidth(240);
            bool submit = ImGui::InputText("##gotoLineInput", state.goToLineBuf,
                                           sizeof(state.goToLineBuf),
                                           ImGuiInputTextFlags_EnterReturnsTrue);
            if (ImGui::Button("Go")) submit = true;
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                state.showGoToLine = false;
                state.goToLineError = false;
                ImGui::CloseCurrentPopup();
            }
            if (submit) {
                int line = 0;
                int col = 0;
                if (parseLineColInput(state.goToLineBuf, line, col)) {
                    if (totalLines > 0) line = std::max(1, std::min(line, totalLines));
                    col = std::max(1, col);
                    if (state.active()) {
                        state.jumpTo(state.active(), line - 1, col - 1);
                    }
                    state.showGoToLine = false;
                    state.goToLineError = false;
                    ImGui::CloseCurrentPopup();
                } else {
                    state.goToLineError = true;
                }
            }
            if (state.goToLineError) {
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f), "Invalid format.");
            }
            ImGui::EndPopup();
        }

        // ---------------------------------------------------------------
        //  LSP Server Settings
        // ---------------------------------------------------------------
        if (state.showLspSettings) {
            ImGui::Begin("LSP Servers", &state.showLspSettings);
            ImGui::PushFont(uiFont);
            ImGui::TextUnformatted("Emacs Config");
            ImGui::SetNextItemWidth(320);
            InputTextStr("Config Path", &state.settings.getEmacsConfigPathMutable(), ImGuiInputTextFlags_None);
            ImGui::Separator();
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
        //  Refactor popup
        // ---------------------------------------------------------------
        if (state.showRefactorPopup) {
            ImGui::OpenPopup("RefactorPopup");
        }
        if (ImGui::BeginPopupModal("RefactorPopup", &state.showRefactorPopup,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            const char* title = "";
            if (state.refactorAction == 1) title = "Rename Variable";
            else if (state.refactorAction == 2) title = "Extract Function";
            else if (state.refactorAction == 3) title = "Inline Variable";
            ImGui::TextUnformatted(title);
            ImGui::Separator();

            if (state.refactorAction == 1) {
                ImGui::InputText("Old Name", state.refactorNameA, sizeof(state.refactorNameA));
                ImGui::InputText("New Name", state.refactorNameB, sizeof(state.refactorNameB));
            } else if (state.refactorAction == 2) {
                ImGui::InputText("New Function Name", state.refactorNameA, sizeof(state.refactorNameA));
                ImGui::TextDisabled("Extracts the first statement of the first function.");
            } else if (state.refactorAction == 3) {
                ImGui::InputText("Variable Name", state.refactorNameA, sizeof(state.refactorNameA));
                ImGui::TextDisabled("Removes the first matching variable declaration.");
            }

            if (!state.refactorError.empty()) {
                ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.0f),
                                   "%s", state.refactorError.c_str());
            }

            bool canRun = state.isStructured() && state.active() && !state.active()->readOnly;
            if (!canRun) ImGui::BeginDisabled();
            if (ImGui::Button("Preview")) {
                state.refactorError.clear();
                Module* ast = state.activeAST();
                RefactorPlan plan;
                if (state.refactorAction == 1) {
                    plan = buildRenameVariablePlan(ast, state.refactorNameA, state.refactorNameB);
                } else if (state.refactorAction == 2) {
                    plan = buildExtractFunctionPlan(ast, state.refactorNameA);
                } else if (state.refactorAction == 3) {
                    plan = buildInlineVariablePlan(ast, state.refactorNameA);
                }
                if (!plan.success) {
                    state.refactorError = plan.error;
                } else {
                    auto previewAst = cloneModule(ast);
                    BatchMutationAPI batch;
                    batch.setRoot(previewAst.get());
                    auto res = batch.applySequence(plan.mutations);
                    if (!res.success) {
                        state.refactorError = res.error;
                    } else {
                        std::string beforeText = state.active()->editBuf;
                        std::string afterText =
                            generateForLanguage(previewAst.get(), state.active()->language);
                        state.openDiff(beforeText, afterText, true, 0, {}, plan.mutations);
                        state.showRefactorPopup = false;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply")) {
                state.refactorError.clear();
                Module* ast = state.mutationAST();
                RefactorPlan plan;
                if (state.refactorAction == 1) {
                    plan = buildRenameVariablePlan(ast, state.refactorNameA, state.refactorNameB);
                } else if (state.refactorAction == 2) {
                    plan = buildExtractFunctionPlan(ast, state.refactorNameA);
                } else if (state.refactorAction == 3) {
                    plan = buildInlineVariablePlan(ast, state.refactorNameA);
                }
                if (!plan.success) {
                    state.refactorError = plan.error;
                } else {
                    BatchMutationAPI batch;
                    batch.setRoot(ast);
                    auto res = batch.applySequence(plan.mutations);
                    if (!res.success) {
                        state.refactorError = res.error;
                    } else {
                        state.outputLog += "Refactor applied (" +
                                           std::to_string(res.appliedCount) + " mutations).\n";
                        state.applyOrchestratorToActive();
                        state.showRefactorPopup = false;
                    }
                }
            }
            if (!canRun) ImGui::EndDisabled();
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                state.showRefactorPopup = false;
            }

            ImGui::EndPopup();
        }

        // ---------------------------------------------------------------
        //  Command Palette
        // ---------------------------------------------------------------
        if (state.showCommandPalette) {
            ImGui::Begin("Command Palette", &state.showCommandPalette,
                         ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::SetNextItemWidth(420.0f);
            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
            }
            if (ImGui::InputText("##cmdQuery", state.commandQuery, sizeof(state.commandQuery))) {
                state.commandSelected = 0;
            }

            auto results = state.commandPalette.search(state.commandQuery);
            int maxIndex = std::max(0, (int)results.size() - 1);
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                state.commandSelected = std::min(state.commandSelected + 1, maxIndex);
            }
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                state.commandSelected = std::max(state.commandSelected - 1, 0);
            }
            bool accept = ImGui::IsKeyPressed(ImGuiKey_Enter);
            bool dismiss = ImGui::IsKeyPressed(ImGuiKey_Escape);

            if (dismiss) {
                state.showCommandPalette = false;
            }

            for (int i = 0; i < (int)results.size(); ++i) {
                const auto& cmd = results[i];
                bool selected = (i == state.commandSelected);
                std::string label = cmd.label;
                if (!cmd.shortcut.empty()) label += "  [" + cmd.shortcut + "]";
                if (ImGui::Selectable(label.c_str(), selected)) {
                    state.commandSelected = i;
                    accept = true;
                }
            }
            if (accept && !results.empty()) {
                state.executeCommand(results[state.commandSelected].id);
                state.showCommandPalette = false;
            }
            ImGui::End();
        }

        // ---------------------------------------------------------------
        //  Editor (center) — editable text area
        // ---------------------------------------------------------------
        ImGui::Begin("Editor");
        ImGui::PushFont(uiFont);
        ImGui::BeginChild("##breadcrumbs", ImVec2(0, 26.0f), false, ImGuiWindowFlags_NoScrollbar);
        if (!state.active()) {
            ImGui::TextDisabled("(no file)");
        } else if (!state.isStructured()) {
            ImGui::TextDisabled("Text mode");
        } else {
            Module* ast = state.activeAST();
            int lineZero = std::max(0, state.active()->cursorLine - 1);
            int colZero = std::max(0, state.active()->cursorCol - 1);
            ASTNode* node = ast ? findNodeAtPosition(ast, lineZero, colZero) : nullptr;
            if (!node) {
                ImGui::TextDisabled("(no scope)");
            } else {
                auto crumbs = buildBreadcrumbTrail(node);
                for (size_t i = 0; i < crumbs.size(); ++i) {
                    if (i > 0) {
                        ImGui::SameLine();
                        ImGui::TextUnformatted(">");
                        ImGui::SameLine();
                    }
                    ImGui::PushID((int)i);
                    const auto& item = crumbs[i];
                    if (item.isRole) {
                        ImGui::TextDisabled("%s", item.label.c_str());
                    } else if (item.node && item.node->hasSpan()) {
                        if (ImGui::SmallButton(item.label.c_str())) {
                            state.jumpTo(state.active(), item.node->spanStartLine, item.node->spanStartCol);
                        }
                    } else {
                        ImGui::TextUnformatted(item.label.c_str());
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndChild();
        ImGui::PopFont();

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
                        if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                            state.updateGenerated();
                        }
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
                        if (state.isStructured()) {
                            Module* ast = state.activeAST();
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
                        opts.readOnly = buf->readOnly;
                        opts.mode = &buf->mode;
                        opts.enableFolding = true;
                        opts.showMinimap = state.showMinimap;
                        opts.showAnnotations = state.showAnnotations;
                        opts.showLineNumbers = state.showLineNumbers;
                        if (state.layoutPreset == LayoutPreset::Emacs) opts.annotationLayout = 1;
                        else if (state.layoutPreset == LayoutPreset::JetBrains) opts.annotationLayout = 2;
                        else opts.annotationLayout = 0;
                        opts.errorLines = &errorLines;
                        opts.warningLines = &warningLines;
                        opts.diagnostics = &diagRanges;
                        opts.annotations = &annoMarkers;
                        opts.suggestions = &suggestionMarkers;
                        opts.conflicts = &conflictMarkers;

                        CodeEditorResult res;
                        if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                            opts.syncScrollX = &buf->splitScrollX;
                            opts.syncScrollY = &buf->splitScrollY;
                            opts.scrollMaster = true;

                            ImGui::BeginTable("##editorSplit", 2,
                                              ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
                            ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthStretch, 0.55f);
                            ImGui::TableSetupColumn("Generated", ImGuiTableColumnFlags_WidthStretch, 0.45f);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            res = buf->widget.render("##editor",
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
                            genOpts.showLineNumbers = state.showLineNumbers;
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
                        } else {
                            res = buf->widget.render("##editor",
                                buf->editBuf, buf->highlights, opts, avail, monoFont);
                        }

                        state.updateCursorPos(res.cursorByte);
                        if (res.ctrlClick && state.active()) {
                            state.goToDefinitionAt(res.ctrlClickLine, res.ctrlClickCol);
                        }
                        if (res.lineClicked && buf->bufferMode == BufferManager::BufferMode::Structured) {
                            Module* ast = state.activeAST();
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
                            if (buf->bufferMode == BufferManager::BufferMode::Structured) {
                                state.analysisPending = true;
                                state.analysisLastChange = ImGui::GetTime();
                            }
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

                        if (state.symbolsPending && (now - state.symbolsLastChange) > 0.3) {
                            if (state.lsp && state.active() && state.active()->path.rfind("(untitled", 0) != 0) {
                                state.lsp->requestDocumentSymbols(EditorState::toFileUri(state.active()->path));
                            }
                            state.symbolsPending = false;
                        }

                        if (state.analysisPending && (now - state.analysisLastChange) > 0.5) {
                            if (state.isStructured()) {
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
                                Module* ast = state.activeAST();
                                if (ast) {
                                    MemoryStrategyInference inf;
                                    state.suggestions = inf.inferAnnotations(ast);
                                } else {
                                    state.suggestions.clear();
                                }
                            } else {
                                state.whetstoneDiagnostics.clear();
                                state.suggestions.clear();
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
                                    Module* ast = state.mutationAST();
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
                                                state.applyOrchestratorToActive();
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

                            if (state.definitionPending) {
                                auto defs = state.lsp->getDefinitionLocations();
                                if (!defs.empty()) {
                                    state.definitionPending = false;
                                    state.definitionPreview = defs.front();
                                    state.definitionPreviewValid = true;
                                    state.lsp->clearDefinitionLocations();
                                    state.jumpToDefinitionLocation(state.definitionPreview);
                                } else if ((ImGui::GetTime() - state.definitionLastRequest) > 0.5) {
                                    state.definitionPending = false;
                                    state.goToDefinitionFallback(state.definitionRequestLine,
                                                                 state.definitionRequestCol);
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

                        if (res.hoverValid && (ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeySuper)) {
                            std::string preview;
                            if (state.previewDefinitionAt(res.hoverLine, res.hoverCol, preview)) {
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(preview.c_str());
                                ImGui::EndTooltip();
                            } else if (state.definitionPreviewValid) {
                                std::string path = EditorState::fromFileUri(state.definitionPreview.uri);
                                std::string label = path + ":" +
                                    std::to_string(state.definitionPreview.range.start.line + 1);
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(label.c_str());
                                ImGui::EndTooltip();
                            }
                        }

                        if (ImGui::BeginPopupContextWindow("AnnotationContext", ImGuiPopupFlags_MouseButtonRight)) {
                            Module* ast = state.mutationAST();
                            int lineZero = state.active() ? std::max(0, state.active()->cursorLine - 1) : 0;
                            ASTNode* target = ast ? findAnnotationTarget(ast, lineZero) : nullptr;
                            if (!target) {
                                ImGui::TextDisabled("No annotatable symbol on this line.");
                            } else {
                                bool mutated = false;
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
                                            if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
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
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Tracing");
                                                    if (!res.error.empty()) state.outputLog += res.error + "\n";
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "OwnerAnnotation") {
                                                if (ImGui::MenuItem("Single")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Single");
                                                    if (!res.error.empty()) state.outputLog += res.error + "\n";
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "DeallocateAnnotation") {
                                                if (ImGui::MenuItem("Explicit")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Explicit");
                                                    if (!res.error.empty()) state.outputLog += res.error + "\n";
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "LifetimeAnnotation") {
                                                if (ImGui::MenuItem("RAII")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "RAII");
                                                    if (!res.error.empty()) state.outputLog += res.error + "\n";
                                                    if (res.error.empty()) mutated = true;
                                                }
                                            } else if (anno->conceptType == "AllocateAnnotation") {
                                                if (ImGui::MenuItem("Static")) {
                                                    state.mutator.setRoot(ast);
                                                    auto res = state.mutator.setProperty(anno->id, "strategy", "Static");
                                                    if (!res.error.empty()) state.outputLog += res.error + "\n";
                                                    if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
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
                                            if (res.error.empty()) mutated = true;
                                        }
                                        if (ImGui::MenuItem("Match parent strategy")) {
                                            state.mutator.setRoot(ast);
                                            auto res = state.mutator.setProperty(lineConflict.childAnnoId,
                                                                                "strategy",
                                                                                lineConflict.parentStrategy);
                                            if (!res.error.empty()) state.outputLog += res.error + "\n";
                                            if (res.error.empty()) mutated = true;
                                        }
                                    }
                                    ImGui::EndMenu();
                                }
                                if (mutated) {
                                    state.applyOrchestratorToActive();
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
        //  Bottom panel — Output / AST / Highlighted Preview / Terminal
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

            if (state.showTerminalPanel && ImGui::BeginTabItem("Terminal")) {
                state.terminal.render(state.workspaceRoot, monoFont);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Agents")) {
                ImGui::PushFont(monoFont);
                bool running = state.agentServer && state.agentServer->isRunning();
                ImGui::Text("Server: %s", running ? "Running" : "Stopped");
                ImGui::SameLine(0, 20);
                ImGui::Text("Port: %d", state.agentPort);
                ImGui::Separator();

                ImGui::TextUnformatted("Active Sessions");
                if (!state.agentServer || state.agentServer->getActiveSessionCount() == 0) {
                    ImGui::TextDisabled("(none)");
                } else {
                    for (const auto& s : state.agentServer->getActiveSessions()) {
                        std::string label = s.sessionId;
                        if (!s.agentName.empty()) label += " (" + s.agentName + ")";
                        label += " - msgs: " + std::to_string(s.messageCount);
                        ImGui::BulletText("%s", label.c_str());
                    }
                }

                ImGui::Separator();
                ImGui::TextUnformatted("Activity Log");
                ImGui::BeginChild("##agentLog", ImVec2(0, 0), false);
                for (const auto& line : state.agentLog) {
                    ImGui::TextUnformatted(line.c_str());
                }
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

            // Optimization controls
            if (ImGui::BeginTabItem("Optimize")) {
                ImGui::PushFont(uiFont);
                Module* ast = state.mutationAST();
                if (!state.active()) {
                    ImGui::TextDisabled("(no active buffer)");
                } else if (state.active()->bufferMode == BufferManager::BufferMode::Text) {
                    ImGui::TextDisabled("(disabled in Text mode)");
                } else if (!ast) {
                    ImGui::TextDisabled("(no AST)");
                } else if (state.active()->readOnly) {
                    ImGui::TextDisabled("(read-only buffer)");
                } else {
                    ImGui::Checkbox("Preview changes", &state.optimizePreview);
                    std::string blockReason = findOptimizationBlockReason(ast);
                    bool blocked = !blockReason.empty();

                    auto showBlockedTooltip = [&](const std::string& reason) {
                        if (!reason.empty() &&
                            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(reason.c_str());
                            ImGui::EndTooltip();
                        }
                    };

                    if (blocked) ImGui::BeginDisabled();
                    if (ImGui::Button("Constant Fold")) {
                        std::string beforeText = state.active()->editBuf;
                        if (state.optimizePreview) {
                            auto previewAst = cloneModule(ast);
                            TransformEngine engine;
                            engine.setRoot(previewAst.get());
                            engine.constantFolding();
                            std::string afterText = generateForLanguage(previewAst.get(),
                                                                        state.active()->language);
                            state.openDiff(beforeText, afterText, true, 1, {});
                        } else {
                            auto& inc = state.active()->incrementalOptimizer;
                            inc.setRoot(ast);
                            std::string tid = inc.applyTransform("constant-fold");
                            auto history = inc.getTransformHistory();
                            size_t nodes = 0;
                            for (const auto& h : history) {
                                if (h.transformId == tid) {
                                    nodes = h.affectedNodeIds.size();
                                    break;
                                }
                            }
                            std::string warning = findOptimizationLockWarning(ast);
                            std::string summary = "Constant Fold: ";
                            if (nodes > 0) {
                                summary += "applied (" + std::to_string(nodes) + " nodes)";
                            } else {
                                summary += "no changes";
                            }
                            if (!warning.empty()) {
                                summary += " — warning: " + warning;
                                state.outputLog += warning + "\n";
                            }
                            state.optimizeFoldSummary = summary;
                            state.outputLog += summary + "\n";
                            if (nodes > 0) state.applyOrchestratorToActive();
                            std::string afterText = state.active()->editBuf;
                            state.openDiff(beforeText, afterText, false, 1, {tid});
                        }
                    }
                    showBlockedTooltip(blockReason);
                    if (blocked) ImGui::EndDisabled();
                    if (!state.optimizeFoldSummary.empty()) {
                        ImGui::TextWrapped("%s", state.optimizeFoldSummary.c_str());
                    }

                    ImGui::Separator();

                    if (blocked) ImGui::BeginDisabled();
                    if (ImGui::Button("Dead Code Elimination")) {
                        std::string beforeText = state.active()->editBuf;
                        if (state.optimizePreview) {
                            auto previewAst = cloneModule(ast);
                            TransformEngine engine;
                            engine.setRoot(previewAst.get());
                            engine.deadCodeElimination();
                            std::string afterText = generateForLanguage(previewAst.get(),
                                                                        state.active()->language);
                            state.openDiff(beforeText, afterText, true, 2, {});
                        } else {
                            auto& inc = state.active()->incrementalOptimizer;
                            inc.setRoot(ast);
                            std::string tid = inc.applyTransform("dead-code-elim");
                            auto history = inc.getTransformHistory();
                            size_t nodes = 0;
                            for (const auto& h : history) {
                                if (h.transformId == tid) {
                                    nodes = h.affectedNodeIds.size();
                                    break;
                                }
                            }
                            std::string summary = "Dead Code Elimination: ";
                            if (nodes > 0) {
                                summary += "applied (" + std::to_string(nodes) + " nodes)";
                            } else {
                                summary += "no changes";
                            }
                            state.optimizeDeadCodeSummary = summary;
                            state.outputLog += summary + "\n";
                            if (nodes > 0) state.applyOrchestratorToActive();
                            std::string afterText = state.active()->editBuf;
                            state.openDiff(beforeText, afterText, false, 2, {tid});
                        }
                    }
                    showBlockedTooltip(blockReason);
                    if (blocked) ImGui::EndDisabled();
                    if (!state.optimizeDeadCodeSummary.empty()) {
                        ImGui::TextWrapped("%s", state.optimizeDeadCodeSummary.c_str());
                    }

                    ImGui::Separator();

                    if (blocked) ImGui::BeginDisabled();
                    if (ImGui::Button("Apply All")) {
                        std::string beforeText = state.active()->editBuf;
                        if (state.optimizePreview) {
                            auto previewAst = cloneModule(ast);
                            TransformEngine engine;
                            engine.setRoot(previewAst.get());
                            engine.applyAll();
                            std::string afterText = generateForLanguage(previewAst.get(),
                                                                        state.active()->language);
                            state.openDiff(beforeText, afterText, true, 3, {});
                        } else {
                            auto& inc = state.active()->incrementalOptimizer;
                            inc.setRoot(ast);
                            std::string tidFold = inc.applyTransform("constant-fold");
                            std::string tidDce = inc.applyTransform("dead-code-elim");
                            auto history = inc.getTransformHistory();
                            size_t totalNodes = 0;
                            for (const auto& h : history) {
                                if (h.transformId == tidFold || h.transformId == tidDce) {
                                    totalNodes += h.affectedNodeIds.size();
                                }
                            }
                            std::string warning = findOptimizationLockWarning(ast);
                            std::string summary = "Apply All: ";
                            if (totalNodes > 0) {
                                summary += "applied (" + std::to_string(totalNodes) + " nodes)";
                            } else {
                                summary += "no changes";
                            }
                            if (!warning.empty()) {
                                summary += " — warning: " + warning;
                                state.outputLog += warning + "\n";
                            }
                            state.optimizeApplySummary = summary;
                            state.outputLog += summary + "\n";
                            if (totalNodes > 0) state.applyOrchestratorToActive();
                            std::string afterText = state.active()->editBuf;
                            state.openDiff(beforeText, afterText, false, 3, {tidFold, tidDce});
                        }
                    }
                    showBlockedTooltip(blockReason);
                    if (blocked) ImGui::EndDisabled();
                    if (!state.optimizeApplySummary.empty()) {
                        ImGui::TextWrapped("%s", state.optimizeApplySummary.c_str());
                    }
                }
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Transform history
            if (ImGui::BeginTabItem("Transforms")) {
                ImGui::PushFont(uiFont);
                auto* buf = state.active();
                Module* ast = state.mutationAST();
                if (!buf) {
                    ImGui::TextDisabled("(no active buffer)");
                } else if (buf->bufferMode == BufferManager::BufferMode::Text) {
                    ImGui::TextDisabled("(disabled in Text mode)");
                } else if (!ast) {
                    ImGui::TextDisabled("(no AST)");
                } else {
                    auto& inc = buf->incrementalOptimizer;
                    inc.setRoot(ast);
                    auto history = inc.getTransformHistory();

                    const bool hasHistory = !history.empty();
                    if (!hasHistory) ImGui::BeginDisabled();
                    if (ImGui::Button("Undo All")) {
                        bool any = false;
                        while (inc.undoLast()) {
                            any = true;
                        }
                        if (any) state.applyOrchestratorToActive();
                    }
                    if (!hasHistory) ImGui::EndDisabled();
                    ImGui::Separator();

                    if (!hasHistory) {
                        ImGui::TextDisabled("(no transforms)");
                    } else {
                        for (const auto& h : history) {
                            ImGui::PushID(h.transformId.c_str());
                            ImVec4 color = transformColorForName(h.transformName);
                            ImGui::TextColored(color, "%s", h.transformName.c_str());
                            ImGui::SameLine();
                            ImGui::TextDisabled("@ %s", h.timestamp.c_str());
                            ImGui::SameLine();
                            if (ImGui::Button("Undo")) {
                                if (inc.undoTransform(h.transformId)) {
                                    state.applyOrchestratorToActive();
                                }
                            }
                            ImGui::Text("Affected: %d", (int)h.affectedNodeIds.size());
                            std::string nodeList;
                            for (size_t i = 0; i < h.affectedNodeIds.size() && i < 5; ++i) {
                                if (!nodeList.empty()) nodeList += ", ";
                                nodeList += h.affectedNodeIds[i];
                            }
                            if (h.affectedNodeIds.size() > 5) nodeList += ", ...";
                            if (nodeList.empty()) nodeList = "(none)";
                            ImGui::TextWrapped("Nodes: %s", nodeList.c_str());
                            ImGui::Separator();
                            ImGui::PopID();
                        }
                    }
                }
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // Diff view
            if (ImGui::BeginTabItem("Diff")) {
                ImGui::PushFont(uiFont);
                auto* buf = state.active();
                Module* ast = state.mutationAST();
                if (!buf) {
                    ImGui::TextDisabled("(no active buffer)");
                } else if (buf->bufferMode == BufferManager::BufferMode::Text) {
                    ImGui::TextDisabled("(diff view disabled in Text mode)");
                } else if (!state.diff.active) {
                    ImGui::TextDisabled("(no diff)");
                } else {
                    if (state.diff.preview) {
                        if (ImGui::Button("Apply")) {
                            if (ast && !buf->readOnly) {
                                if (state.diff.batch) {
                                    BatchMutationAPI batch;
                                    batch.setRoot(ast);
                                    auto res = batch.applySequence(state.diff.batchMutations);
                                    if (!res.success) {
                                        state.outputLog += res.error + "\n";
                                    } else {
                                        state.outputLog += "Refactor applied (" +
                                                           std::to_string(res.appliedCount) + " mutations).\n";
                                        state.applyOrchestratorToActive();
                                    }
                                } else {
                                    buf->incrementalOptimizer.setRoot(ast);
                                    if (state.diff.action == 1) {
                                        buf->incrementalOptimizer.applyTransform("constant-fold");
                                    } else if (state.diff.action == 2) {
                                        buf->incrementalOptimizer.applyTransform("dead-code-elim");
                                    } else if (state.diff.action == 3) {
                                        buf->incrementalOptimizer.applyTransform("constant-fold");
                                        buf->incrementalOptimizer.applyTransform("dead-code-elim");
                                    }
                                    state.applyOrchestratorToActive();
                                }
                            }
                            state.diff.active = false;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel")) {
                            state.diff.active = false;
                        }
                    } else {
                        if (ImGui::Button("Keep")) {
                            state.diff.active = false;
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Undo")) {
                            if (ast && !buf->readOnly) {
                                buf->incrementalOptimizer.setRoot(ast);
                                for (auto it = state.diff.transformIds.rbegin();
                                     it != state.diff.transformIds.rend(); ++it) {
                                    buf->incrementalOptimizer.undoTransform(*it);
                                }
                                state.applyOrchestratorToActive();
                            }
                            state.diff.active = false;
                        }
                    }

                    ImGui::Separator();
                    ImGui::BeginTable("##diffSplit", 2,
                                      ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp);
                    ImGui::TableSetupColumn("Before", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                    ImGui::TableSetupColumn("After", ImGuiTableColumnFlags_WidthStretch, 0.5f);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    CodeEditorOptions leftOpts;
                    leftOpts.readOnly = true;
                    leftOpts.showWhitespace = state.showWhitespace;
                    leftOpts.showLineNumbers = state.showLineNumbers;
                    leftOpts.showCurrentLine = false;
                    leftOpts.highlightLines = &state.diff.beforeLines;
                    leftOpts.highlightLineColor = IM_COL32(160, 80, 80, 120);
                    leftOpts.syncScrollX = &state.diffScrollX;
                    leftOpts.syncScrollY = &state.diffScrollY;
                    leftOpts.scrollMaster = true;
                    state.diffLeftWidget.render("##diffBefore",
                        state.diff.beforeText, std::vector<HighlightSpan>{}, leftOpts,
                        ImVec2(0, 0), monoFont);

                    ImGui::TableSetColumnIndex(1);
                    CodeEditorOptions rightOpts;
                    rightOpts.readOnly = true;
                    rightOpts.showWhitespace = state.showWhitespace;
                    rightOpts.showLineNumbers = state.showLineNumbers;
                    rightOpts.showCurrentLine = false;
                    rightOpts.highlightLines = &state.diff.afterLines;
                    rightOpts.highlightLineColor = IM_COL32(80, 160, 80, 120);
                    rightOpts.syncScrollX = &state.diffScrollX;
                    rightOpts.syncScrollY = &state.diffScrollY;
                    rightOpts.scrollMaster = false;
                    state.diffRightWidget.render("##diffAfter",
                        state.diff.afterText, std::vector<HighlightSpan>{}, rightOpts,
                        ImVec2(0, 0), monoFont);
                    ImGui::EndTable();
                }
                ImGui::PopFont();
                ImGui::EndTabItem();
            }

            // AST view
            if (ImGui::BeginTabItem("AST")) {
                ImGui::PushFont(monoFont);
                ImGui::BeginChild("##astScroll", ImVec2(0, 0), false);
                Module* ast = state.active() ? state.active()->sync.getAST() : nullptr;
                if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
                    ImGui::TextDisabled("(disabled in Text mode)");
                } else if (ast) {
                    std::map<std::string, std::string> transformNames;
                    if (state.active()) {
                        auto history = state.active()->incrementalOptimizer.getTransformHistory();
                        for (const auto& h : history) {
                            transformNames[h.transformId] = h.transformName;
                        }
                    }
                    // Show basic AST info
                    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f),
                                       "Module: %s  [%s]",
                                       ast->name.c_str(), ast->targetLanguage.c_str());
                    ImGui::Separator();
                    auto functions = ast->getChildren("functions");
                    for (size_t i = 0; i < functions.size(); ++i) {
                        auto* fn = static_cast<Function*>(functions[i]);
                        std::string tid;
                        std::string tname;
                        if (state.active()) {
                            tid = state.active()->incrementalOptimizer.getProvenance(fn->id);
                            auto it = transformNames.find(tid);
                            if (it != transformNames.end()) tname = it->second;
                        }
                        ImVec4 fnColor = tname.empty() ?
                            ImVec4(0.86f, 0.86f, 0.55f, 1.0f) :
                            transformColorForName(tname);
                        ImGui::TextColored(fnColor, "  Function: %s", fn->name.c_str());
                        if (!tname.empty() && ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("Transform: %s (%s)", tname.c_str(), tid.c_str());
                            ImGui::EndTooltip();
                        }
                        auto params = fn->getChildren("parameters");
                        for (auto* p : params) {
                            auto* param = static_cast<Parameter*>(p);
                            std::string pid;
                            std::string pname;
                            if (state.active()) {
                                pid = state.active()->incrementalOptimizer.getProvenance(param->id);
                                auto it = transformNames.find(pid);
                                if (it != transformNames.end()) pname = it->second;
                            }
                            ImVec4 pColor = pname.empty() ?
                                ImVec4(0.6f, 0.78f, 0.9f, 1.0f) :
                                transformColorForName(pname);
                            ImGui::TextColored(pColor, "    param: %s", param->name.c_str());
                            if (!pname.empty() && ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::Text("Transform: %s (%s)", pname.c_str(), pid.c_str());
                                ImGui::EndTooltip();
                            }
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
                if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
                    ImGui::TextDisabled("(disabled in Text mode)");
                } else {
                    state.updateGenerated();
                    if (ast && state.active()) {
                        ImGui::TextUnformatted(state.active()->generatedBuf.c_str());
                    } else {
                        ImGui::TextDisabled("(no AST)");
                    }
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
        if (state.active() && state.active()->bufferMode == BufferManager::BufferMode::Text) {
            ImGui::TextDisabled("(disabled in Text mode)");
        } else if (!ast) {
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

            // Mode
            if (state.active()) {
                const char* modeLabel =
                    state.active()->bufferMode == BufferManager::BufferMode::Text ? "Text" : "Structured";
                ImGui::Text("Mode: %s", modeLabel);
            } else {
                ImGui::Text("Mode: -");
            }
            ImGui::SameLine(0, 30);

            // Keybinding profile
            ImGui::Text("Keys: %s", KeybindingManager::profileName(state.keys.getProfile()));
            ImGui::SameLine(0, 30);

            // Zoom
            ImGui::Text("Zoom: %d%%", zoomPercent(state.settings.getFontSize(), baseFontSize));
            ImGui::SameLine(0, 30);

            // Undo depth
            ImGui::Text("Undo: %d", state.active() ? state.active()->undoDepth : 0);
            ImGui::SameLine(0, 30);

            if (state.runInProgress) {
                ImGui::Text("Run: Running...");
            } else if (state.hasRunResult) {
                ImGui::Text("Run: Exit %d", state.lastRunExitCode);
            } else {
                ImGui::Text("Run: -");
            }
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
    size_t iniSize = 0;
    const char* iniData = ImGui::SaveIniSettingsToMemory(&iniSize);
    if (iniData && iniSize > 0) {
        state.saveSession(std::string(iniData, iniSize));
    }
    state.shutdownAgentServer();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
