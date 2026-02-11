#pragma once
#include "imgui.h"
#include "DependencyParser.h"
#include "PackageRegistry.h"
#include "NotificationSystem.h"
#include "VulnerabilityDatabase.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct DependencyFile {
    std::string path;
    std::string label;
};

struct DependencyPanelState {
    std::vector<DependencySpec> deps;
    std::vector<DependencyFile> sources;
    int selected = -1;
    int sourceSelected = -1;
    int editVersionIndex = -1;
    char searchBuf[128] = {};
    char versionBuf[32] = {};
    char editVersionBuf[32] = {};
    char addBuf[128] = {};
    bool showAddPopup = false;
    PackageEcosystem addEcosystem = PackageEcosystem::Python;
    PackageInfo lastLookup;
    std::string lastWorkspaceRoot;
    bool needsIndex = false;
    bool vulnIgnoreLoaded = false;
    std::unordered_map<std::string, std::string> vulnIgnore;
    std::unordered_set<std::string> vulnerableLibraries;
    char vulnIgnoreReason[128] = {};
};

static std::vector<std::string> dependencyFileNames() {
    return {
        "requirements.txt",
        "package.json",
        "Cargo.toml",
        "go.mod",
        "pom.xml",
        "build.gradle",
        "vcpkg.json",
        "CMakeLists.txt",
        "setup.py",
        "pyproject.toml"
    };
}

static std::string sourceBase(const std::string& source) {
    auto pos = source.find(':');
    if (pos == std::string::npos) return source;
    return source.substr(0, pos);
}

static std::string sourceGroup(const std::string& source) {
    auto pos = source.find(':');
    if (pos == std::string::npos) return "";
    return source.substr(pos + 1);
}

static PackageEcosystem ecosystemForSource(const std::string& source) {
    std::string base = sourceBase(source);
    if (base == "requirements.txt" || base == "setup.py" || base == "pyproject.toml")
        return PackageEcosystem::Python;
    if (base == "package.json")
        return PackageEcosystem::Npm;
    if (base == "Cargo.toml")
        return PackageEcosystem::Rust;
    if (base == "go.mod")
        return PackageEcosystem::Go;
    if (base == "pom.xml" || base == "build.gradle")
        return PackageEcosystem::Java;
    return PackageEcosystem::Cpp;
}

static const char* ecosystemLabel(PackageEcosystem eco) {
    switch (eco) {
        case PackageEcosystem::Python: return "Python (PyPI)";
        case PackageEcosystem::Npm: return "JavaScript (npm)";
        case PackageEcosystem::Rust: return "Rust (crates.io)";
        case PackageEcosystem::Go: return "Go (pkg.go.dev)";
        case PackageEcosystem::Java: return "Java (Maven)";
        case PackageEcosystem::Cpp: return "C/C++ (vcpkg/Conan)";
        default: return "Unknown";
    }
}

static std::vector<DependencyFile> discoverDependencyFiles(const std::string& workspaceRoot) {
    std::vector<DependencyFile> out;
    if (workspaceRoot.empty()) return out;
    std::filesystem::path root(workspaceRoot);
    for (const auto& name : dependencyFileNames()) {
        auto path = root / name;
        if (std::filesystem::exists(path)) {
            DependencyFile file;
            file.path = path.string();
            file.label = name;
            out.push_back(file);
        }
    }
    return out;
}

static std::string resolveSourcePath(const std::string& workspaceRoot, const std::string& source) {
    std::filesystem::path root(workspaceRoot);
    std::string base = sourceBase(source);
    return (root / base).string();
}

static std::vector<DependencySpec> depsForSource(const std::vector<DependencySpec>& deps,
                                                 const std::string& base) {
    std::vector<DependencySpec> out;
    for (const auto& dep : deps) {
        if (sourceBase(dep.source) == base) {
            out.push_back(dep);
        }
    }
    return out;
}

