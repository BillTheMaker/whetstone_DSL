// Step 670: wire whetstone_generate_inference_job (12 tests)

#include "InferenceJobGenerator.h"
#include "MCPServer.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json callTool(MCPServer& mcp, const std::string& name, const json& args) {
    json req = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "tools/call"},
        {"params", {{"name", name}, {"arguments", args}}}
    };
    json resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return json::parse(text);
}

void t1() {
    TEST(goal_required);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job", {{"entropy_score", 1}, {"files", json::array({"a.cpp"})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t2() {
    TEST(entropy_score_required);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job", {{"goal", "extract"}, {"files", json::array({"a.cpp"})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t3() {
    TEST(files_required);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job", {{"goal", "extract"}, {"entropy_score", 1}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t4() {
    TEST(valid_inputs_succeed);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract duplicate handlers"}, {"entropy_score", 3}, {"files", json::array({"main.cpp"})}});
    CHECK(out.value("success", false), "expected success");
    PASS();
}

void t5() {
    TEST(negative_entropy_fails);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract"}, {"entropy_score", -1}, {"files", json::array({"a.cpp"})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t6() {
    TEST(empty_goal_fails);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", ""}, {"entropy_score", 1}, {"files", json::array({"a.cpp"})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t7() {
    TEST(job_type_refactor);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract"}, {"entropy_score", 1}, {"files", json::array({"a.cpp"})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["job"].value("type", "") == "refactor", "wrong type");
    PASS();
}

void t8() {
    TEST(job_goal_matches_input);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract duplicate handlers"}, {"entropy_score", 3}, {"files", json::array({"main.cpp"})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["job"].value("goal", "") == "extract duplicate handlers", "goal mismatch");
    PASS();
}

void t9() {
    TEST(job_context_files_match_input);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract"}, {"entropy_score", 1}, {"files", json::array({"a.cpp", "b.cpp"})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["job"]["context"]["files"].size() == 2, "wrong file count");
    PASS();
}

void t10() {
    TEST(default_bounty_normal);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract"}, {"entropy_score", 1}, {"files", json::array({"a.cpp"})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["job"].value("bounty", "") == "normal", "expected default bounty normal");
    PASS();
}

void t11() {
    TEST(explicit_bounty_high);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract"}, {"entropy_score", 1}, {"files", json::array({"a.cpp"})}, {"bounty", "high"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["job"].value("bounty", "") == "high", "expected bounty high");
    PASS();
}

void t12() {
    TEST(tool_name_matches);
    CHECK(InferenceJobGenerator::toolName() == "whetstone_generate_inference_job", "wrong tool name");
    PASS();
}

int main() {
    std::cout << "Step 670: whetstone_generate_inference_job wiring\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
