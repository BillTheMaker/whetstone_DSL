#pragma once
#include "imgui.h"
#include "EmacsIntegration.h"
#include "StringUtils.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

struct EmacsPackageEntry {
    std::string name;
    std::string version;
    std::string description;
    std::string status; // loaded | available | not-installed
};

struct EmacsPackageBrowserState {
    std::vector<EmacsPackageEntry> packages;
    char filterBuf[128] = {};
    char loadBuf[128] = {};
    int selected = -1;
    bool showAvailable = true;
    bool needsRefresh = true;
    std::string lastError;
};

// trimCopy is provided by EditorUtils.h

static inline std::string toLowerCopyPkg(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

static std::string buildPackageListCommand(const std::string& listSymbol) {
    std::string cmd =
        "(progn "
        "(require 'package) "
        "(mapconcat "
        " (lambda (p) "
        "  (let* ((desc (cond "
        "                ((and (consp (cdr p)) (listp (cdr p))) (car (cdr p))) "
        "                ((consp (cdr p)) (cdr p)) "
        "                (t nil))) "
        "         (name (symbol-name (car p))) "
        "         (ver (if (and desc (fboundp 'package-desc-version)) "
        "                  (package-version-join (package-desc-version desc)) \"\")) "
        "         (summary (if (and desc (fboundp 'package-desc-summary)) "
        "                     (package-desc-summary desc) \"\"))) "
        "    (concat name \"|\" ver \"|\" summary))) "
        + listSymbol +
        " \"\\n\"))";
    return cmd;
}

static std::vector<EmacsPackageEntry> parsePackageListText(const std::string& text) {
    std::vector<EmacsPackageEntry> out;
    if (text.empty()) return out;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        std::string line = trimCopy(text.substr(start, end - start));
        if (!line.empty()) {
            size_t a = line.find('|');
            size_t b = a == std::string::npos ? std::string::npos : line.find('|', a + 1);
            EmacsPackageEntry entry;
            if (a == std::string::npos) {
                entry.name = line;
            } else {
                entry.name = trimCopy(line.substr(0, a));
                if (b == std::string::npos) {
                    entry.version = trimCopy(line.substr(a + 1));
                } else {
                    entry.version = trimCopy(line.substr(a + 1, b - a - 1));
                    entry.description = trimCopy(line.substr(b + 1));
                }
            }
            if (!entry.name.empty()) {
                out.push_back(std::move(entry));
            }
        }
        start = end + 1;
    }
    return out;
}

static std::vector<EmacsPackageEntry> mergePackageLists(const std::vector<EmacsPackageEntry>& loaded,
                                                        const std::vector<EmacsPackageEntry>& available,
                                                        bool includeAvailable) {
    std::unordered_map<std::string, EmacsPackageEntry> merged;
    for (auto entry : loaded) {
        entry.status = "loaded";
        merged[entry.name] = std::move(entry);
    }
    if (includeAvailable) {
        for (auto entry : available) {
            auto it = merged.find(entry.name);
            if (it == merged.end()) {
                entry.status = "available";
                merged[entry.name] = std::move(entry);
            } else if (it->second.description.empty() && !entry.description.empty()) {
                it->second.description = entry.description;
            }
        }
    }
    std::vector<EmacsPackageEntry> out;
    out.reserve(merged.size());
    for (auto& kv : merged) out.push_back(std::move(kv.second));
    std::sort(out.begin(), out.end(),
              [](const EmacsPackageEntry& a, const EmacsPackageEntry& b) {
                  return a.name < b.name;
              });
    return out;
}

static std::vector<EmacsPackageEntry> queryEmacsPackageList(EmacsConnection& emacs,
                                                            bool includeAvailable,
                                                            std::string& logOut,
                                                            std::string& outError) {
    outError.clear();
    std::string loadedText = emacs.sendCommand(buildPackageListCommand("package-alist"));
    if (loadedText.empty() && !emacs.getLastError().empty()) {
        outError = emacs.getLastError();
        logOut += "[emacs] " + outError + "\n";
    }
    auto loaded = parsePackageListText(loadedText);
    std::vector<EmacsPackageEntry> available;
    if (includeAvailable) {
        std::string availText = emacs.sendCommand(buildPackageListCommand("package-archive-contents"));
        if (availText.empty() && !emacs.getLastError().empty()) {
            outError = emacs.getLastError();
            logOut += "[emacs] " + outError + "\n";
        }
        available = parsePackageListText(availText);
    }
    return mergePackageLists(loaded, available, includeAvailable);
}

