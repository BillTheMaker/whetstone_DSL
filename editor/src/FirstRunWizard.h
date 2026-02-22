#pragma once

#include "imgui.h"
#include "ThemeEngine.h"
#include "EditorState.h"

#include <vector>

struct FirstRunWizardState {
    bool open = false;
    int step = 0;
    int layoutIndex = 0;
    int themeIndex = 0;
    int keyIndex = 0;
    int modeIndex = 0;
};

static void renderFirstRunWizard(EditorState& state, FirstRunWizardState& wizard) {
    if (!wizard.open) return;

    ImGui::SetNextWindowSize(ImVec2(520, 360), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Setup Wizard", &wizard.open, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return;
    }

    const char* layoutLabels[] = {"VSCode", "Emacs", "JetBrains"};
    const LayoutPreset layoutValues[] = {LayoutPreset::VSCode, LayoutPreset::Emacs, LayoutPreset::JetBrains};
    const char* keyLabels[] = {"VSCode", "JetBrains", "Emacs"};
    const KeybindingProfile keyValues[] = {KeybindingProfile::VSCode, KeybindingProfile::JetBrains, KeybindingProfile::Emacs};
    std::vector<std::string> themeNames = ThemeEngine::instance().listThemes();
    if (themeNames.empty()) {
        themeNames = {"Whetstone Dark", "Whetstone Light"};
    }
    int maxThemes = std::min(4, (int)themeNames.size());

    if (wizard.step == 0) {
        ImGui::TextUnformatted("Welcome to Whetstone");
        ImGui::Separator();
        ImGui::TextWrapped("This short wizard helps you pick a layout, theme, and keybindings.");
    } else if (wizard.step == 1) {
        ImGui::TextUnformatted("Choose a layout preset");
        ImGui::Separator();
        for (int i = 0; i < IM_ARRAYSIZE(layoutLabels); ++i) {
            if (ImGui::RadioButton(layoutLabels[i], wizard.layoutIndex == i)) {
                wizard.layoutIndex = i;
            }
        }
    } else if (wizard.step == 2) {
        ImGui::TextUnformatted("Choose a theme");
        ImGui::Separator();
        for (int i = 0; i < maxThemes; ++i) {
            if (ImGui::RadioButton(themeNames[i].c_str(), wizard.themeIndex == i)) {
                wizard.themeIndex = i;
            }
        }
    } else if (wizard.step == 3) {
        ImGui::TextUnformatted("Choose keybindings");
        ImGui::Separator();
        for (int i = 0; i < IM_ARRAYSIZE(keyLabels); ++i) {
            if (ImGui::RadioButton(keyLabels[i], wizard.keyIndex == i)) {
                wizard.keyIndex = i;
            }
        }
    } else if (wizard.step == 4) {
        ImGui::TextUnformatted("Choose editor mode");
        ImGui::Separator();
        ImGui::TextWrapped("Text mode is recommended for a familiar editor experience. "
                           "Structured mode enables AST-driven features.");
        if (ImGui::RadioButton("Text (Recommended)", wizard.modeIndex == 0)) {
            wizard.modeIndex = 0;
        }
        if (ImGui::RadioButton("Structured", wizard.modeIndex == 1)) {
            wizard.modeIndex = 1;
        }
    } else if (wizard.step == 5) {
        ImGui::TextUnformatted("Get started");
        ImGui::Separator();
        if (ImGui::Button("Open Example Project")) {
            state.notify(NotificationLevel::Info, "Example project not bundled yet.");
        }
        if (ImGui::Button("Open Folder")) {
            auto path = FileDialog::openFolder({"Open Folder", state.workspaceRoot});
            if (!path.empty()) {
                std::string error;
                if (!state.setWorkspaceRoot(path, &error)) {
                    state.notify(NotificationLevel::Error,
                                 "Failed to open folder: " + error);
                }
            }
        }
        if (ImGui::Button("New File")) {
            std::string lang = state.active() ? state.active()->language : "python";
            state.createBuffer(state.makeUntitledName(), "", lang, state.defaultBufferMode());
        }
    }

    ImGui::Separator();
    if (wizard.step > 0) {
        if (ImGui::Button("Back")) {
            wizard.step = std::max(0, wizard.step - 1);
        }
        ImGui::SameLine();
    }
    if (wizard.step < 5) {
        if (ImGui::Button("Next")) {
            wizard.step = std::min(5, wizard.step + 1);
        }
    } else {
        if (ImGui::Button("Finish")) {
            if (wizard.layoutIndex >= 0 && wizard.layoutIndex < IM_ARRAYSIZE(layoutValues)) {
                state.ui.layoutPreset = layoutValues[wizard.layoutIndex];
            }
            if (wizard.keyIndex >= 0 && wizard.keyIndex < IM_ARRAYSIZE(keyValues)) {
                state.keys.setProfile(keyValues[wizard.keyIndex]);
                state.registerCommands();
            }
            if (wizard.themeIndex >= 0 && wizard.themeIndex < maxThemes) {
                state.settings.setTheme(themeNames[wizard.themeIndex]);
                ThemeEngine::instance().applyTheme(themeNames[wizard.themeIndex]);
            }
            state.settings.setDefaultBufferMode(wizard.modeIndex == 1 ? "structured" : "text");
            state.saveSettingsToDisk();
            wizard.open = false;
            state.ui.showFirstRunWizard = false;
        }
    }

    ImGui::End();
}
