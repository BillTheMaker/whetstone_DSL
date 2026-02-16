// Step 328: Context Assembly + Budget (12 tests)
// Tests context width strategies, budget enforcement, truncation priority,
// import graph inclusion, empty project, token estimation.

#include "ContextAssembler.h"
#include <iostream>
#include <string>
#include <map>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& name, const std::string& bufferId = "main.py",
                          const std::string& contextWidth = "local") {
    WorkItem wi;
    wi.id = "wi-" + name;
    wi.nodeId = "n-" + name;
    wi.nodeName = name;
    wi.nodeType = "Function";
    wi.bufferId = bufferId;
    wi.contextWidth = contextWidth;
    wi.priority = "medium";
    wi.createdAt = workItemTimestamp();
    return wi;
}

static std::map<std::string, BufferInfo> makeBuffers() {
    std::map<std::string, BufferInfo> buffers;

    BufferInfo main;
    main.path = "main.py";
    main.content = "def compute(x):\n    return x * 2\n\ndef validate(data):\n    return True\n";
    main.compactAst = json::array({
        {{"id", "n-compute"}, {"type", "Function"}, {"name", "compute"}},
        {{"id", "n-validate"}, {"type", "Function"}, {"name", "validate"}}
    });
    main.importGraph = json{{"imports", json::array({"utils", "math"})}};
    buffers["main.py"] = main;

    BufferInfo utils;
    utils.path = "utils.py";
    utils.content = "def helper(): pass\n";
    utils.compactAst = json::array({
        {{"id", "n-helper"}, {"type", "Function"}, {"name", "helper"}}
    });
    buffers["utils.py"] = utils;

    return buffers;
}

ContextAssembler assembler;

// 1. Local context contains only target node
void test_local_context() {
    TEST(local_context);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "local");
    auto ctx = assembler.assembleContext(wi, buffers, "local");

    CHECK(ctx.nodeAst.contains("id"), "has node id");
    CHECK(ctx.nodeAst["id"] == "n-compute", "correct node");
    CHECK(ctx.bufferContent.empty(), "no buffer content in local");
    CHECK(ctx.projectSummary.is_null() || ctx.projectSummary.empty(),
          "no project summary in local");
    PASS();
}

// 2. File context includes buffer content
void test_file_context() {
    TEST(file_context);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "file");
    auto ctx = assembler.assembleContext(wi, buffers, "file");

    CHECK(!ctx.bufferContent.empty(), "has buffer content");
    CHECK(ctx.bufferContent.find("def compute") != std::string::npos, "includes source");
    PASS();
}

// 3. Project context includes other buffer summaries
void test_project_context() {
    TEST(project_context);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "project");
    auto ctx = assembler.assembleContext(wi, buffers, "project");

    CHECK(!ctx.bufferContent.empty(), "has buffer content");
    CHECK(!ctx.projectSummary.is_null(), "has project summary");
    CHECK(ctx.projectSummary.contains("utils.py"), "includes other buffer");
    PASS();
}

// 4. Budget truncation activates when over limit
void test_budget_truncation() {
    TEST(budget_truncation);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "file");

    // Very small budget — should truncate buffer content
    auto ctx = assembler.assembleContext(wi, buffers, "file", 30);
    // Node AST takes some tokens, buffer should be truncated
    CHECK(ctx.bufferContent.size() < buffers["main.py"].content.size(),
          "content truncated");
    PASS();
}

// 5. Priority ordering preserved under truncation
void test_priority_under_truncation() {
    TEST(priority_under_truncation);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "project");

    // Budget enough for node + some content but not project summaries
    auto ctx = assembler.assembleContext(wi, buffers, "project", 50);
    CHECK(ctx.nodeAst.contains("id"), "node always included");
    // Project summary may be excluded due to budget
    PASS();
}

// 6. Import graph included in project context
void test_import_graph() {
    TEST(import_graph);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "project");
    auto ctx = assembler.assembleContext(wi, buffers, "project");

    CHECK(!ctx.projectSummary.is_null(), "has project summary");
    CHECK(ctx.projectSummary.contains("importGraph"), "has import graph");
    PASS();
}

// 7. Cross-project context extends project
void test_cross_project() {
    TEST(cross_project);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "cross-project");
    auto ctx = assembler.assembleContext(wi, buffers, "cross-project");

    // Should include everything project does
    CHECK(!ctx.bufferContent.empty(), "has buffer content");
    CHECK(!ctx.projectSummary.is_null(), "has project summary");
    PASS();
}

// 8. Empty project returns minimal context
void test_empty_project() {
    TEST(empty_project);
    std::map<std::string, BufferInfo> empty;
    auto wi = makeItem("compute", "nonexistent.py", "project");
    auto ctx = assembler.assembleContext(wi, empty, "project");

    CHECK(ctx.nodeAst.contains("id"), "has fallback node");
    CHECK(ctx.bufferContent.empty(), "empty content");
    PASS();
}

// 9. Token estimate within reasonable range
void test_token_estimate() {
    TEST(token_estimate);
    json small = {{"a", "hello"}};
    int est = estimateTokens(small);
    CHECK(est > 0, "positive estimate");
    CHECK(est < 20, "reasonable for small json");

    std::string text = "def foo(): return 42";
    int textEst = estimateTokens(text);
    CHECK(textEst == 5, "20 chars / 4 = 5 tokens");
    PASS();
}

// 10. Budget=0 means unlimited
void test_unlimited_budget() {
    TEST(unlimited_budget);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "project");

    auto ctx = assembler.assembleContext(wi, buffers, "project", 0);
    CHECK(!ctx.bufferContent.empty(), "has content with unlimited");
    CHECK(!ctx.projectSummary.is_null(), "has summaries with unlimited");
    PASS();
}

// 11. Siblings included in local context
void test_siblings_in_local() {
    TEST(siblings_in_local);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "local");
    auto ctx = assembler.assembleContext(wi, buffers, "local");

    // Node should have sibling info
    CHECK(ctx.nodeAst.contains("siblings"), "has siblings");
    bool hasValidate = false;
    for (const auto& s : ctx.nodeAst["siblings"]) {
        if (s["name"] == "validate") hasValidate = true;
    }
    CHECK(hasValidate, "validate is a sibling");
    PASS();
}

// 12. Feedback from rejection preserved in context
void test_feedback_preserved() {
    TEST(feedback_preserved);
    auto buffers = makeBuffers();
    auto wi = makeItem("compute", "main.py", "local");
    wi.result.reasoning = "re-routed with feedback: needs error handling";

    auto ctx = assembler.assembleContext(wi, buffers, "local");
    CHECK(!ctx.feedbackFromRejection.empty(), "feedback preserved");
    CHECK(ctx.feedbackFromRejection.find("error handling") != std::string::npos,
          "specific feedback");
    PASS();
}

int main() {
    std::cout << "=== Step 328: Context Assembly + Budget ===\n";
    try {
        test_local_context();
        test_file_context();
        test_project_context();
        test_budget_truncation();
        test_priority_under_truncation();
        test_import_graph();
        test_cross_project();
        test_empty_project();
        test_token_estimate();
        test_unlimited_budget();
        test_siblings_in_local();
        test_feedback_preserved();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
