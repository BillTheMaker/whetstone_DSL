#pragma once
// Step 646: In-editor test runner model

#include <string>
#include <vector>

struct TestRunItem {
    std::string target;
    bool passed = false;
    std::string output;
};

struct TestRunSummary {
    std::vector<TestRunItem> items;
    int passed = 0;
    int failed = 0;
    bool complete = false;
};

class InEditorTestRunnerModel {
public:
    static TestRunSummary run(const std::vector<std::string>& targets) {
        TestRunSummary out;
        for (const auto& t : targets) {
            TestRunItem item;
            item.target = t;
            item.passed = t.find("fail") == std::string::npos;
            item.output = item.passed ? "PASS" : "FAIL";
            out.items.push_back(item);
            if (item.passed) ++out.passed;
            else ++out.failed;
        }
        out.complete = true;
        return out;
    }

    static std::string linkForFailure(const std::string& target) {
        return "editor/tests/" + target + ".cpp";
    }
};
