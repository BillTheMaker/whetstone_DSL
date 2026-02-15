// Step 323: Workflow Sidecar Persistence (12 tests)
// Tests save/load roundtrip, field preservation, history, missing file,
// directory creation, delete, multiple workflows, results, empty workflow.

#include "WorkflowPersistence.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string testDir;

static void setupTestDir() {
    testDir = "/tmp/whetstone_step323_test_" + std::to_string(getpid());
    fs::create_directories(testDir);
}

static void cleanupTestDir() {
    fs::remove_all(testDir);
}

// Helper: create a WorkflowState with some items
static WorkflowState makeWorkflow(const std::string& name, int itemCount) {
    WorkflowState ws(name);
    for (int i = 0; i < itemCount; ++i) {
        WorkItem wi;
        wi.id = "wi-" + std::to_string(i);
        wi.nodeId = "node_" + std::to_string(i);
        wi.nodeName = "func_" + std::to_string(i);
        wi.nodeType = "Function";
        wi.bufferId = "buf-1";
        wi.contextWidth = "local";
        wi.workerType = "llm";
        wi.priority = "medium";
        wi.status = WI_READY;
        wi.createdAt = workItemTimestamp();
        ws.queue.enqueue(wi);
    }
    return ws;
}

// 1. Save/load roundtrip
void test_save_load_roundtrip() {
    TEST(save_load_roundtrip);
    auto ws = makeWorkflow("proj1", 3);
    auto result = saveWorkflow(testDir, ws);
    CHECK(result.success, "save succeeded");
    CHECK(result.itemCount == 3, "itemCount=3");
    CHECK(result.bytesWritten > 0, "bytes written");

    auto loaded = loadWorkflow(testDir, "proj1");
    CHECK(loaded.has_value(), "load succeeded");
    CHECK(loaded->projectName == "proj1", "projectName");
    CHECK(loaded->queue.size() == 3, "3 items loaded");
    PASS();
}

// 2. All work item fields preserved
void test_fields_preserved() {
    TEST(fields_preserved);
    WorkflowState ws("fields");
    WorkItem wi;
    wi.id = "wi-field";
    wi.nodeId = "n42";
    wi.nodeName = "computeHash";
    wi.nodeType = "Function";
    wi.bufferId = "buf-main";
    wi.contextWidth = "module";
    wi.workerType = "deterministic";
    wi.reviewRequired = true;
    wi.priority = "critical";
    wi.dependencies = {"wi-0", "wi-1"};
    wi.status = WI_ASSIGNED;
    wi.assignee = "worker-7";
    wi.createdAt = "2026-01-01T00:00:00Z";
    wi.assignedAt = "2026-01-01T00:01:00Z";
    ws.queue.enqueue(wi);

    saveWorkflow(testDir, ws);
    auto loaded = loadWorkflow(testDir, "fields");
    CHECK(loaded.has_value(), "loaded");

    auto item = loaded->queue.getItem("wi-field");
    CHECK(item.has_value(), "item found");
    CHECK(item->nodeId == "n42", "nodeId");
    CHECK(item->nodeName == "computeHash", "nodeName");
    CHECK(item->contextWidth == "module", "contextWidth");
    CHECK(item->workerType == "deterministic", "workerType");
    CHECK(item->reviewRequired == true, "reviewRequired");
    CHECK(item->priority == "critical", "priority");
    CHECK(item->assignee == "worker-7", "assignee");
    CHECK(item->assignedAt == "2026-01-01T00:01:00Z", "assignedAt");
    PASS();
}

// 3. Status history preserved
void test_history_preserved() {
    TEST(history_preserved);
    WorkflowState ws("hist");
    ws.recordChange("wi-1", "", "pending", "system", "created");
    ws.recordChange("wi-1", "pending", "ready", "system", "deps met");
    ws.recordChange("wi-1", "ready", "assigned", "routing-engine");

    saveWorkflow(testDir, ws);
    auto loaded = loadWorkflow(testDir, "hist");
    CHECK(loaded.has_value(), "loaded");

    auto h = loaded->getHistory("wi-1");
    CHECK(h.size() == 3, "3 history entries, got " + std::to_string(h.size()));
    CHECK(h[0].toStatus == "pending", "first=pending");
    CHECK(h[1].actor == "system", "second actor=system");
    CHECK(h[2].toStatus == "assigned", "third=assigned");
    PASS();
}

// 4. Missing file returns nullopt
void test_missing_file() {
    TEST(missing_file);
    auto loaded = loadWorkflow(testDir, "nonexistent");
    CHECK(!loaded.has_value(), "nullopt for missing file");
    PASS();
}

// 5. Save creates parent directories
void test_creates_directories() {
    TEST(creates_directories);
    std::string subDir = testDir + "/nested/workspace";
    auto ws = makeWorkflow("dirtest", 1);
    auto result = saveWorkflow(subDir, ws);
    CHECK(result.success, "save succeeded with new dirs");
    CHECK(fs::exists(subDir + "/.whetstone/dirtest.workflow.json"), "file exists");
    PASS();
}

