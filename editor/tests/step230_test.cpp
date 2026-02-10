// Step 230: Enhanced session recorder.
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
    WorkflowRecorder::RecordingConfig cfg;
    cfg.recordAllSessions = true;
    cfg.recordControlMethods = true;
    cfg.recordEditorEvents = true;
    cfg.autoRecording = true;

    json meta = {{"editorVersion", "0.3.0"}, {"os", "test"}};
    rec.startRecording("session", "", cfg, meta);

    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "getAST"}};
    json res = {{"jsonrpc", "2.0"}, {"id", 1}, {"result", true}};
    rec.record("s1", req, res);
    rec.recordEvent("file_opened", {"path", "main.py"});

    json out = rec.stopRecording();
    expect(out.value("name", "") == "session", "recording name", passed, failed);
    expect(out["metadata"].value("editorVersion", "") == "0.3.0",
           "metadata preserved", passed, failed);
    expect(out["calls"].size() == 1, "records rpc calls", passed, failed);
    expect(out["calls"][0].value("sessionId", "") == "s1", "session id captured", passed, failed);
    expect(out["events"].size() == 1, "records editor events", passed, failed);
    expect(out["events"][0].value("type", "") == "file_opened", "event type captured", passed, failed);
    expect(out["events"][0].value("timestamp", "").find('.') != std::string::npos,
           "timestamp includes milliseconds", passed, failed);

    std::cout << "\n=== Step 230 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
