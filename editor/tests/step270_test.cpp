// Step 270 TDD Test: Annotation Prompt Templates
//
// MCP prompt templates that guide a frontier model through annotating
// a codebase. Tests prompt registration, static analysis hints in
// getUnannotatedNodes, and prompt message content.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>

static void expect(bool cond, const std::string& name,
                   int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1)
                  << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1)
                  << " FAIL: " << name << "\n";
        ++failed;
    }
}

static json rpc(HeadlessEditorState& state, const std::string& session,
                 const std::string& method,
                 json params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", method}, {"params", params}};
    return state.processAgentRequest(request, session);
}

int main() {
    int passed = 0;
    int failed = 0;

    // ---------------------------------------------------------------
    //  Test 1: annotate_intent prompt registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& p : mcp.getPrompts()) {
            if (p.name == "annotate_intent") found = true;
        }
        expect(found, "annotate_intent prompt registered", passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: annotate_complexity prompt registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& p : mcp.getPrompts()) {
            if (p.name == "annotate_complexity") found = true;
        }
        expect(found, "annotate_complexity prompt registered", passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: annotate_risk prompt registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& p : mcp.getPrompts()) {
            if (p.name == "annotate_risk") found = true;
        }
        expect(found, "annotate_risk prompt registered", passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: annotate_contracts prompt registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& p : mcp.getPrompts()) {
            if (p.name == "annotate_contracts") found = true;
        }
        expect(found, "annotate_contracts prompt registered", passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: annotate_full prompt registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& p : mcp.getPrompts()) {
            if (p.name == "annotate_full") found = true;
        }
        expect(found, "annotate_full prompt registered", passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: annotate_intent prompt returns message with instructions
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "prompts/get"},
                    {"params", {{"name", "annotate_intent"},
                                {"arguments", json::object()}}}};
        json resp = mcp.handleRequest(req);
        bool hasMessages = resp.contains("result") &&
                           resp["result"].contains("messages");
        bool mentionsSetAnnotation = false;
        bool mentionsGetUnannotated = false;
        if (hasMessages) {
            std::string text = resp["result"]["messages"][0]
                               ["content"].value("text", "");
            mentionsSetAnnotation =
                text.find("setSemanticAnnotation") != std::string::npos ||
                text.find("set_semantic_annotation") != std::string::npos;
            mentionsGetUnannotated =
                text.find("getUnannotatedNodes") != std::string::npos ||
                text.find("get_unannotated") != std::string::npos;
        }
        expect(hasMessages && mentionsSetAnnotation && mentionsGetUnannotated,
               "annotate_intent prompt mentions setAnnotation and getUnannotated",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: annotate_full prompt mentions save
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "prompts/get"},
                    {"params", {{"name", "annotate_full"},
                                {"arguments", json::object()}}}};
        json resp = mcp.handleRequest(req);
        bool mentionsSave = false;
        if (resp.contains("result") &&
            resp["result"].contains("messages")) {
            std::string text = resp["result"]["messages"][0]
                               ["content"].value("text", "");
            mentionsSave =
                text.find("save") != std::string::npos ||
                text.find("Save") != std::string::npos;
        }
        expect(mentionsSave,
               "annotate_full prompt mentions saving annotated AST",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: getUnannotatedNodes includes static analysis hints
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string src =
            "def greet(name):\n"
            "    return \"hello \" + name\n"
            "\n"
            "def process(data):\n"
            "    x = data + 1\n"
            "    y = x * 2\n"
            "    return y\n";

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        json resp = rpc(state, "s1", "getUnannotatedNodes",
                        {{"includeHints", true}});
        bool hasHints = false;
        if (resp.contains("result") && resp["result"].contains("nodes")) {
            for (const auto& n : resp["result"]["nodes"]) {
                if (n.contains("hints")) {
                    hasHints = true;
                    break;
                }
            }
        }

        expect(hasHints,
               "getUnannotatedNodes with includeHints returns static hints",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Hints include bodyStatements count
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string src =
            "def process(data):\n"
            "    x = data + 1\n"
            "    y = x * 2\n"
            "    return y\n";

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        json resp = rpc(state, "s1", "getUnannotatedNodes",
                        {{"includeHints", true}});
        int bodyStmts = -1;
        if (resp.contains("result") && resp["result"].contains("nodes")) {
            for (const auto& n : resp["result"]["nodes"]) {
                if (n.value("name", "") == "process" &&
                    n.contains("hints")) {
                    bodyStmts = n["hints"].value("bodyStatements", -1);
                }
            }
        }

        expect(bodyStmts >= 2,
               "Hints include bodyStatements=" + std::to_string(bodyStmts) +
               " for process()",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: All 5 prompts available via prompts/list
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "prompts/list"}};
        json resp = mcp.handleRequest(req);
        int annotatePrompts = 0;
        if (resp.contains("result") && resp["result"].contains("prompts")) {
            for (const auto& p : resp["result"]["prompts"]) {
                std::string name = p.value("name", "");
                if (name.find("annotate_") == 0)
                    ++annotatePrompts;
            }
        }

        expect(annotatePrompts >= 5,
               std::to_string(annotatePrompts) +
               " annotate_* prompts in prompts/list",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: annotate_risk prompt mentions callers/dependents
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "prompts/get"},
                    {"params", {{"name", "annotate_risk"},
                                {"arguments", json::object()}}}};
        json resp = mcp.handleRequest(req);
        bool mentionsCallers = false;
        if (resp.contains("result") && resp["result"].contains("messages")) {
            std::string text = resp["result"]["messages"][0]
                               ["content"].value("text", "");
            mentionsCallers =
                text.find("caller") != std::string::npos ||
                text.find("dependent") != std::string::npos ||
                text.find("Caller") != std::string::npos;
        }
        expect(mentionsCallers,
               "annotate_risk prompt mentions callers/dependents",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Total prompt count after Sprint 10
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        int total = (int)mcp.getPrompts().size();
        expect(total >= 9,
               "Total prompts: " + std::to_string(total) + " (>= 9 expected)",
               passed, failed);
    }

    std::cout << "\n=== Step 270 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