static std::string formatRequirementLine(const DependencySpec& dep) {
    if (dep.version.empty()) return dep.name;
    if (!dep.version.empty()) {
        char c = dep.version[0];
        if (c == '>' || c == '<' || c == '=' || c == '~' || c == '^') {
            return dep.name + dep.version;
        }
    }
    return dep.name + "==" + dep.version;
}

static bool writeRequirementsFile(const std::string& path,
                                  const std::vector<DependencySpec>& deps,
                                  NotificationSystem& notifications) {
    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write " + path);
        return false;
    }
    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        out << formatRequirementLine(dep) << "\n";
    }
    notifications.notify(NotificationLevel::Success,
                         "[deps] Wrote " + path);
    return true;
}

static std::string osvEcosystem(PackageEcosystem eco) {
    switch (eco) {
        case PackageEcosystem::Python: return "PyPI";
        case PackageEcosystem::Npm: return "npm";
        case PackageEcosystem::Rust: return "crates.io";
        case PackageEcosystem::Go: return "Go";
        case PackageEcosystem::Java: return "Maven";
        case PackageEcosystem::Cpp: return "OSS-Fuzz";
        default: return "";
    }
}

static std::filesystem::path vulnIgnorePath() {
    const char* home = std::getenv("USERPROFILE");
    if (!home) home = std::getenv("HOME");
    std::filesystem::path base = home ? home : ".";
    return base / ".whetstone" / "vuln_ignore.json";
}

static std::string vulnKey(const std::string& ecosystem, const std::string& package) {
    return ecosystem + ":" + package;
}

static void loadVulnIgnore(DependencyPanelState& state) {
    state.vulnIgnoreLoaded = true;
    state.vulnIgnore.clear();
    auto path = vulnIgnorePath();
    std::ifstream in(path);
    if (!in.is_open()) return;
    nlohmann::json j;
    try { in >> j; } catch (...) { return; }
    if (!j.is_object()) return;
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it.value().is_string()) {
            state.vulnIgnore[it.key()] = it.value().get<std::string>();
        }
    }
}

static void saveVulnIgnore(const DependencyPanelState& state,
                           NotificationSystem& notifications) {
    auto path = vulnIgnorePath();
    std::filesystem::create_directories(path.parent_path());
    nlohmann::json j = nlohmann::json::object();
    for (const auto& [key, reason] : state.vulnIgnore) {
        j[key] = reason;
    }
    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write vuln_ignore.json");
        return;
    }
    out << j.dump(2);
}

static int severityRank(const std::string& severity) {
    if (severity == "Critical") return 3;
    if (severity == "High") return 2;
    if (severity == "Medium") return 1;
    return 0;
}

static ImU32 severityColor(const std::string& severity) {
    if (severity == "Critical" || severity == "High") {
        return IM_COL32(220, 80, 80, 255);
    }
    if (severity == "Medium") {
        return IM_COL32(230, 150, 60, 255);
    }
    return IM_COL32(150, 150, 150, 255);
}

static void drawSecurityBadge(ImDrawList* draw, ImVec2 pos, float size, ImU32 color) {
    ImVec2 top(pos.x, pos.y - size * 0.5f);
    ImVec2 left(pos.x - size * 0.4f, pos.y - size * 0.1f);
    ImVec2 right(pos.x + size * 0.4f, pos.y - size * 0.1f);
    ImVec2 bottom(pos.x, pos.y + size * 0.5f);
    draw->AddTriangleFilled(top, left, right, color);
    draw->AddTriangleFilled(left, right, bottom, color);
}

static int compareVersions(const std::string& a, const std::string& b) {
    auto split = [](const std::string& v) {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : v) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '.') {
                cur.push_back(c);
            } else if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            }
        }
        if (!cur.empty()) parts.push_back(cur);
        return parts;
    };
    auto pa = split(a);
    auto pb = split(b);
    size_t count = std::max(pa.size(), pb.size());
    for (size_t i = 0; i < count; ++i) {
        std::string sa = i < pa.size() ? pa[i] : "0";
        std::string sb = i < pb.size() ? pb[i] : "0";
        bool na = !sa.empty() && std::all_of(sa.begin(), sa.end(), ::isdigit);
        bool nb = !sb.empty() && std::all_of(sb.begin(), sb.end(), ::isdigit);
        if (na && nb) {
            long long ia = std::stoll(sa);
            long long ib = std::stoll(sb);
            if (ia < ib) return -1;
            if (ia > ib) return 1;
        } else {
            if (sa < sb) return -1;
            if (sa > sb) return 1;
        }
    }
    return 0;
}

