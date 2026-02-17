#pragma once
// Step 579: Taskitem Generator v2

#include "ScopeMilestoneDecomposer.h"

#include <map>
#include <string>
#include <vector>

struct GeneratedTaskitem {
    std::string taskId;
    std::string title;
    std::string milestoneId;
    std::vector<std::string> dependencyTaskIds;
    std::vector<std::string> prerequisiteOps;
    bool queueReady = false;
};

class TaskitemGeneratorV2 {
public:
    static bool generate(const DecomposedScopePlan& plan,
                         std::vector<GeneratedTaskitem>* outTasks,
                         std::string* error) {
        if (!outTasks || !error) return false;
        error->clear();
        outTasks->clear();
        if (plan.milestones.empty()) {
            *error = "plan_empty";
            return false;
        }

        std::map<std::string, std::string> workstreamToTask;
        int nextId = 1;
        for (std::size_t mi = 0; mi < plan.milestones.size(); ++mi) {
            const auto& milestone = plan.milestones[mi];
            for (const auto& workstream : milestone.workstreams) {
                GeneratedTaskitem task;
                task.taskId = "task-" + std::to_string(nextId++);
                task.title = workstream.title;
                task.milestoneId = milestone.milestoneId;
                task.prerequisiteOps = inferPrerequisiteOps(workstream);
                task.queueReady = !task.prerequisiteOps.empty() && !workstream.requirementIds.empty();
                if (mi > 0 && !outTasks->empty()) task.dependencyTaskIds.push_back(outTasks->back().taskId);
                workstreamToTask[workstream.workstreamId] = task.taskId;
                outTasks->push_back(task);
            }
        }

        if (outTasks->empty()) {
            *error = "tasks_empty";
            return false;
        }
        return true;
    }

private:
    static std::vector<std::string> inferPrerequisiteOps(const DecomposedWorkstream& workstream) {
        std::vector<std::string> ops;
        ops.push_back("validate-intake");
        if (workstream.uncertaintyScore >= 35) ops.push_back("architect-review");
        if (workstream.title.find("Execution") != std::string::npos) ops.push_back("resolve-dependencies");
        if (workstream.title.find("Review") != std::string::npos) ops.push_back("manual-approval");
        return ops;
    }
};
