// Step 266 TDD Test: Semantic Annotation AST Node Types
//
// Tests 5 new semantic annotation types: IntentAnnotation, ComplexityAnnotation,
// RiskAnnotation, ContractAnnotation, SemanticTagAnnotation.
// Validates construction, JSON roundtrip, compact AST semantic fields,
// and attachment to Function nodes.
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
    //  Test 1: IntentAnnotation construction and fields
    // ---------------------------------------------------------------
    {
        IntentAnnotation a;
        a.summary = "validates user input before DB write";
        a.category = "validation";
        expect(a.conceptType == "IntentAnnotation" &&
               a.summary == "validates user input before DB write" &&
               a.category == "validation",
               "IntentAnnotation construction and fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: ComplexityAnnotation construction and fields
    // ---------------------------------------------------------------
    {
        ComplexityAnnotation a;
        a.timeComplexity = "O(n log n)";
        a.cognitiveComplexity = 7;
        a.linesOfLogic = 42;
        expect(a.conceptType == "ComplexityAnnotation" &&
               a.timeComplexity == "O(n log n)" &&
               a.cognitiveComplexity == 7 &&
               a.linesOfLogic == 42,
               "ComplexityAnnotation construction and fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: RiskAnnotation construction and fields
    // ---------------------------------------------------------------
    {
        RiskAnnotation a;
        a.level = "high";
        a.reason = "12 callers depend on exact return format";
        a.dependentCount = 12;
        expect(a.conceptType == "RiskAnnotation" &&
               a.level == "high" &&
               a.reason == "12 callers depend on exact return format" &&
               a.dependentCount == 12,
               "RiskAnnotation construction and fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: ContractAnnotation construction and fields
    // ---------------------------------------------------------------
    {
        ContractAnnotation a;
        a.preconditions = "name must be non-empty string";
        a.postconditions = "returns greeting string starting with 'hello'";
        a.returnShape = "str";
        a.sideEffects = "none";
        expect(a.conceptType == "ContractAnnotation" &&
               a.preconditions == "name must be non-empty string" &&
               a.returnShape == "str" &&
               a.sideEffects == "none",
               "ContractAnnotation construction and fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: SemanticTagAnnotation construction and fields
    // ---------------------------------------------------------------
    {
        SemanticTagAnnotation a;
        a.tags = {"@serialize", "@validation", "@auth"};
        expect(a.conceptType == "SemanticTagAnnotation" &&
               a.tags.size() == 3 &&
               a.tags[0] == "@serialize" &&
               a.tags[2] == "@auth",
               "SemanticTagAnnotation construction and fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: IntentAnnotation JSON roundtrip via Serialization.h
    // ---------------------------------------------------------------
    {
        IntentAnnotation orig;
        orig.id = "intent_1";
        orig.summary = "computes tax for order";
        orig.category = "computation";
        json j = toJson(&orig);

        auto* restored = static_cast<IntentAnnotation*>(fromJson(j));
        bool ok = restored &&
                  restored->conceptType == "IntentAnnotation" &&
                  restored->summary == "computes tax for order" &&
                  restored->category == "computation" &&
                  restored->id == "intent_1";
        delete restored;
        expect(ok, "IntentAnnotation JSON roundtrip",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: ComplexityAnnotation JSON roundtrip
    // ---------------------------------------------------------------
    {
        ComplexityAnnotation orig;
        orig.id = "cx_1";
        orig.timeComplexity = "O(n^2)";
        orig.cognitiveComplexity = 9;
        orig.linesOfLogic = 55;
        json j = toJson(&orig);

        auto* restored = static_cast<ComplexityAnnotation*>(fromJson(j));
        bool ok = restored &&
                  restored->timeComplexity == "O(n^2)" &&
                  restored->cognitiveComplexity == 9 &&
                  restored->linesOfLogic == 55;
        delete restored;
        expect(ok, "ComplexityAnnotation JSON roundtrip",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: RiskAnnotation + ContractAnnotation roundtrip
    // ---------------------------------------------------------------
    {
        RiskAnnotation risk;
        risk.id = "risk_1";
        risk.level = "critical";
        risk.reason = "payment processing";
        risk.dependentCount = 25;
        json rj = toJson(&risk);
        auto* rr = static_cast<RiskAnnotation*>(fromJson(rj));

        ContractAnnotation contract;
        contract.id = "ct_1";
        contract.preconditions = "amount > 0";
        contract.postconditions = "returns transaction_id";
        contract.returnShape = "str";
        contract.sideEffects = "network";
        json cj = toJson(&contract);
        auto* cr = static_cast<ContractAnnotation*>(fromJson(cj));

        bool ok = rr && rr->level == "critical" && rr->dependentCount == 25 &&
                  cr && cr->preconditions == "amount > 0" &&
                  cr->sideEffects == "network";
        delete rr;
        delete cr;
        expect(ok, "RiskAnnotation + ContractAnnotation roundtrip",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: SemanticTagAnnotation JSON roundtrip
    // ---------------------------------------------------------------
    {
        SemanticTagAnnotation orig;
        orig.id = "tag_1";
        orig.tags = {"@io", "@crypto"};
        json j = toJson(&orig);

        auto* restored = static_cast<SemanticTagAnnotation*>(fromJson(j));
        bool ok = restored &&
                  restored->tags.size() == 2 &&
                  restored->tags[0] == "@io" &&
                  restored->tags[1] == "@crypto";
        delete restored;
        expect(ok, "SemanticTagAnnotation JSON roundtrip",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Attach semantic annotations to Function node
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string src = "def greet(name):\n    return \"hello \" + name\n";
        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Find function node
        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Attach IntentAnnotation via applyMutation insertNode
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "greets a person by name"},
                                       {"category", "io"}}}}}});

        // Check it's there
        bool found = false;
        ASTNode* fn = findNodeById(state.activeAST(), fnId);
        if (fn) {
            for (auto* child : fn->getChildren("annotations")) {
                if (child->conceptType == "IntentAnnotation") {
                    auto* ia = static_cast<IntentAnnotation*>(child);
                    if (ia->summary == "greets a person by name")
                        found = true;
                }
            }
        }

        expect(!fnId.empty() && found,
               "Attach IntentAnnotation to Function via applyMutation",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Compact AST includes semantic summary field
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string src = "def greet(name):\n    return \"hello \" + name\n";
        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Add intent + risk annotations
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "greets user"},
                                       {"category", "io"}}}}}});

        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"},
                                       {"reason", "simple function"},
                                       {"dependentCount", 2}}}}}});

        // Get compact AST
        json resp = rpc(state, "s1", "getAST", {{"compact", true}});
        json nodes = resp["result"]["nodes"];

        // Find greet node in compact output
        bool hasSemantic = false;
        for (const auto& n : nodes) {
            if (n.value("name", "") == "greet" &&
                n.contains("semantic")) {
                auto sem = n["semantic"];
                hasSemantic = sem.contains("intent") &&
                              sem.contains("risk");
            }
        }

        expect(hasSemantic,
               "Compact AST includes semantic summary for annotated function",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: All 5 annotation types survive AST clone (undo roundtrip)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string src = "def greet(name):\n    return \"hello \" + name\n";
        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Add all 5 annotation types
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "test"}, {"category", "io"}}}}}});

        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ComplexityAnnotation"},
                       {"properties", {{"timeComplexity", "O(1)"},
                                       {"cognitiveComplexity", 1},
                                       {"linesOfLogic", 2}}}}}});

        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"},
                                       {"reason", "trivial"},
                                       {"dependentCount", 0}}}}}});

        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ContractAnnotation"},
                       {"properties", {{"preconditions", "name is str"},
                                       {"postconditions", "returns str"},
                                       {"returnShape", "str"},
                                       {"sideEffects", "none"}}}}}});

        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "SemanticTagAnnotation"},
                       {"properties", {{"tags", json::array({"@io"})}} }}}});

        // Serialize AST to JSON and back (simulates undo/clone roundtrip)
        json astJson = toJson(state.activeAST());
        auto* restored = static_cast<Module*>(fromJson(astJson));

        int annoCount = 0;
        for (auto* c : restored->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "IntentAnnotation" ||
                        a->conceptType == "ComplexityAnnotation" ||
                        a->conceptType == "RiskAnnotation" ||
                        a->conceptType == "ContractAnnotation" ||
                        a->conceptType == "SemanticTagAnnotation")
                        ++annoCount;
                }
            }
        }
        delete restored;

        expect(annoCount == 5,
               "All 5 semantic annotation types survive JSON roundtrip (" +
               std::to_string(annoCount) + "/5)",
               passed, failed);
    }

    std::cout << "\n=== Step 266 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