static std::string bestFixedVersion(const std::vector<VulnerabilityRecord>& records) {
    std::string best;
    for (const auto& record : records) {
        for (const auto& fixed : record.fixedVersions) {
            if (best.empty() || compareVersions(fixed, best) > 0) {
                best = fixed;
            }
        }
    }
    return best;
}

// --- Dependency write-back helpers (extracted) ---
#include "DependencyPanelWrite.h"

static void refreshDependencies(DependencyPanelState& state,
                                const std::string& workspaceRoot,
                                NotificationSystem& notifications,
                                VulnerabilityDatabase& vulnDb) {
    state.deps.clear();
    state.sources = discoverDependencyFiles(workspaceRoot);
    state.selected = -1;
    state.editVersionIndex = -1;
    state.lastWorkspaceRoot = workspaceRoot;
    state.needsIndex = true;
    state.vulnerableLibraries.clear();
    if (workspaceRoot.empty()) return;
    for (const auto& file : state.sources) {
        auto parsed = DependencyParser::parseFile(file.path);
        state.deps.insert(state.deps.end(), parsed.begin(), parsed.end());
    }
    for (const auto& dep : state.deps) {
        PackageEcosystem eco = ecosystemForSource(dep.source);
        std::string osv = osvEcosystem(eco);
        if (!osv.empty() && !dep.name.empty()) {
            vulnDb.trackPackage(osv, dep.name);
            std::string key = vulnKey(osv, dep.name);
            if (state.vulnIgnore.find(key) == state.vulnIgnore.end()) {
                auto vulns = vulnDb.query(osv, dep.name, dep.version);
                if (!vulns.empty()) {
                    state.vulnerableLibraries.insert(dep.name);
                }
            }
        }
    }
    if (!state.sources.empty()) {
        if (state.sourceSelected < 0 || state.sourceSelected >= (int)state.sources.size()) {
            state.sourceSelected = 0;
        }
    } else {
        state.sourceSelected = -1;
    }
    notifications.notify(NotificationLevel::Info,
                         "[deps] Loaded " + std::to_string(state.deps.size()) + " dependencies.");
}

