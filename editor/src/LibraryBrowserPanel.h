#pragma once
#include "imgui.h"
#include "LibraryIndexer.h"
#include "ast/ExternalModule.h"
#include "NotificationSystem.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

struct LibraryBrowserState {
    char filterBuf[128] = {};
    std::string selectedLibrary;
    std::string selectedSymbol;
    std::string docText;
    std::vector<std::string> activeTags;
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

static std::string joinTags(const std::vector<std::string>& tags) {
    std::string out;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) out += " ";
        out += tags[i];
    }
    return out;
}

static std::vector<std::string> combinedTags(const ExternalModule* ext,
                                             const TypeSignature* sig) {
    std::vector<std::string> out = ext ? ext->semanticTags : std::vector<std::string>();
    if (sig) {
        out.insert(out.end(), sig->semanticTags.begin(), sig->semanticTags.end());
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

static bool symbolMatchesTags(const std::vector<std::string>& tags,
                              const std::vector<std::string>& active) {
    if (active.empty()) return true;
    for (const auto& tag : active) {
        if (std::find(tags.begin(), tags.end(), tag) == tags.end()) return false;
    }
    return true;
}

static std::vector<std::string> collectAvailableTags(Module* ast) {
    std::vector<std::string> tags;
    if (!ast) return tags;
    for (auto* node : ast->getChildren("externalModules")) {
        auto* ext = static_cast<ExternalModule*>(node);
        for (const auto& tag : ext->semanticTags) tags.push_back(tag);
        for (auto* sigNode : ext->getChildren("signatures")) {
            auto* sig = static_cast<TypeSignature*>(sigNode);
            for (const auto& tag : sig->semanticTags) tags.push_back(tag);
        }
    }
    std::sort(tags.begin(), tags.end());
    tags.erase(std::unique(tags.begin(), tags.end()), tags.end());
    return tags;
}

static bool renderLibraryBrowser(LibraryBrowserState& state,
                                 Module* ast,
                                 const LibraryIndexData& index,
                                 std::string& outInsert,
                                 std::string& outLibrary,
                                 NotificationSystem& notifications) {
    outInsert.clear();
    outLibrary.clear();
    if (!ast) {
        ImGui::TextDisabled("(no structured AST)");
        return false;
    }

    auto availableTags = collectAvailableTags(ast);
    if (!availableTags.empty()) {
        if (!state.activeTags.empty()) {
            ImGui::TextUnformatted("Active tags");
            for (const auto& tag : state.activeTags) {
                ImGui::PushID(tag.c_str());
                ImGui::TextDisabled("%s", tag.c_str());
                ImGui::SameLine();
                if (ImGui::SmallButton("x")) {
                    state.activeTags.erase(std::remove(state.activeTags.begin(),
                                                       state.activeTags.end(), tag),
                                           state.activeTags.end());
                    ImGui::PopID();
                    break;
                }
                ImGui::SameLine();
                ImGui::PopID();
            }
            ImGui::NewLine();
        }
        ImGui::TextUnformatted("Filter by tag");
        for (const auto& tag : availableTags) {
            bool active = std::find(state.activeTags.begin(), state.activeTags.end(), tag)
                != state.activeTags.end();
            if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::SmallButton(tag.c_str())) {
                if (active) {
                    state.activeTags.erase(std::remove(state.activeTags.begin(),
                                                       state.activeTags.end(), tag),
                                           state.activeTags.end());
                } else {
                    state.activeTags.push_back(tag);
                }
            }
            if (active) ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::NewLine();
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
                auto tags = combinedTags(ext, sig);
                if (!symbolMatchesFilter(sig->name, filter)) continue;
                if (!symbolMatchesTags(tags, state.activeTags)) continue;
                std::string label = sig->name;
                bool selected = (state.selectedLibrary == ext->name && state.selectedSymbol == sig->name);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    state.selectedLibrary = ext->name;
                    state.selectedSymbol = sig->name;
                    updateDocText(state, index, ext->name, sig->name);
                }
                if (!tags.empty()) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", joinTags(tags).c_str());
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
        for (auto* node : modules) {
            auto* ext = static_cast<ExternalModule*>(node);
            if (ext->name != state.selectedLibrary) continue;
            for (auto* sigNode : ext->getChildren("signatures")) {
                auto* sig = static_cast<TypeSignature*>(sigNode);
                if (sig->name != state.selectedSymbol) continue;
                auto tags = combinedTags(ext, sig);
                if (!tags.empty()) {
                    ImGui::TextDisabled("Tags: %s", joinTags(tags).c_str());
                }
                break;
            }
            break;
        }
        ImGui::TextWrapped("%s", state.docText.c_str());
        std::string usage = formatUsageTemplate(state.selectedSymbol);
        if (ImGui::Button("Insert")) {
            outInsert = usage;
            outLibrary = state.selectedLibrary;
            notifications.notify(NotificationLevel::Success,
                                 "[libs] Inserted " + usage);
            inserted = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", usage.c_str());
    }

    return inserted;
}
