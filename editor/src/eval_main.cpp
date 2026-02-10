#include <iostream>
#include <string>
#include "EvalHarness.h"

static std::string getArg(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == key) return argv[i + 1];
    }
    return "";
}

int main(int argc, char** argv) {
    std::string tasksDir = getArg(argc, argv, "--tasks");
    std::string tracePath = getArg(argc, argv, "--trace");
    std::string reportPath = getArg(argc, argv, "--report");

    if (tasksDir.empty()) {
        std::cerr << "Usage: whetstone_eval --tasks <dir> [--trace file.jsonl] [--report out.json]\n";
        return 1;
    }

    auto tasks = EvalRunner::loadTasksFromDir(tasksDir);
    std::vector<Trace> traces;
    if (!tracePath.empty()) {
        traces = EvalRunner::loadTracesJsonl(tracePath);
    }

    json report = EvalRunner::run(tasks, traces);
    if (!reportPath.empty()) {
        std::ofstream out(reportPath);
        if (!out.is_open()) {
            std::cerr << "Failed to write report: " << reportPath << "\n";
            return 2;
        }
        out << report.dump(2);
    } else {
        std::cout << report.dump(2) << "\n";
    }
    return 0;
}
