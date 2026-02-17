#pragma once

// Step 447: Migration Execution Integration — hooks migration plans into the
// orchestration engine with cross-unit dependencies, rollback annotations,
// and progressive (partial) migration support.

#include "MigrationPlanGenerator.h"
#include "APIBoundaryPreserver.h"
#include "MigrationTestGenerator.h"

#include <string>
#include <vector>

enum class MigrationStatus { Pending, InProgress, Completed, Failed, Skipped };

struct MigrationWorkItem {
    std::string id;
    std::string unitId;
    std::string filePath;
    std::string title;
    MigrationRouting routing = MigrationRouting::LLM;
    MigrationStatus status = MigrationStatus::Pending;
    std::vector<std::string> dependsOn;
    bool requiresHumanReview = false;
    std::string rollbackAnnotation;
};

struct PartialMigrationState {
    std::string unitId;
    std::string filePath;
    bool migrated = false;
    bool hasFfiBoundary = false;
    std::string ffiAnnotation;
};

struct MigrationExecution {
    std::string projectName;
    std::string sourceLanguage;
    std::string targetLanguage;
    std::vector<MigrationWorkItem> workItems;
    std::vector<PartialMigrationState> partialState;
    int currentPhase = 0;

    int countByStatus(MigrationStatus s) const {
        int n = 0;
        for (const auto& w : workItems)
            if (w.status == s) ++n;
        return n;
    }

    bool isUnitMigrated(const std::string& unitId) const {
        for (const auto& p : partialState)
            if (p.unitId == unitId) return p.migrated;
        return false;
    }

    std::vector<MigrationWorkItem> readyItems() const {
        std::vector<MigrationWorkItem> out;
        for (const auto& w : workItems) {
            if (w.status != MigrationStatus::Pending) continue;
            bool blocked = false;
            for (const auto& dep : w.dependsOn) {
                for (const auto& other : workItems) {
                    if (other.id == dep && other.status != MigrationStatus::Completed)
                        blocked = true;
                }
            }
            if (!blocked) out.push_back(w);
        }
        return out;
    }

    int countFfiBoundaries() const {
        int n = 0;
        for (const auto& p : partialState)
            if (p.hasFfiBoundary) ++n;
        return n;
    }
};

class MigrationExecutor {
public:
    static MigrationExecution createExecution(const MigrationPlan& plan,
                                               const std::vector<FileInfo>& files) {
        MigrationExecution exec;
        exec.projectName = plan.projectName;
        exec.sourceLanguage = plan.sourceLanguage;
        exec.targetLanguage = plan.targetLanguage;
        exec.currentPhase = 0;

        int idx = 0;
        auto ordered = plan.orderedUnits();
        for (const auto& unit : ordered) {
            MigrationWorkItem item;
            item.id = "mig-" + std::to_string(idx++);
            item.unitId = unit.id;
            item.filePath = unit.filePath;
            item.title = "Migrate " + unit.moduleName + " (" +
                         unit.sourceLanguage + " -> " + unit.targetLanguage + ")";
            item.routing = unit.routing;
            item.requiresHumanReview = (unit.risk >= 3 || unit.routing == MigrationRouting::Human);
            item.rollbackAnnotation = "@Rollback(revert_to=\"" + unit.filePath + "\")";

            // Map unit dependencies to work item dependencies
            for (const auto& depId : unit.dependsOn) {
                for (const auto& wi : exec.workItems) {
                    if (wi.unitId == depId)
                        item.dependsOn.push_back(wi.id);
                }
            }

            exec.workItems.push_back(std::move(item));
        }

        // Initialize partial migration state
        for (const auto& unit : plan.units) {
            PartialMigrationState ps;
            ps.unitId = unit.id;
            ps.filePath = unit.filePath;
            ps.migrated = false;
            ps.hasFfiBoundary = false;
            exec.partialState.push_back(std::move(ps));
        }

        return exec;
    }

    // Mark a unit as completed and update FFI boundaries
    static void completeUnit(MigrationExecution& exec, const std::string& workItemId) {
        for (auto& w : exec.workItems) {
            if (w.id == workItemId) {
                w.status = MigrationStatus::Completed;
                // Update partial state
                for (auto& ps : exec.partialState) {
                    if (ps.unitId == w.unitId) {
                        ps.migrated = true;
                    }
                }
            }
        }
        // Update FFI boundaries for mixed-language state
        updateFfiBoundaries(exec);
    }

    // Mark a unit as failed (requires human review)
    static void failUnit(MigrationExecution& exec, const std::string& workItemId) {
        for (auto& w : exec.workItems) {
            if (w.id == workItemId) {
                w.status = MigrationStatus::Failed;
                w.requiresHumanReview = true;
            }
        }
    }

    // Check if execution is in a partial (mixed-language) state
    static bool isPartialMigration(const MigrationExecution& exec) {
        bool hasMigrated = false, hasUnmigrated = false;
        for (const auto& ps : exec.partialState) {
            if (ps.migrated) hasMigrated = true;
            else hasUnmigrated = true;
        }
        return hasMigrated && hasUnmigrated;
    }

private:
    static void updateFfiBoundaries(MigrationExecution& exec) {
        for (auto& ps : exec.partialState) {
            if (!ps.migrated) {
                // Check if any migrated unit depends on this unmigrated one
                // or this unmigrated one depends on a migrated one
                bool needsFfi = false;
                for (const auto& other : exec.partialState) {
                    if (other.unitId == ps.unitId) continue;
                    if (other.migrated != ps.migrated) {
                        // Check if they have a dependency relationship
                        for (const auto& w : exec.workItems) {
                            if (w.unitId == ps.unitId || w.unitId == other.unitId) {
                                for (const auto& dep : w.dependsOn) {
                                    for (const auto& w2 : exec.workItems) {
                                        if (w2.id == dep &&
                                            (w2.unitId == ps.unitId || w2.unitId == other.unitId))
                                            needsFfi = true;
                                    }
                                }
                            }
                        }
                    }
                }
                ps.hasFfiBoundary = needsFfi;
                if (needsFfi)
                    ps.ffiAnnotation = "@FFI(boundary=" + exec.sourceLanguage +
                                       "/" + exec.targetLanguage + ")";
            } else {
                ps.hasFfiBoundary = false;
                ps.ffiAnnotation = "";
            }
        }
    }
};