static void renderDependencyPanel(DependencyPanelState& state,
                                  const std::string& workspaceRoot,
                                  NotificationSystem& notifications,
                                  VulnerabilityDatabase& vulnDb) {
    if (workspaceRoot.empty()) {
        ImGui::TextDisabled("Open a workspace to manage dependencies.");
        return;
    }
    if (!state.vulnIgnoreLoaded) {
        loadVulnIgnore(state);
    }
    if (state.lastWorkspaceRoot != workspaceRoot || state.deps.empty()) {
        refreshDependencies(state, workspaceRoot, notifications, vulnDb);
    }

    ImGui::InputText("Search", state.searchBuf, sizeof(state.searchBuf));
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refreshDependencies(state, workspaceRoot, notifications, vulnDb);
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Package")) {
        state.showAddPopup = true;
        state.addBuf[0] = '\0';
        state.versionBuf[0] = '\0';
    }

    std::string targetLabel = "(create requirements.txt)";
    if (state.sourceSelected >= 0 && state.sourceSelected < (int)state.sources.size()) {
        targetLabel = state.sources[state.sourceSelected].label;
    }
    if (ImGui::BeginCombo("Target File", targetLabel.c_str())) {
        for (int i = 0; i < (int)state.sources.size(); ++i) {
            bool selected = (state.sourceSelected == i);
            if (ImGui::Selectable(state.sources[i].label.c_str(), selected)) {
                state.sourceSelected = i;
            }
        }
        if (ImGui::Selectable("(create requirements.txt)", state.sourceSelected == -1)) {
            state.sourceSelected = -1;
        }
        ImGui::EndCombo();
    }

    if (state.showAddPopup) {
        ImGui::OpenPopup("Add Dependency");
        state.showAddPopup = false;
    }
    if (ImGui::BeginPopupModal("Add Dependency", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Package", state.addBuf, sizeof(state.addBuf));
        ImGui::InputText("Version (optional)", state.versionBuf, sizeof(state.versionBuf));
        if (ImGui::BeginCombo("Ecosystem", ecosystemLabel(state.addEcosystem))) {
            for (int i = 0; i < 6; ++i) {
                auto eco = static_cast<PackageEcosystem>(i);
                bool selected = (state.addEcosystem == eco);
                if (ImGui::Selectable(ecosystemLabel(eco), selected)) {
                    state.addEcosystem = eco;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Search Registry")) {
            state.lastLookup = PackageRegistry::query(state.addEcosystem, state.addBuf);
            notifications.notify(NotificationLevel::Info,
                                 "[deps] Registry: " + state.lastLookup.name + " - " +
                                 state.lastLookup.description);
        }
        if (!state.lastLookup.name.empty()) {
            ImGui::TextWrapped("Found: %s", state.lastLookup.name.c_str());
        }
        ImGui::Separator();
        if (ImGui::Button("Add")) {
            if (state.addBuf[0] == '\0') {
                notifications.notify(NotificationLevel::Warning,
                                     "[deps] Add aborted: empty package name.");
            } else {
                DependencySpec dep;
                dep.name = state.addBuf;
                dep.version = state.versionBuf;
                if (state.sourceSelected >= 0 && state.sourceSelected < (int)state.sources.size()) {
                    dep.source = state.sources[state.sourceSelected].label;
                } else {
                    dep.source = "requirements.txt";
                }
                state.deps.push_back(dep);
                if (writeDependenciesForSource(dep.source, workspaceRoot, state.deps, notifications)) {
                    refreshDependencies(state, workspaceRoot, notifications, vulnDb);
                    state.needsIndex = true;
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::BeginChild("##depList", ImVec2(0, 0), false);
    for (int i = 0; i < (int)state.deps.size(); ++i) {
        const auto& dep = state.deps[i];
        std::string label = dep.name;
        if (!dep.version.empty()) label += " (" + dep.version + ")";
        if (!dep.source.empty()) label += " [" + dep.source + "]";
        if (state.searchBuf[0] != '\0') {
            std::string q = state.searchBuf;
            if (label.find(q) == std::string::npos) continue;
        }
        bool selected = (state.selected == i);
        if (ImGui::Selectable(label.c_str(), selected)) {
            state.selected = i;
        }
        PackageEcosystem eco = ecosystemForSource(dep.source);
        std::string osv = osvEcosystem(eco);
        std::string ignoreKey = vulnKey(osv, dep.name);
        if (!osv.empty() && !dep.name.empty() && state.vulnIgnore.find(ignoreKey) == state.vulnIgnore.end()) {
            auto vulns = vulnDb.query(osv, dep.name, dep.version);
            if (!vulns.empty()) {
                int maxRank = 0;
                std::string maxSeverity = "Low";
                for (const auto& v : vulns) {
                    int rank = severityRank(v.severity);
                    if (rank > maxRank) {
                        maxRank = rank;
                        maxSeverity = v.severity;
                    }
                }
                ImVec2 max = ImGui::GetItemRectMax();
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 center(max.x - 10.0f, (min.y + max.y) * 0.5f);
                drawSecurityBadge(ImGui::GetWindowDrawList(), center, 10.0f,
                                  severityColor(maxSeverity));
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Vulnerabilities detected (%s)", maxSeverity.c_str());
                }
            }
        }
    }
    ImGui::EndChild();

    if (state.selected >= 0 && state.selected < (int)state.deps.size()) {
        auto& dep = state.deps[state.selected];
        ImGui::Separator();
        ImGui::Text("Selected: %s", dep.name.c_str());
        if (state.editVersionIndex != state.selected) {
            std::snprintf(state.editVersionBuf, sizeof(state.editVersionBuf), "%s", dep.version.c_str());
            state.editVersionIndex = state.selected;
        }
        ImGui::InputText("Version", state.editVersionBuf, sizeof(state.editVersionBuf));
        if (ImGui::Button("Apply Version")) {
            dep.version = state.editVersionBuf;
            if (writeDependenciesForSource(dep.source, workspaceRoot, state.deps, notifications)) {
                refreshDependencies(state, workspaceRoot, notifications, vulnDb);
                state.needsIndex = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            std::string src = dep.source;
            state.deps.erase(state.deps.begin() + state.selected);
            state.selected = -1;
            if (writeDependenciesForSource(src, workspaceRoot, state.deps, notifications)) {
                refreshDependencies(state, workspaceRoot, notifications, vulnDb);
                state.needsIndex = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Update to Latest")) {
            PackageEcosystem eco = ecosystemForSource(dep.source);
            auto info = PackageRegistry::query(eco, dep.name);
            if (!info.versions.empty()) {
                dep.version = info.versions.back();
                if (writeDependenciesForSource(dep.source, workspaceRoot, state.deps, notifications)) {
                    refreshDependencies(state, workspaceRoot, notifications, vulnDb);
                    state.needsIndex = true;
                }
            } else {
                notifications.notify(NotificationLevel::Warning,
                                     "[deps] No version info for " + dep.name);
            }
        }

        PackageEcosystem eco = ecosystemForSource(dep.source);
        std::string osv = osvEcosystem(eco);
        std::string ignoreKey = vulnKey(osv, dep.name);
        if (!osv.empty() && !dep.name.empty()) {
            ImGui::Separator();
            ImGui::TextUnformatted("Security");
            auto vulns = vulnDb.query(osv, dep.name, dep.version);
            bool ignored = state.vulnIgnore.find(ignoreKey) != state.vulnIgnore.end();
            if (ignored) {
                ImGui::TextDisabled("Ignored: %s", state.vulnIgnore[ignoreKey].c_str());
                if (ImGui::Button("Unignore")) {
                    state.vulnIgnore.erase(ignoreKey);
                    saveVulnIgnore(state, notifications);
                }
            } else if (vulns.empty()) {
                ImGui::TextDisabled("No known vulnerabilities.");
            } else {
                int maxRank = 0;
                std::string maxSeverity = "Low";
                for (const auto& v : vulns) {
                    int rank = severityRank(v.severity);
                    if (rank > maxRank) {
                        maxRank = rank;
                        maxSeverity = v.severity;
                    }
                }
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
                                   "Vulnerabilities found (%s)", maxSeverity.c_str());
                for (const auto& v : vulns) {
                    ImGui::BulletText("%s: %s", v.cveId.c_str(), v.summary.c_str());
                }
                std::string safeVersion = bestFixedVersion(vulns);
                bool hasSafe = !safeVersion.empty();
                if (!hasSafe) ImGui::BeginDisabled();
                if (ImGui::Button("Upgrade to Safe Version")) {
                    dep.version = safeVersion;
                    if (writeDependenciesForSource(dep.source, workspaceRoot, state.deps, notifications)) {
                        refreshDependencies(state, workspaceRoot, notifications, vulnDb);
                        state.needsIndex = true;
                    }
                }
                if (!hasSafe) ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::InputTextWithHint("##vulnIgnoreReason",
                                         "Reason to ignore",
                                         state.vulnIgnoreReason,
                                         sizeof(state.vulnIgnoreReason));
                ImGui::SameLine();
                if (ImGui::Button("Ignore")) {
                    state.vulnIgnore[ignoreKey] = state.vulnIgnoreReason;
                    state.vulnIgnoreReason[0] = '\0';
                    saveVulnIgnore(state, notifications);
                }
            }
        }
    }
}
