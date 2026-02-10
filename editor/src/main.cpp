// Whetstone Editor — ImGui-based structured code editor
//
// Panel rendering is delegated to headers in panels/.
// main.cpp handles: initialization, event loop, panel dispatch, cleanup.

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cctype>
#include <filesystem>
#include <vector>

#include "EditorState.h"
#include "EditorUtils.h"
#include "CompletionUtils.h"
#include "ThemeEngine.h"

// --- Panel headers ---
#include "panels/MenuBarPanel.h"
#include "panels/ExplorerPanel.h"
#include "panels/SidePanels.h"
#include "panels/SearchPanels.h"
#include "panels/SettingsPanel.h"
#include "panels/DialogPanels.h"
#include "panels/EditorPanel.h"
#include "panels/BottomPanel.h"
#include "panels/StatusBarPanel.h"

static std::string emacsChordForEvent(SDL_Keycode sym, Uint16 mods) {
    std::string base;
    if (sym >= SDLK_a && sym <= SDLK_z) {
        char c = (char)('a' + (sym - SDLK_a));
        if (mods & KMOD_SHIFT) c = (char)std::toupper(c);
        base.push_back(c);
    } else if (sym >= SDLK_0 && sym <= SDLK_9) {
        base.push_back((char)('0' + (sym - SDLK_0)));
    } else if (sym == SDLK_SPACE) {
        base = "SPC";
    } else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
        base = "RET";
    } else if (sym == SDLK_TAB) {
        base = "TAB";
    } else if (sym == SDLK_BACKSPACE) {
        base = "DEL";
    } else if (sym == SDLK_ESCAPE) {
        base = "ESC";
    } else if (sym == SDLK_MINUS) {
        base = "-";
    } else if (sym == SDLK_EQUALS) {
        base = "=";
    } else if (sym == SDLK_SLASH) {
        base = "/";
    } else if (sym == SDLK_SEMICOLON) {
        base = ";";
    } else if (sym == SDLK_PERIOD) {
        base = ".";
    }

    if (base.empty()) return "";

    std::string prefix;
    if (mods & KMOD_CTRL) prefix += "C-";
    if (mods & KMOD_ALT) prefix += "M-";
    if ((mods & KMOD_SHIFT) && base.size() > 1) prefix += "S-";

    return prefix + base;
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

    // Load fonts
    const float baseFontSize = 15.0f;
    ImFont* monoFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", baseFontSize);
    if (!monoFont) {
        monoFont = io.Fonts->AddFontDefault();
    }
    ImFont* uiFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", baseFontSize);
    if (!uiFont) {
        uiFont = io.Fonts->AddFontDefault();
    }

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Editor state
    EditorState state;
    state.monoFont = monoFont;
    state.uiFont = uiFont;
    state.baseFontSize = baseFontSize;
    state.init();

    auto loadFontFromPath = [&](const std::string& preferred,
                                const std::vector<std::string>& fallbacks,
                                float size) -> ImFont* {
        namespace fs = std::filesystem;
        if (!preferred.empty() && fs::exists(preferred)) {
            if (auto* f = io.Fonts->AddFontFromFileTTF(preferred.c_str(), size)) return f;
        }
        for (const auto& path : fallbacks) {
            if (fs::exists(path)) {
                if (auto* f = io.Fonts->AddFontFromFileTTF(path.c_str(), size)) return f;
            }
        }
        return io.Fonts->AddFontDefault();
    };

    auto reloadFonts = [&]() {
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        io.Fonts->Clear();
        std::vector<std::string> codeFallbacks;
        std::vector<std::string> uiFallbacks;
#ifdef _WIN32
        codeFallbacks = {
            "C:\\Windows\\Fonts\\consola.ttf",
            "C:\\Windows\\Fonts\\CascadiaCode.ttf",
            "C:\\Windows\\Fonts\\CascadiaMono.ttf",
            "C:\\Windows\\Fonts\\cour.ttf"
        };
        uiFallbacks = {
            "C:\\Windows\\Fonts\\segoeui.ttf",
            "C:\\Windows\\Fonts\\calibri.ttf",
            "C:\\Windows\\Fonts\\arial.ttf"
        };
#else
        codeFallbacks = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"
        };
        uiFallbacks = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"
        };
