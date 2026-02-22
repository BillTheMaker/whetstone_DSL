// Step 826: whetstone_review_porting_decision MCP tool (8 tests)
#include "MCPServer.h"
#include <iostream>

static int p=0,f=0;
#define T(n) { std::cout << "  " << #n << "... "; }
#define P() { std::cout << "PASS\n"; ++p; }
#define F(m) { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m) if (!(c)) { F(m); return; }

static nlohmann::json getResult(const nlohmann::json& r) {
    return nlohmann::json::parse(r["result"]["content"][0]["text"].get<std::string>());
}

void t1() {
    T(tool_registered);
    MCPServer mcp;
    auto list = mcp.handleRequest({{"jsonrpc","2.0"},{"id",1},{"method","tools/list"}});
    bool found = false;
    for (const auto& t : list["result"]["tools"])
        if (t.value("name","") == "whetstone_review_porting_decision") found = true;
    C(found, "tool_not_found");
    P();
}
void t2() {
    T(record_approved_decision);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",2},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-01"},{"reviewer","alice"},{"decision","approved"},{"rationale","looks good"}}}}}});
    auto res = getResult(r);
    C(res["success"].get<bool>(), "success");
    P();
}
void t3() {
    T(missing_issue_id);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",3},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"reviewer","alice"},{"decision","approved"},{"rationale","ok"}}}}}});
    auto res = getResult(r);
    C(!res["success"].get<bool>(), "fail");
    P();
}
void t4() {
    T(missing_reviewer);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",4},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-02"},{"decision","approved"},{"rationale","ok"}}}}}});
    auto res = getResult(r);
    C(!res["success"].get<bool>(), "fail");
    P();
}
void t5() {
    T(missing_decision);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",5},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-03"},{"reviewer","bob"},{"rationale","ok"}}}}}});
    auto res = getResult(r);
    C(!res["success"].get<bool>(), "fail");
    P();
}
void t6() {
    T(with_waiver_scope);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",6},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-04"},{"reviewer","carol"},{"decision","approved"},
            {"rationale","ok"},{"waiver_scope","temporary"}}}}}});
    auto res = getResult(r);
    C(res["success"].get<bool>(), "success");
    P();
}
void t7() {
    T(ledger_summary_in_response);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",7},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-05"},{"reviewer","alice"},{"decision","rejected"},{"rationale","fail"}}}}}});
    auto res = getResult(r);
    C(res.contains("ledger_summary"), "ledger");
    P();
}
void t8() {
    T(decision_in_response);
    MCPServer mcp;
    auto r = mcp.handleRequest({{"jsonrpc","2.0"},{"id",8},{"method","tools/call"},
        {"params",{{"name","whetstone_review_porting_decision"},{"arguments",{
            {"issue_id","ISS-06"},{"reviewer","alice"},{"decision","modified"},{"rationale","tweak"}}}}}});
    auto res = getResult(r);
    C(res.value("decision","") == "modified", "decision");
    P();
}

int main() {
    std::cout << "Step 826: whetstone_review_porting_decision MCP tool\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p+f) << " passed\n";
    return f ? 1 : 0;
}
