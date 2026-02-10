#pragma once
#include "imgui.h"
#include "DependencyParser.h"
#include "PackageRegistry.h"
#include "NotificationSystem.h"
#include <filesystem>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <string>
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

static bool writePackageJson(const std::string& path,
                             const std::vector<DependencySpec>& deps,
                             NotificationSystem& notifications) {
    nlohmann::json j;
    if (std::filesystem::exists(path)) {
        std::ifstream in(path);
        if (in.is_open()) {
            try { in >> j; } catch (...) { j = nlohmann::json::object(); }
        }
    }
    if (!j.is_object()) j = nlohmann::json::object();

    nlohmann::json depsObj = nlohmann::json::object();
    nlohmann::json devObj = nlohmann::json::object();
    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        std::string group = sourceGroup(dep.source);
        std::string version = dep.version.empty() ? "*" : dep.version;
        if (group == "devDependencies") {
            devObj[dep.name] = version;
        } else {
            depsObj[dep.name] = version;
        }
    }
    j["dependencies"] = depsObj;
    j["devDependencies"] = devObj;

    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write " + path);
        return false;
    }
    out << j.dump(2);
    notifications.notify(NotificationLevel::Success,
                         "[deps] Wrote " + path);
    return true;
}

static bool writeCargoToml(const std::string& path,
                           const std::vector<DependencySpec>& deps,
                           NotificationSystem& notifications) {
    std::vector<std::string> prefix;
    bool foundDeps = false;
    if (std::filesystem::exists(path)) {
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) {
            if (line.rfind("[dependencies]", 0) == 0) {
                foundDeps = true;
                break;
            }
            prefix.push_back(line);
        }
    }
    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write " + path);
        return false;
    }
    for (const auto& line : prefix) {
        out << line << "\n";
    }
    if (!foundDeps) out << "\n";
    out << "[dependencies]\n";
    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        std::string version = dep.version.empty() ? "*" : dep.version;
        out << dep.name << " = \"" << version << "\"\n";
    }
    notifications.notify(NotificationLevel::Success,
                         "[deps] Wrote " + path);
    return true;
}

static bool writeGoMod(const std::string& path,
                       const std::vector<DependencySpec>& deps,
                       NotificationSystem& notifications) {
    std::string moduleLine;
    std::string goLine;
    if (std::filesystem::exists(path)) {
        std::ifstream in(path);
        std::string line;
        while (std::getline(in, line)) {
            if (line.rfind("module ", 0) == 0) moduleLine = line;
            if (line.rfind("go ", 0) == 0) goLine = line;
        }
    }
    if (moduleLine.empty()) moduleLine = "module example.com/project";
    if (goLine.empty()) goLine = "go 1.20";

    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write " + path);
        return false;
    }
    out << moduleLine << "\n" << goLine << "\n\n";
    out << "require (\n";
    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        std::string version = dep.version.empty() ? "v0.0.0" : dep.version;
        out << "    " << dep.name << " " << version << "\n";
    }
    out << ")\n";
    notifications.notify(NotificationLevel::Success,
                         "[deps] Wrote " + path);
    return true;
}

static bool writeVcpkgJson(const std::string& path,
                           const std::vector<DependencySpec>& deps,
                           NotificationSystem& notifications) {
    nlohmann::json j;
    if (std::filesystem::exists(path)) {
        std::ifstream in(path);
        if (in.is_open()) {
            try { in >> j; } catch (...) { j = nlohmann::json::object(); }
        }
    }
    if (!j.is_object()) j = nlohmann::json::object();
    nlohmann::json depsArr = nlohmann::json::array();
    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        if (dep.version.empty()) {
            depsArr.push_back(dep.name);
        } else {
            depsArr.push_back({{"name", dep.name}, {"version", dep.version}});
        }
    }
    j["dependencies"] = depsArr;
    std::ofstream out(path);
    if (!out.is_open()) {
        notifications.notify(NotificationLevel::Error,
                             "[deps] Failed to write " + path);
        return false;
    }
    out << j.dump(2);
    notifications.notify(NotificationLevel::Success,
                         "[deps] Wrote " + path);
    return true;
}

static bool writeDependenciesForSource(const std::string& source,
                                       const std::string& workspaceRoot,
                                       const std::vector<DependencySpec>& deps,
                                       NotificationSystem& notifications) {
    if (workspaceRoot.empty()) {
        notifications.notify(NotificationLevel::Warning,
                             "[deps] No workspace root set.");
        return false;
    }
    std::string base = sourceBase(source);
    std::string path = resolveSourcePath(workspaceRoot, base);
    auto filtered = depsForSource(deps, base);

    if (base == "requirements.txt") return writeRequirementsFile(path, filtered, notifications);
    if (base == "package.json") return writePackageJson(path, filtered, notifications);
    if (base == "Cargo.toml") return writeCargoToml(path, filtered, notifications);
    if (base == "go.mod") return writeGoMod(path, filtered, notifications);
    if (base == "vcpkg.json") return writeVcpkgJson(path, filtered, notifications);

    notifications.notify(NotificationLevel::Warning,
                         "[deps] Writeback not implemented for " + base);
    return false;
}

static void refreshDependencies(DependencyPanelState& state,
                                const std::string& workspaceRoot,
                                NotificationSystem& notifications) {
    state.deps.clear();
    state.sources = discoverDependencyFiles(workspaceRoot);
    state.selected = -1;
    state.editVersionIndex = -1;
    state.lastWorkspaceRoot = workspaceRoot;
    state.needsIndex = true;
    if (workspaceRoot.empty()) return;
    for (const auto& file : state.sources) {
        auto parsed = DependencyParser::parseFile(file.path);
        state.deps.insert(state.deps.end(), parsed.begin(), parsed.end());
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
                                  NotificationSystem& notifications) {
    if (workspaceRoot.empty()) {
        ImGui::TextDisabled("Open a workspace to manage dependencies.");
        return;
    }
    if (state.lastWorkspaceRoot != workspaceRoot || state.deps.empty()) {
        refreshDependencies(state, workspaceRoot, notifications);
    }

    ImGui::InputText("Search", state.searchBuf, sizeof(state.searchBuf));
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        refreshDependencies(state, workspaceRoot, notifications);
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
                    refreshDependencies(state, workspaceRoot, notifications);
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
        if (ImGui::Selectable(label.c_str(), state.selected == i)) {
            state.selected = i;
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
                refreshDependencies(state, workspaceRoot, notifications);
                state.needsIndex = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            std::string src = dep.source;
            state.deps.erase(state.deps.begin() + state.selected);
            state.selected = -1;
            if (writeDependenciesForSource(src, workspaceRoot, state.deps, notifications)) {
                refreshDependencies(state, workspaceRoot, notifications);
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
                    refreshDependencies(state, workspaceRoot, notifications);
                    state.needsIndex = true;
                }
            } else {
                notifications.notify(NotificationLevel::Warning,
                                     "[deps] No version info for " + dep.name);
            }
        }
    }
}