#endif
        state.monoFont = loadFontFromPath(state.settings.getCodeFontPath(),
                                          codeFallbacks,
                                          state.baseFontSize);
        state.uiFont = loadFontFromPath(state.settings.getUiFontPath(),
                                        uiFallbacks,
                                        state.baseFontSize);
        io.FontGlobalScale = state.settings.getFontSize() / state.baseFontSize;
        ImGui_ImplOpenGL3_CreateFontsTexture();
    };

    reloadFonts();
    ThemeEngine& themes = ThemeEngine::instance();
    themes.loadThemesFromDirectory("themes");
    themes.loadThemesFromDirectory("editor/themes");
    themes.loadThemesFromDirectory(ThemeEngine::userThemeDirectory());
    std::string themeName = state.settings.getTheme();
    if (themeName == "Dark") themeName = "Whetstone Dark";
    if (themeName == "Light") themeName = "Whetstone Light";
    if (!themes.applyTheme(themeName)) {
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

    bool done = false;
    while (!done) {
        // --- Event handling ---
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
                            state.search.projectSearch.setRoot(state.workspaceRoot);
                            state.refreshBuildSystem();
                        });
                    SDL_free(droppedFile);
                }
            }

            // Keyboard shortcuts
            if (event.type == SDL_KEYDOWN && !io.WantTextInput) {
                int key = 0;
                auto sym = event.key.keysym.sym;
                auto sdlMod = event.key.keysym.mod;
                if (state.ui.layoutPreset == LayoutPreset::Emacs) {
                    std::string chord = emacsChordForEvent(sym, sdlMod);
                    if (!chord.empty() && state.handleEmacsKeyChord(chord)) {
                        continue;
                    }
                }
                if (sym >= SDLK_a && sym <= SDLK_z) key = 'A' + (sym - SDLK_a);
                else if (sym >= SDLK_0 && sym <= SDLK_9) key = '0' + (sym - SDLK_0);
                else if (sym == SDLK_EQUALS) key = '=';
                else if (sym == SDLK_MINUS) key = '-';
                else if (sym == SDLK_SLASH) key = '/';
                else if (sym == SDLK_SEMICOLON) key = ';';
                else if (sym == SDLK_PERIOD) key = '.';
                else if (sym == SDLK_BACKQUOTE) key = '`';

                int mods = WMOD_NONE;
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
                    else if (action == "search.find") {
                        state.search.showFind = true;
                        state.search.showReplace = false;
                    }
                    else if (action == "search.replace") {
                        state.search.showFind = true;
                        state.search.showReplace = true;
                    }
                    else if (action == "search.findNext") state.doFindNext(false);
                    else if (action == "search.findPrev") state.doFindNext(true);
                    else if (action == "search.findInFiles") state.search.showProjectSearch = !state.search.showProjectSearch;
                    else if (action == "nav.goToLine") {
                        state.search.showGoToLine = true;
                        state.search.goToLineBuf[0] = '\0';
                        state.search.goToLineError = false;
                    }
                    else if (action == "view.toggleTerminal") {
                        state.build.showTerminalPanel = !state.build.showTerminalPanel;
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

        // --- Per-frame updates ---
        state.handleFileChanges();
        if (state.settings.getAutoSaveSeconds() > 0 && state.active() && state.active()->modified) {
            double now = ImGui::GetTime();
            if ((now - state.lastAutoSave) >= state.settings.getAutoSaveSeconds()) {
                state.doSave();
                state.lastAutoSave = now;
            }
        }
        state.pollLspMessages();
        state.processLibraryIndexResponses();
        if (state.emacsState.emacsFunctionIndexDirty) {
            state.updateEmacsFunctionIndex();
        }
        state.refreshEmacsModeLine(ImGui::GetTime());
        state.events.tick(ImGui::GetTime());
        themes.refreshWatchedThemes();
        if (state.fontsDirty) {
            reloadFonts();
            state.fontsDirty = false;
        }

        // --- Start frame ---
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

        // --- Render all panels ---
        renderMenuBar(state);
        renderExplorerPanel(state);
        renderOutlinePanel(state);
        renderDependenciesPanel(state);
        renderLibrariesPanel(state);
        renderCompositionPanel(state);
        renderEmacsPackagesPanel(state);
        renderEmacsBridgePanel(state);
        renderMinibuffer(state);
        renderFindReplaceBar(state);
        renderProjectSearchPanel(state);
        renderSettingsPanel(state);
        renderGoToLineDialog(state);
        renderLspSettingsPanel(state);
        renderRefactorPopup(state);
        renderCommandPalette(state);
        renderEditorPanel(state);
        renderBottomPanel(state);
        renderMemoryStrategiesPanel(state);
        renderStatusBar(state);
        state.notifications.renderHistoryWindow([&](const Notification& note) {
            if (note.hasTarget) state.navigateToTarget(note.target);
        });
        state.notifications.renderToasts([&](const Notification& note) {
            if (note.hasTarget) state.navigateToTarget(note.target);
        }, state.settings.getReduceMotion());

        // Check for exit request from menu
        if (state.exitRequested) done = true;

        // --- Render frame ---
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
