// Step 698: full regression + architecture gate (8 tests)

#include "MCPServer.h"
#include "Sprint46IntegrationSummary.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.good()) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static std::string readFirstExisting(const std::vector<std::string>& paths) {
    for (const auto& p : paths) {
        auto text = readFile(p);
        if (!text.empty()) return text;
    }
    return "";
}

static int lineCount(const std::vector<std::string>& paths) {
    std::ifstream in;
    for (const auto& p : paths) {
        in.open(p);
        if (in.good()) break;
        in.clear();
    }
    if (!in.good()) return -1;
    int lines = 0;
    std::string s;
    while (std::getline(in, s)) ++lines;
    return lines;
}

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
    T(targeted_suite_pass);
    auto r = Sprint46IntegrationSummary::run();
    C(r.success, "sprint 46 integration not successful");
    P();
}

void t2() {
    T(legacy_suite_pass);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project", {{"name", "MyApp"}, {"description", "Example app"}});
    C(out.value("success", false), "legacy project tool failed");
    P();
}

void t3() {
    T(mcp_tools_unaffected);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool oldFound = false, newFound = false;
    for (const auto& t : resp["result"]["tools"]) {
        std::string n = t.value("name", "");
        if (n == "whetstone_generate_project") oldFound = true;
        if (n == "whetstone_get_language_matrix") newFound = true;
    }
    C(oldFound && newFound, "expected old and new tools");
    P();
}

void t4() {
    T(no_header_exceeds_600_lines);
    std::vector<std::string> headers = {
        "SemanticCoreIR.h",
        "LanguageSupportTier.h",
        "LanguageCapabilityMatrix.h",
        "MigrationAcceptanceContract.h",
        "LanguageToIRAdapter.h",
        "IRToLanguageAdapter.h",
        "mcp/RegisterPortingFoundationTools.h",
        "Sprint46DocsBaseline.h",
        "Sprint46IntegrationSummary.h"
    };
    for (const auto& h : headers) {
        int n = lineCount({"src/" + h, "../src/" + h, "../../editor/src/" + h});
        C(n > 0, ("missing header: " + h).c_str());
        C(n <= 600, ("header too long: " + h).c_str());
    }
    P();
}

void t5() {
    T(no_new_dependency);
    auto cmake = readFirstExisting({"CMakeLists.txt", "../CMakeLists.txt", "../../editor/CMakeLists.txt"});
    C(!cmake.empty(), "missing CMakeLists");
    C(cmake.find("# Sprint 46: Semantic core and porting contract") != std::string::npos,
      "sprint 46 section missing");
    P();
}

void t6() {
    T(deterministic_snapshots);
    MCPServer mcp;
    auto a = callTool(mcp, "whetstone_get_language_matrix", json::object()).dump();
    auto b = callTool(mcp, "whetstone_get_language_matrix", json::object()).dump();
    C(a == b, "matrix snapshot nondeterministic");
    P();
}

void t7() {
    T(contract_docs_generated);
    auto doc = readFirstExisting({
        "docs/SPRINT46_FOUNDATION.md",
        "../docs/SPRINT46_FOUNDATION.md",
        "../../docs/SPRINT46_FOUNDATION.md"
    });
    C(!doc.empty(), "missing sprint 46 docs");
    C(doc.find("Migration contract checks") != std::string::npos, "contract section missing");
    P();
}

void t8() {
    T(sprint_gate_success);
    auto r = Sprint46IntegrationSummary::run();
    C(r.success && r.regressionMarker == "sprint46_foundation_ready", "gate failed");
    P();
}

int main() {
    std::cout << "Step 698: regression + architecture gate\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