// 6. Delete removes file
void test_delete_workflow() {
    TEST(delete_workflow);
    auto ws = makeWorkflow("deleteme", 1);
    saveWorkflow(testDir, ws);

    std::string path = workflowSidecarPath(testDir, "deleteme");
    CHECK(fs::exists(path), "file exists before delete");

    bool deleted = deleteWorkflow(testDir, "deleteme");
    CHECK(deleted, "delete returned true");
    CHECK(!fs::exists(path), "file removed");
    PASS();
}

// 7. Multiple workflows in same workspace
void test_multiple_workflows() {
    TEST(multiple_workflows);
    auto ws1 = makeWorkflow("alpha", 2);
    auto ws2 = makeWorkflow("beta", 4);
    saveWorkflow(testDir, ws1);
    saveWorkflow(testDir, ws2);

    auto l1 = loadWorkflow(testDir, "alpha");
    auto l2 = loadWorkflow(testDir, "beta");
    CHECK(l1.has_value(), "alpha loaded");
    CHECK(l2.has_value(), "beta loaded");
    CHECK(l1->queue.size() == 2, "alpha has 2 items");
    CHECK(l2->queue.size() == 4, "beta has 4 items");
    CHECK(l1->projectName == "alpha", "alpha name");
    CHECK(l2->projectName == "beta", "beta name");
    PASS();
}

// 8. Workflow with results attached
void test_results_preserved() {
    TEST(results_preserved);
    WorkflowState ws("results");
    WorkItem wi;
    wi.id = "wi-res";
    wi.nodeId = "n1";
    wi.nodeName = "gen";
    wi.nodeType = "Function";
    wi.priority = "medium";
    wi.status = WI_COMPLETE;
    wi.createdAt = workItemTimestamp();
    wi.result.generatedCode = "def gen(): return 42";
    wi.result.confidence = 0.95f;
    wi.result.tokensBudget = 4000;
    wi.result.tokensGenerated = 120;
    wi.result.reasoning = "trivial pure function";
    wi.result.diagnostics = {json{{"level", "info"}, {"msg", "ok"}}};
    wi.result.astJson = json{{"type", "Function"}, {"name", "gen"}};
    ws.queue.enqueue(wi);

    saveWorkflow(testDir, ws);
    auto loaded = loadWorkflow(testDir, "results");
    CHECK(loaded.has_value(), "loaded");

    auto item = loaded->queue.getItem("wi-res");
    CHECK(item.has_value(), "item found");
    CHECK(item->result.generatedCode == "def gen(): return 42", "code");
    CHECK(item->result.confidence > 0.94f, "confidence");
    CHECK(item->result.tokensBudget == 4000, "budget");
    CHECK(item->result.tokensGenerated == 120, "generated");
    CHECK(item->result.reasoning == "trivial pure function", "reasoning");
    CHECK(item->result.diagnostics.size() == 1, "diagnostics");
    CHECK(item->result.astJson.contains("name"), "astJson");
    PASS();
}

// 9. Empty workflow save/load
void test_empty_workflow() {
    TEST(empty_workflow);
    WorkflowState ws("empty");
    auto result = saveWorkflow(testDir, ws);
    CHECK(result.success, "save empty succeeded");
    CHECK(result.itemCount == 0, "0 items");

    auto loaded = loadWorkflow(testDir, "empty");
    CHECK(loaded.has_value(), "loaded empty");
    CHECK(loaded->queue.size() == 0, "0 items loaded");
    CHECK(loaded->projectName == "empty", "name preserved");
    PASS();
}

// 10. Sidecar path format
void test_sidecar_path() {
    TEST(sidecar_path);
    auto path = workflowSidecarPath("/home/user/project", "myapp");
    CHECK(path == "/home/user/project/.whetstone/myapp.workflow.json", "path format: " + path);
    PASS();
}

// 11. Overwrite existing workflow
void test_overwrite() {
    TEST(overwrite);
    auto ws1 = makeWorkflow("overwrite", 2);
    saveWorkflow(testDir, ws1);

    auto ws2 = makeWorkflow("overwrite", 5);
    saveWorkflow(testDir, ws2);

    auto loaded = loadWorkflow(testDir, "overwrite");
    CHECK(loaded.has_value(), "loaded");
    CHECK(loaded->queue.size() == 5, "5 items (overwritten)");
    PASS();
}

// 12. Delete nonexistent returns false
void test_delete_nonexistent() {
    TEST(delete_nonexistent);
    bool deleted = deleteWorkflow(testDir, "ghost");
    CHECK(!deleted, "delete nonexistent returns false");
    PASS();
}

int main() {
    std::cout << "=== Step 323: Workflow Sidecar Persistence ===\n";
    setupTestDir();
    try {
        test_save_load_roundtrip();
        test_fields_preserved();
        test_history_preserved();
        test_missing_file();
        test_creates_directories();
        test_delete_workflow();
        test_multiple_workflows();
        test_results_preserved();
        test_empty_workflow();
        test_sidecar_path();
        test_overwrite();
        test_delete_nonexistent();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    cleanupTestDir();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
