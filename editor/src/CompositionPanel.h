#pragma once
#include "imgui.h"
#include "CompositionBuilder.h"
#include <string>
#include <cstdio>

struct CompositionPanelState {
    char inputType[64] = {};
    char outputType[64] = {};
    char functionName[64] = "compose_pipeline";
    std::string inputVar = "x";
    int selected = -1;
};

static bool renderCompositionPanel(CompositionPanelState& state,
                                   const std::vector<PrimitiveSymbol>& primitives,
                                   std::string& outCode,
                                   std::string& outputLog) {
    outCode.clear();
    ImGui::InputText("Input Type", state.inputType, sizeof(state.inputType));
    ImGui::InputText("Output Type", state.outputType, sizeof(state.outputType));
    ImGui::InputText("Function Name", state.functionName, sizeof(state.functionName));
    char inputVarBuf[64];
    std::snprintf(inputVarBuf, sizeof(inputVarBuf), "%s", state.inputVar.c_str());
    if (ImGui::InputText("Input Var", inputVarBuf, sizeof(inputVarBuf))) {
        state.inputVar = inputVarBuf;
    }

    std::string input = state.inputType;
    std::string output = state.outputType;
    auto suggestions = suggestCompositionSteps(primitives, input, output);

    ImGui::Separator();
    ImGui::TextDisabled("Pick functions to build a pipeline:");
    ImGui::BeginChild("##composeList", ImVec2(0, 180), true);
    for (int i = 0; i < (int)suggestions.size(); ++i) {
        std::string label = suggestions[i].symbol + " (" + suggestions[i].inputType +
                            " -> " + suggestions[i].outputType + ")";
        if (ImGui::Selectable(label.c_str(), state.selected == i)) {
            state.selected = i;
        }
    }
    ImGui::EndChild();

    ImGui::Separator();
    if (state.selected >= 0 && state.selected < (int)suggestions.size()) {
        if (ImGui::Button("Add Step")) {
            if (state.steps.size() >= 8) {
                outputLog += "[compose] Max 8 steps.\n";
            } else {
                state.steps.push_back(suggestions[state.selected]);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            state.steps.clear();
        }
    } else {
        if (ImGui::Button("Clear")) {
            state.steps.clear();
        }
    }

    ImGui::Separator();
    ImGui::TextDisabled("Pipeline:");
    for (size_t i = 0; i < state.steps.size(); ++i) {
        ImGui::Text("%zu. %s", i + 1, state.steps[i].symbol.c_str());
    }

    if (!state.steps.empty()) {
        if (ImGui::Button("Remove Last")) {
            state.steps.pop_back();
        }
        ImGui::SameLine();
    }

    std::string funcName = state.functionName;
    if (funcName.empty()) funcName = "compose_pipeline";
    std::string code = buildCompositionFunction(state.steps, funcName, state.inputVar);
    if (!code.empty()) {
        ImGui::Separator();
        ImGui::TextDisabled("Generated:");
        ImGui::BeginChild("##composeCode", ImVec2(0, 120), true);
        ImGui::TextUnformatted(code.c_str());
        ImGui::EndChild();
        if (ImGui::Button("Insert Code")) {
            outCode = code;
            outputLog += "[compose] Inserted pipeline.\n";
            return true;
        }
    }

    return false;
}
