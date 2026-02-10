#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../ThemeEngine.h"
#include "../AnimationUtils.h"
#include <cstdlib>
#include <filesystem>
#include <vector>

static void openThemesFolder(const std::string& path) {
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + path + "\"";
    std::system(cmd.c_str());
#elif __APPLE__
    std::string cmd = "open \"" + path + "\"";
    std::system(cmd.c_str());
#else
    std::string cmd = "xdg-open \"" + path + "\"";
    std::system(cmd.c_str());
#endif
}

struct FontOption {
    std::string label;
    std::string path;
};

static void pushFontOption(std::vector<FontOption>& out,
                           const std::string& label,
                           const std::string& path) {
    std::error_code ec;
    if (path.empty() || std::filesystem::exists(path, ec)) {
        out.push_back({label, path});
    }
}

static std::vector<FontOption> collectFontOptions(bool monospace) {
    std::vector<FontOption> out;
    pushFontOption(out, "Default", "");
#ifdef _WIN32
    if (monospace) {
        pushFontOption(out, "Consolas", "C:\\Windows\\Fonts\\consola.ttf");
        pushFontOption(out, "Cascadia Code", "C:\\Windows\\Fonts\\CascadiaCode.ttf");
        pushFontOption(out, "Cascadia Mono", "C:\\Windows\\Fonts\\CascadiaMono.ttf");
        pushFontOption(out, "Courier New", "C:\\Windows\\Fonts\\cour.ttf");
    } else {
        pushFontOption(out, "Segoe UI", "C:\\Windows\\Fonts\\segoeui.ttf");
        pushFontOption(out, "Calibri", "C:\\Windows\\Fonts\\calibri.ttf");
        pushFontOption(out, "Arial", "C:\\Windows\\Fonts\\arial.ttf");
    }
#else
    if (monospace) {
        pushFontOption(out, "DejaVu Sans Mono", "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
        pushFontOption(out, "Liberation Mono", "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf");
    } else {
        pushFontOption(out, "DejaVu Sans", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
        pushFontOption(out, "Liberation Sans", "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    }
#endif
    return out;
}

static int findFontIndex(const std::vector<FontOption>& options, const std::string& path) {
    for (size_t i = 0; i < options.size(); ++i) {
        if (options[i].path == path) return (int)i;
    }
    return 0;
}

static void renderSettingsPanel(EditorState& state) {
    const double now = ImGui::GetTime();
    const bool reduceMotion = state.settings.getReduceMotion();
    float alpha = 1.0f;
    float offset = 0.0f;
    const bool render = AnimationUtils::panelTransition("SettingsPanel",
                                                        state.ui.showSettingsPanel,
                                                        now,
                                                        reduceMotion,
                                                        0.18f,
                                                        18.0f,
                                                        alpha,
                                                        offset);
    if (!render) return;
    bool open = state.ui.showSettingsPanel;
    ImGui::SetNextWindowBgAlpha(alpha);
    ImGui::Begin("Settings", &open);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
    ImGui::PushFont(state.uiFont);
    float startX = ImGui::GetCursorPosX();
    ImGui::SetCursorPosX(startX + offset);
    bool settingsChanged = false;
    bool emacsConfigChanged = false;
    bool themeChanged = false;
    bool fontsChanged = false;
    ImGuiIO& io = ImGui::GetIO();

    int fontSize = state.settings.getFontSize();
    if (ImGui::SliderInt("Font Size", &fontSize, 12, 24)) {
        state.settings.setFontSize(fontSize);
        io.FontGlobalScale = fontSize / state.baseFontSize;
        settingsChanged = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Typography");

    auto codeFonts = collectFontOptions(true);
    int codeFontIndex = findFontIndex(codeFonts, state.settings.getCodeFontPath());
    std::vector<const char*> codeFontLabels;
    codeFontLabels.reserve(codeFonts.size());
    for (const auto& opt : codeFonts) codeFontLabels.push_back(opt.label.c_str());
    if (ImGui::Combo("Code Font", &codeFontIndex, codeFontLabels.data(),
                     (int)codeFontLabels.size())) {
        state.settings.setCodeFontPath(codeFonts[codeFontIndex].path);
        settingsChanged = true;
        fontsChanged = true;
    }
    std::string codeFontPath = state.settings.getCodeFontPath();
    if (InputTextStr("Code Font Path", &codeFontPath)) {
        state.settings.setCodeFontPath(codeFontPath);
        settingsChanged = true;
        fontsChanged = true;
    }

    auto uiFonts = collectFontOptions(false);
    int uiFontIndex = findFontIndex(uiFonts, state.settings.getUiFontPath());
    std::vector<const char*> uiFontLabels;
    uiFontLabels.reserve(uiFonts.size());
    for (const auto& opt : uiFonts) uiFontLabels.push_back(opt.label.c_str());
    if (ImGui::Combo("UI Font", &uiFontIndex, uiFontLabels.data(),
                     (int)uiFontLabels.size())) {
        state.settings.setUiFontPath(uiFonts[uiFontIndex].path);
        settingsChanged = true;
        fontsChanged = true;
    }
    std::string uiFontPath = state.settings.getUiFontPath();
    if (InputTextStr("UI Font Path", &uiFontPath)) {
        state.settings.setUiFontPath(uiFontPath);
        settingsChanged = true;
        fontsChanged = true;
    }

    float lineHeight = state.settings.getLineHeightScale();
    if (ImGui::SliderFloat("Line Height", &lineHeight, 1.0f, 2.0f, "%.2f")) {
        state.settings.setLineHeightScale(lineHeight);
        settingsChanged = true;
    }
    float letterSpacing = state.settings.getLetterSpacing();
    if (ImGui::SliderFloat("Letter Spacing", &letterSpacing, 0.0f, 3.0f, "%.1f")) {
        state.settings.setLetterSpacing(letterSpacing);
        settingsChanged = true;
    }
    float blinkRate = state.settings.getCursorBlinkRate();
    if (ImGui::SliderFloat("Cursor Blink Rate (Hz)", &blinkRate, 0.0f, 4.0f, "%.1f")) {
        state.settings.setCursorBlinkRate(blinkRate);
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

    std::vector<std::string> themeNames = ThemeEngine::instance().listThemes();
    if (themeNames.empty()) {
        themeNames = {"Whetstone Dark", "Whetstone Light"};
    }
    std::string savedTheme = state.settings.getTheme();
    if (savedTheme == "Dark") savedTheme = "Whetstone Dark";
    if (savedTheme == "Light") savedTheme = "Whetstone Light";

    static std::string selectedTheme;
    static std::string previewAnchor;
    static bool previewActive = false;
    if (selectedTheme.empty()) selectedTheme = savedTheme;
    bool selectedExists = false;
    for (const auto& name : themeNames) {
        if (name == selectedTheme) {
            selectedExists = true;
            break;
        }
    }
    if (!selectedExists) selectedTheme = savedTheme;

    if (ImGui::CollapsingHeader("Themes", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Hover to preview, click to select.");
        const float cardWidth = 180.0f;
        const float cardHeight = 90.0f;
        const float padding = 8.0f;
        float avail = ImGui::GetContentRegionAvail().x;
        int columns = std::max(1, (int)(avail / (cardWidth + padding)));

        bool anyHovered = false;
        std::string hoveredTheme;

        if (ImGui::BeginTable("##themeGallery", columns, ImGuiTableFlags_SizingFixedFit)) {
            for (const auto& name : themeNames) {
                ImGui::TableNextColumn();
                ImGui::PushID(name.c_str());
                ImGui::BeginChild("##themeCard", ImVec2(cardWidth, cardHeight), true);
                ImGui::TextUnformatted(name.c_str());
                if (name == selectedTheme) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("Selected");
                }
                if (name == savedTheme) {
                    ImDrawList* draw = ImGui::GetWindowDrawList();
                    ImVec2 pos = ImGui::GetWindowPos();
                    ImVec2 size = ImGui::GetWindowSize();
                    ImVec2 a(pos.x + size.x - 18.0f, pos.y + 10.0f);
                    ImVec2 b(pos.x + size.x - 12.0f, pos.y + 16.0f);
                    ImVec2 c(pos.x + size.x - 4.0f, pos.y + 6.0f);
                    ImU32 color = IM_COL32(80, 200, 120, 255);
                    draw->AddLine(a, b, color, 2.0f);
                    draw->AddLine(b, c, color, 2.0f);
                }

                ImU32 swatch[4] = {
                    IM_COL32(40, 40, 40, 255),
                    IM_COL32(197, 134, 192, 255),
                    IM_COL32(206, 145, 120, 255),
                    IM_COL32(106, 153, 85, 255)
                };
                ThemeEngine::instance().getSwatch(name, swatch);
                ImVec2 start = ImGui::GetCursorScreenPos();
                ImDrawList* draw = ImGui::GetWindowDrawList();
                float swatchWidth = (cardWidth - 2.0f * padding) / 4.0f;
                float swatchHeight = 24.0f;
                for (int i = 0; i < 4; ++i) {
                    ImVec2 a(start.x + i * swatchWidth, start.y);
                    ImVec2 b(start.x + (i + 1) * swatchWidth - 2.0f, start.y + swatchHeight);
                    draw->AddRectFilled(a, b, swatch[i], 3.0f);
                }
                ImGui::Dummy(ImVec2(0.0f, swatchHeight + 4.0f));
                ImGui::EndChild();

                if (ImGui::IsItemHovered()) {
                    anyHovered = true;
                    hoveredTheme = name;
                }
                if (ImGui::IsItemClicked()) {
                    selectedTheme = name;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (anyHovered) {
            if (!previewActive) {
                previewAnchor = savedTheme;
                previewActive = true;
            }
            if (!hoveredTheme.empty() &&
                ThemeEngine::instance().currentThemeName() != hoveredTheme) {
                ThemeEngine::instance().applyTheme(hoveredTheme);
            }
        } else if (previewActive) {
            ThemeEngine::instance().applyTheme(previewAnchor);
            previewActive = false;
        }

        ImGui::Separator();
        bool canApply = !selectedTheme.empty() && selectedTheme != savedTheme;
        if (!canApply) ImGui::BeginDisabled();
        if (ImGui::Button("Apply")) {
            state.settings.setTheme(selectedTheme);
            if (!ThemeEngine::instance().applyTheme(selectedTheme)) {
                if (selectedTheme.find("Light") != std::string::npos)
                    SetupVSCodeLightTheme();
                else
                    SetupVSCodeDarkTheme();
            }
            previewActive = false;
            previewAnchor = selectedTheme;
            settingsChanged = true;
            themeChanged = true;
            savedTheme = selectedTheme;
        }
        if (!canApply) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            selectedTheme = savedTheme;
            ThemeEngine::instance().applyTheme(savedTheme);
            previewActive = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Open Themes Folder")) {
            openThemesFolder(ThemeEngine::userThemeDirectory());
        }
    }

    bool telemetryOptIn = state.settings.getTelemetryOptIn();
    if (ImGui::Checkbox("Telemetry (opt-in)", &telemetryOptIn)) {
        state.settings.setTelemetryOptIn(telemetryOptIn);
        state.telemetry.setOptIn(telemetryOptIn);
        if (telemetryOptIn) {
            state.telemetry.recordEvent("telemetry_opt_in", json::object());
        }
        settingsChanged = true;
    }

    char updateUrlBuf[256];
    std::snprintf(updateUrlBuf, sizeof(updateUrlBuf), "%s",
                  state.settings.getUpdateUrl().c_str());
    if (ImGui::InputText("Update URL", updateUrlBuf, sizeof(updateUrlBuf))) {
        state.settings.setUpdateUrl(updateUrlBuf);
        settingsChanged = true;
    }
    if (ImGui::Button("Check for Updates")) {
        UpdateChecker checker;
        auto info = checker.check(state.settings.getUpdateUrl());
        if (info.available) {
            state.notify(NotificationLevel::Info,
                         "[update] Available: " + info.version + " (" + info.url + ")");
        } else {
            state.notify(NotificationLevel::Info,
                         "[update] No update info (offline/stub).");
        }
    }

    int autoSave = state.settings.getAutoSaveSeconds();
    if (ImGui::InputInt("Auto-save (sec)", &autoSave)) {
        autoSave = std::max(0, autoSave);
        state.settings.setAutoSaveSeconds(autoSave);
        settingsChanged = true;
    }

    bool showMinimap = state.ui.showMinimap;
    if (ImGui::Checkbox("Show Minimap", &showMinimap)) {
        state.ui.showMinimap = showMinimap;
        settingsChanged = true;
    }
    bool showLineNumbers = state.ui.showLineNumbers;
    if (ImGui::Checkbox("Show Line Numbers", &showLineNumbers)) {
        state.ui.showLineNumbers = showLineNumbers;
        settingsChanged = true;
    }
    std::string defaultMode = state.settings.getDefaultBufferMode();
    int modeIndex = (defaultMode == "structured") ? 1 : 0;
    const char* modeLabels[] = {"Text (Recommended)", "Structured"};
    if (ImGui::Combo("Default Buffer Mode", &modeIndex, modeLabels, 2)) {
        state.settings.setDefaultBufferMode(modeIndex == 1 ? "structured" : "text");
        settingsChanged = true;
    }
    bool reduceMotionSetting = state.settings.getReduceMotion();
    if (ImGui::Checkbox("Reduce Motion", &reduceMotionSetting)) {
        state.settings.setReduceMotion(reduceMotionSetting);
        settingsChanged = true;
    }
    bool useAnnotationShapes = state.settings.getUseAnnotationShapes();
    if (ImGui::Checkbox("Use Shapes for Annotations", &useAnnotationShapes)) {
        state.settings.setUseAnnotationShapes(useAnnotationShapes);
        settingsChanged = true;
    }
    bool blockVulnImports = state.settings.getBlockVulnerableImports();
    if (ImGui::Checkbox("Block Vulnerable Imports", &blockVulnImports)) {
        state.settings.setBlockVulnerableImports(blockVulnImports);
        settingsChanged = true;
    }
    bool autoRecordSessions = state.settings.getAutoRecordSessions();
    if (ImGui::Checkbox("Auto-record Sessions", &autoRecordSessions)) {
        state.settings.setAutoRecordSessions(autoRecordSessions);
        if (autoRecordSessions && !state.agent.workflowRecorder.isRecording()) {
            state.startSessionRecording("auto-session", true);
        } else if (!autoRecordSessions &&
                   state.agent.workflowRecorder.isRecording() &&
                   state.agent.workflowRecorder.isAutoRecording()) {
            state.stopSessionRecording();
        }
        settingsChanged = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Large File Thresholds (MB)");
    int warnMb = state.settings.getLargeFileWarnMB();
    if (ImGui::InputInt("Warn > MB", &warnMb)) {
        warnMb = std::max(1, warnMb);
        state.settings.setLargeFileWarnMB(warnMb);
        settingsChanged = true;
    }
    int textMb = state.settings.getLargeFileTextMB();
    if (ImGui::InputInt("Auto Text Mode > MB", &textMb)) {
        textMb = std::max(1, textMb);
        state.settings.setLargeFileTextMB(textMb);
        settingsChanged = true;
    }
    int disableHlMb = state.settings.getLargeFileDisableHighlightMB();
    if (ImGui::InputInt("Disable Highlight > MB", &disableHlMb)) {
        disableHlMb = std::max(1, disableHlMb);
        state.settings.setLargeFileDisableHighlightMB(disableHlMb);
        settingsChanged = true;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Debounce (ms)");
    int lspDebounce = state.settings.getLspDebounceMs();
    if (ImGui::InputInt("LSP Change", &lspDebounce)) {
        lspDebounce = std::max(0, lspDebounce);
        state.settings.setLspDebounceMs(lspDebounce);
        settingsChanged = true;
    }
    int diagDebounce = state.settings.getDiagnosticsDebounceMs();
    if (ImGui::InputInt("Diagnostics", &diagDebounce)) {
        diagDebounce = std::max(0, diagDebounce);
        state.settings.setDiagnosticsDebounceMs(diagDebounce);
        settingsChanged = true;
    }
    int highlightDebounce = state.settings.getHighlightDebounceMs();
    if (ImGui::InputInt("Highlighting", &highlightDebounce)) {
        highlightDebounce = std::max(0, highlightDebounce);
        state.settings.setHighlightDebounceMs(highlightDebounce);
        settingsChanged = true;
    }

    LayoutPreset preset = state.ui.layoutPreset;
    int presetIndex = 0;
    if (preset == LayoutPreset::Emacs) presetIndex = 1;
    else if (preset == LayoutPreset::JetBrains) presetIndex = 2;
    const char* presetLabels[] = {"VSCode", "Emacs", "JetBrains"};
    if (ImGui::Combo("Layout Preset", &presetIndex, presetLabels, 3)) {
        state.ui.layoutPreset = (presetIndex == 1) ? LayoutPreset::Emacs :
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
        emacsConfigChanged = true;
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
        if (fontsChanged) {
            state.fontsDirty = true;
        }
        state.saveSettingsToDisk();
        state.events.publish(UIEventType::SettingsChanged, {}, {}, ImGui::GetTime());
    }
    if (emacsConfigChanged) {
        state.startEmacsDaemonFromSettings();
    }
    if (themeChanged) {
        state.events.publish(UIEventType::ThemeChanged, {}, {}, ImGui::GetTime());
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::End();
    if (!open) {
        state.ui.showSettingsPanel = false;
    }
}
