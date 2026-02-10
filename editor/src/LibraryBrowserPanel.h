#pragma once
#include "imgui.h"
#include "LibraryIndexer.h"
#include "ast/ExternalModule.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

struct LibraryBrowserState {
    char filterBuf[128] = {};
    std::string selectedLibrary;
    std::string selectedSymbol;
    std::string docText;
};

static std::string formatUsageTemplate(const std::string& symbol) {
    if (symbol.empty()) return symbol;
    if (std::isupper((unsigned char)symbol[0])) return symbol;
    if (symbol.find('(') != std::string::npos) return symbol;
    return symbol + "()";
}

static void updateDocText(LibraryBrowserState& state,
                          const LibraryIndexData& index,
                          const std::string& library,
                          const std::string& symbol) {
    state.docText.clear();
    auto it = index.symbolsByLibrary.find(library);
    if (it != index.symbolsByLibrary.end()) {
        for (const auto& sym : it->second) {
            if (sym.name == symbol) {
                if (!sym.detail.empty()) {
                    state.docText = sym.detail;
                }
                break;
            }
        }
    }
    if (state.docText.empty()) {
        state.docText = "No documentation available.";
    }
}

static bool symbolMatchesFilter(const std::string& symbol, const std::string& filter) {
    if (filter.empty()) return true;
    std::string a = symbol;
    std::string b = filter;
    std::transform(a.begin(), a.end(), a.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    std::transform(b.begin(), b.end(), b.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return a.find(b) != std::string::npos;
}

static bool renderLibraryBrowser(LibraryBrowserState& state,
                                 Module* ast,
                                 const LibraryIndexData& index,
                                 std::string& outInsert,
                                 std::string& outputLog) {
    outInsert.clear();
    if (!ast) {
        ImGui::TextDisabled("(no structured AST)");
        return false;
    }

    ImGui::InputText("Filter", state.filterBuf, sizeof(state.filterBuf));
    std::string filter = state.filterBuf;

    ImGui::Separator();
    bool inserted = false;
    auto modules = ast->getChildren("externalModules");
    if (modules.empty()) {
        ImGui::TextDisabled("(no libraries indexed)");
        return false;
    }

    for (auto* node : modules) {
        auto* ext = static_cast<ExternalModule*>(node);
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        bool open = ImGui::TreeNodeEx(ext->name.c_str(), flags);
        if (open) {
            auto sigs = ext->getChildren("signatures");
            for (auto* sigNode : sigs) {
                auto* sig = static_cast<TypeSignature*>(sigNode);
                if (!symbolMatchesFilter(sig->name, filter)) continue;
                std::string label = sig->name;
                bool selected = (state.selectedLibrary == ext->name && state.selectedSymbol == sig->name);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    state.selectedLibrary = ext->name;
                    state.selectedSymbol = sig->name;
                    updateDocText(state, index, ext->name, sig->name);
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    std::string usage = formatUsageTemplate(sig->name);
                    ImGui::SetDragDropPayload("WhetstoneSymbol", usage.c_str(), usage.size() + 1);
                    ImGui::TextUnformatted(usage.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::TreePop();
        }
    }

    if (!state.selectedSymbol.empty()) {
        ImGui::Separator();
        ImGui::Text("Selected: %s", state.selectedSymbol.c_str());
        ImGui::TextWrapped("%s", state.docText.c_str());
        std::string usage = formatUsageTemplate(state.selectedSymbol);
        if (ImGui::Button("Insert")) {
            outInsert = usage;
            outputLog += "[libs] Inserted " + usage + "\n";
            inserted = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", usage.c_str());
    }

    return inserted;
}
