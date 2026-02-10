#pragma once
#include "../EditorState.h"
#include "../EditorUtils.h"
#include "../ThemeEngine.h"

static void renderSettingsPanel(EditorState& state) {
    if (!state.ui.showSettingsPanel) return;
    ImGui::Begin("Settings", &state.ui.showSettingsPanel);
    ImGui::PushFont(state.uiFont);
    bool settingsChanged = false;
    bool emacsConfigChanged = false;
    ImGuiIO& io = ImGui::GetIO();

    int fontSize = state.settings.getFontSize();
    if (ImGui::SliderInt("Font Size", &fontSize, 12, 24)) {
        state.settings.setFontSize(fontSize);
        io.FontGlobalScale = fontSize / state.baseFontSize;
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
        themeNames = {"VSCode Dark", "VSCode Light"};
    }
    std::string currentTheme = state.settings.getTheme();
    if (currentTheme == "Dark") currentTheme = "VSCode Dark";
    if (currentTheme == "Light") currentTheme = "VSCode Light";
    int themeIndex = 0;
    for (size_t i = 0; i < themeNames.size(); ++i) {
        if (themeNames[i] == currentTheme) {
            themeIndex = (int)i;
            break;
        }
    }
    std::vector<const char*> themeLabels;
    themeLabels.reserve(themeNames.size());
    for (const auto& name : themeNames) themeLabels.push_back(name.c_str());
    if (ImGui::Combo("Theme", &themeIndex, themeLabels.data(), (int)themeLabels.size())) {
        state.settings.setTheme(themeNames[themeIndex]);
        if (!ThemeEngine::instance().applyTheme(themeNames[themeIndex])) {
            if (themeNames[themeIndex].find("Light") != std::string::npos)
                SetupVSCodeLightTheme();
            else
                SetupVSCodeDarkTheme();
        }
        settingsChanged = true;
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
        state.saveSettingsToDisk();
    }
    if (emacsConfigChanged) {
        state.startEmacsDaemonFromSettings();
    }
    ImGui::PopFont();
    ImGui::End();
}
