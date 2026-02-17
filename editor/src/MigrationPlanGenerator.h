#pragma once

// Step 444: Migration Plan Generator — analyzes a multi-file project,
// identifies migration units, determines order, and annotates with effort/risk/routing.

#include <algorithm>
#include <string>
#include <vector>

enum class MigrationRouting { Deterministic, LLM, Human };

struct MigrationUnit {
    std::string id;
    std::string filePath;
    std::string moduleName;
    std::string sourceLanguage;
    std::string targetLanguage;
    int effort = 1;         // 1=low, 2=medium, 3=high
    int risk = 1;           // 1=low, 2=medium, 3=high
    MigrationRouting routing = MigrationRouting::LLM;
    std::vector<std::string> dependsOn;    // unit IDs this depends on
    std::vector<std::string> exports;       // public API symbols
    std::vector<std::string> imports;       // symbols imported from other units
    int phase = 0;                         // migration phase (0 = first)
};

struct MigrationPlan {
    std::string projectName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<MigrationUnit> units;
    int phaseCount = 0;

    bool hasUnit(const std::string& filePath) const {
        for (const auto& u : units)
            if (u.filePath == filePath) return true;
        return false;
    }

    MigrationUnit unitByPath(const std::string& filePath) const {
        for (const auto& u : units)
            if (u.filePath == filePath) return u;
        return {};
    }

    std::vector<MigrationUnit> unitsInPhase(int phase) const {
        std::vector<MigrationUnit> out;
        for (const auto& u : units)
            if (u.phase == phase) out.push_back(u);
        return out;
    }

    std::vector<MigrationUnit> orderedUnits() const {
        auto sorted = units;
        std::sort(sorted.begin(), sorted.end(),
                  [](const MigrationUnit& a, const MigrationUnit& b) {
                      if (a.phase != b.phase) return a.phase < b.phase;
                      return a.risk < b.risk;  // lower risk first within phase
                  });
        return sorted;
    }

    int countByRouting(MigrationRouting r) const {
        int n = 0;
        for (const auto& u : units)
            if (u.routing == r) ++n;
        return n;
    }
};

struct FileInfo {
    std::string filePath;
    std::string source;
    std::vector<std::string> exports;   // public function/type names
    std::vector<std::string> imports;   // referenced external symbols
};

class MigrationPlanGenerator {
public:
    static MigrationPlan generate(const std::string& projectName,
                                  const std::vector<FileInfo>& files,
                                  const std::string& sourceLanguage,
                                  const std::string& targetLanguage) {
        MigrationPlan plan;
        plan.projectName = projectName;
        plan.sourceLanguage = sourceLanguage;
        plan.targetLanguage = targetLanguage;

        // Create migration units from files
        for (size_t i = 0; i < files.size(); ++i) {
            MigrationUnit unit;
            unit.id = "unit-" + std::to_string(i);
            unit.filePath = files[i].filePath;
            unit.moduleName = extractModuleName(files[i].filePath);
            unit.sourceLanguage = sourceLanguage;
            unit.targetLanguage = targetLanguage;
            unit.exports = files[i].exports;
            unit.imports = files[i].imports;
            plan.units.push_back(std::move(unit));
        }

        // Resolve dependencies between units
        resolveDependencies(plan);

        // Assign phases (topological order — leaves first)
        assignPhases(plan);

        // Classify effort/risk/routing
        for (auto& unit : plan.units)
            classifyUnit(unit, files);

        return plan;
    }

private:
    static std::string extractModuleName(const std::string& path) {
        size_t slash = path.rfind('/');
        size_t dot = path.rfind('.');
        size_t start = (slash == std::string::npos) ? 0 : slash + 1;
        size_t end = (dot == std::string::npos) ? path.size() : dot;
        return path.substr(start, end - start);
    }

    static void resolveDependencies(MigrationPlan& plan) {
        // For each unit, find which other units provide its imports
        for (auto& unit : plan.units) {
            for (const auto& imp : unit.imports) {
                for (const auto& other : plan.units) {
                    if (other.id == unit.id) continue;
                    for (const auto& exp : other.exports) {
                        if (exp == imp) {
                            unit.dependsOn.push_back(other.id);
                        }
                    }
                }
            }
            // Deduplicate
            std::sort(unit.dependsOn.begin(), unit.dependsOn.end());
            unit.dependsOn.erase(
                std::unique(unit.dependsOn.begin(), unit.dependsOn.end()),
                unit.dependsOn.end());
        }
    }

    static void assignPhases(MigrationPlan& plan) {
        // Leaf modules (no dependencies) go in phase 0
        // Units depending on phase-N units go in phase N+1
        int maxPhase = 0;
        bool changed = true;
        // Initialize all to phase 0
        for (auto& u : plan.units) u.phase = 0;

        while (changed) {
            changed = false;
            for (auto& unit : plan.units) {
                for (const auto& depId : unit.dependsOn) {
                    for (const auto& dep : plan.units) {
                        if (dep.id == depId && dep.phase >= unit.phase) {
                            unit.phase = dep.phase + 1;
                            maxPhase = std::max(maxPhase, unit.phase);
                            changed = true;
                        }
                    }
                }
            }
        }
        plan.phaseCount = maxPhase + 1;
    }

    static void classifyUnit(MigrationUnit& unit,
                              const std::vector<FileInfo>& files) {
        // Find corresponding file
        const FileInfo* fi = nullptr;
        for (const auto& f : files)
            if (f.filePath == unit.filePath) { fi = &f; break; }
        if (!fi) return;

        int lineCount = countLines(fi->source);
        int exportCount = (int)fi->exports.size();

        // Effort based on size
        if (lineCount > 200) unit.effort = 3;
        else if (lineCount > 50) unit.effort = 2;
        else unit.effort = 1;

        // Risk based on API surface + dependencies
        if (exportCount > 5 || !unit.dependsOn.empty()) unit.risk = 3;
        else if (exportCount > 2) unit.risk = 2;
        else unit.risk = 1;

        // Routing
        if (unit.effort == 1 && unit.risk == 1)
            unit.routing = MigrationRouting::Deterministic;
        else if (unit.effort == 3 || unit.risk == 3)
            unit.routing = MigrationRouting::Human;
        else
            unit.routing = MigrationRouting::LLM;
    }

    static int countLines(const std::string& src) {
        int n = 1;
        for (char c : src) if (c == '\n') ++n;
        return n;
    }
};
