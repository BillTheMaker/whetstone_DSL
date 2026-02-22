#pragma once
// Step 1471: repro command runner facade.

#include <string>

#include <nlohmann/json.hpp>

struct ReproRunResult {
    std::string command;
    int exitCode = 0;
    std::string output;
    bool mocked = true;
};

class ReproCommandRunner {
public:
    static ReproRunResult run(const std::string& command) {
        ReproRunResult r;
        r.command = command;
        if (command.rfind("mock:", 0) == 0) {
            r.output = command.substr(5);
            r.exitCode = (r.output.find("ok") != std::string::npos) ? 0 : 1;
            r.mocked = true;
            return r;
        }
        r.output = "runner_non_mock_mode_disabled";
        r.exitCode = 1;
        r.mocked = false;
        return r;
    }

    static nlohmann::json toJson(const ReproRunResult& r) {
        return {{"command", r.command}, {"exit_code", r.exitCode}, {"output", r.output}, {"mocked", r.mocked}};
    }
};