static void refreshEmacsPackages(EmacsPackageBrowserState& state,
                                 EmacsConnection& emacs,
                                 std::string& logOut) {
    state.lastError.clear();
    state.packages = queryEmacsPackageList(emacs, state.showAvailable, logOut, state.lastError);
    state.needsRefresh = false;
}

static bool loadEmacsPackage(EmacsConnection& emacs,
                             const std::string& packageName,
                             std::string& logOut) {
    if (packageName.empty()) return false;
    std::string cmd = "(require '" + packageName + ")";
    std::string result = emacs.sendCommand(cmd);
    if (!emacs.getLastError().empty() && result == "error") {
        logOut += "[emacs] Failed to load " + packageName + ": " + emacs.getLastError() + "\n";
        return false;
    }
    logOut += "[emacs] Loaded package: " + packageName + "\n";
    return true;
}

static bool emacsPackageMatchesFilter(const EmacsPackageEntry& entry, const std::string& filter) {
    if (filter.empty()) return true;
    std::string hay = toLowerCopyPkg(entry.name + " " + entry.description);
    std::string needle = toLowerCopyPkg(filter);
    return hay.find(needle) != std::string::npos;
}

static bool renderEmacsPackageBrowser(EmacsPackageBrowserState& state,
                                      EmacsConnection& emacs,
                                      std::string& logOut) {
    bool updated = false;
    if (ImGui::Button("Refresh")) {
        state.needsRefresh = true;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Show Available", &state.showAvailable)) {
        state.needsRefresh = true;
    }
    ImGui::SameLine();
    ImGui::InputText("Filter", state.filterBuf, sizeof(state.filterBuf));

    if (state.needsRefresh) {
        refreshEmacsPackages(state, emacs, logOut);
        updated = true;
    }

    ImGui::Separator();
    if (state.packages.empty()) {
        ImGui::TextDisabled("(no packages loaded)");
    } else {
        if (ImGui::BeginTable("##emacsPkgs", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("Package", ImGuiTableColumnFlags_WidthStretch, 0.25f);
            ImGui::TableSetupColumn("Version", ImGuiTableColumnFlags_WidthStretch, 0.15f);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 0.15f);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch, 0.45f);
            ImGui::TableHeadersRow();

            std::string filter = state.filterBuf;
            int row = 0;
            for (int i = 0; i < (int)state.packages.size(); ++i) {
                const auto& entry = state.packages[i];
                if (!emacsPackageMatchesFilter(entry, filter)) continue;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                bool selected = (state.selected == i);
                if (ImGui::Selectable(entry.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    state.selected = i;
                }
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(entry.version.empty() ? "-" : entry.version.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::TextUnformatted(entry.status.empty() ? "-" : entry.status.c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::TextWrapped("%s", entry.description.empty() ? "-" : entry.description.c_str());
                ++row;
            }
            if (row == 0) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextDisabled("(no matches)");
            }
            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::InputText("Load Package", state.loadBuf, sizeof(state.loadBuf));
    ImGui::SameLine();
    std::string loadTarget = state.loadBuf;
    if (loadTarget.empty() && state.selected >= 0 &&
        state.selected < (int)state.packages.size()) {
        loadTarget = state.packages[state.selected].name;
    }
    if (ImGui::Button("Load")) {
        if (loadTarget.empty()) {
            logOut += "[emacs] No package selected.\n";
        } else {
            bool ok = loadEmacsPackage(emacs, loadTarget, logOut);
            if (!ok) {
                logOut += "[emacs] Package not installed: " + loadTarget + "\n";
            }
            state.needsRefresh = true;
            updated = true;
        }
    }
    if (!loadTarget.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", loadTarget.c_str());
    }
    return updated;
}
