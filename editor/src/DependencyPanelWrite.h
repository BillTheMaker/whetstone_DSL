#pragma once
// --- DependencyPanelWrite.h ---
// Extracted from DependencyPanel.h (Sprint 8 cleanup).
// Dependency write-back helpers.

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
