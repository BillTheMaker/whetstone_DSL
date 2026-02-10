#pragma once

#include "imgui.h"

enum class WizardNavAction {
    None,
    Back,
    Next,
    Finish,
    Cancel
};

struct WizardFrame {
    bool open = false;
    int step = 0;
    int stepCount = 1;
};

static bool beginWizardWindow(const char* title,
                              WizardFrame& frame,
                              const ImVec2& size = ImVec2(560, 420)) {
    if (!frame.open) return false;
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(title, &frame.open, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return false;
    }
    int total = frame.stepCount > 0 ? frame.stepCount : 1;
    int step = std::max(0, std::min(frame.step, total - 1));
    frame.step = step;
    ImGui::TextDisabled("Step %d of %d", step + 1, total);
    ImGui::Separator();
    return true;
}

static WizardNavAction renderWizardFooter(WizardFrame& frame,
                                          const char* finishLabel = "Finish") {
    WizardNavAction action = WizardNavAction::None;
    ImGui::Separator();
    if (ImGui::Button("Cancel")) {
        frame.open = false;
        action = WizardNavAction::Cancel;
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(8.0f, 0.0f));
    ImGui::SameLine();
    if (frame.step > 0) {
        if (ImGui::Button("Back")) {
            frame.step = std::max(0, frame.step - 1);
            action = WizardNavAction::Back;
        }
        ImGui::SameLine();
    }
    bool isLast = frame.step >= frame.stepCount - 1;
    if (!isLast) {
        if (ImGui::Button("Next")) {
            frame.step = std::min(frame.step + 1, frame.stepCount - 1);
            action = WizardNavAction::Next;
        }
    } else {
        if (ImGui::Button(finishLabel)) {
            frame.open = false;
            action = WizardNavAction::Finish;
        }
    }
    return action;
}
