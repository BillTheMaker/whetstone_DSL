// Step 158 TDD Test: Workflow recorder
#include "WorkflowRecorder.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    WorkflowRecorder rec;
    rec.startRecording("wf1", "s1");

    json req1 = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "getAST"}};
    json res1 = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", true}};
    rec.record("s1", req1, res1);
    rec.record("s2", req1, res1);

    json wf = rec.stopRecording();
    expect(wf.value("name", "") == "wf1", "workflow name recorded", passed, failed);
    expect(wf["calls"].size() == 1, "recorded calls filtered by session", passed, failed);

    WorkflowRecorder rec2;
    expect(rec2.loadWorkflow(wf), "load workflow succeeds", passed, failed);
    auto replay = rec2.buildReplayRequests();
    expect(replay.size() == 1, "replay request count", passed, failed);
    expect(replay[0].value("method", "") == "getAST",
           "replay request method", passed, failed);

    std::cout << "\n=== Step 158 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
