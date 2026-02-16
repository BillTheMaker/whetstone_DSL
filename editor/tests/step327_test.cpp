// Step 327: WorkerRegistry — Worker Abstractions (12 tests)
// Tests worker interface, concrete workers, registry lookup,
// context assembly, canHandle, rejection feedback.

#include "WorkerRegistry.h"
#include <iostream>
#include <string>
#include <set>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& name,
                          const std::string& contextWidth = "local") {
    WorkItem wi;
    wi.id = "wi-" + name;
    wi.nodeId = "n-" + name;
    wi.nodeName = name;
    wi.nodeType = "Function";
    wi.bufferId = "buf";
    wi.contextWidth = contextWidth;
    wi.priority = "medium";
    wi.createdAt = workItemTimestamp();
    return wi;
}

static WorkerContext makeContext(const std::string& intent = "",
                                 const std::string& feedback = "") {
    WorkerContext ctx;
    ctx.nodeAst = json{{"type", "Function"}, {"name", "test"}};
    ctx.bufferContent = "# test buffer";
    ctx.skeletonIntent = intent;
    ctx.feedbackFromRejection = feedback;
    return ctx;
}

// 1. Deterministic worker produces code for simple skeleton
void test_deterministic_worker() {
    TEST(deterministic_worker);
    DeterministicWorker w;
    CHECK(w.type() == "deterministic", "type");

    auto wi = makeItem("compute");
    auto ctx = makeContext("return x + y");
    auto result = w.execute(wi, ctx);

    CHECK(!result.generatedCode.empty(), "produces code");
    CHECK(result.confidence > 0.8f, "high confidence with intent");
    CHECK(result.reasoning.find("Intent") != std::string::npos, "reasoning mentions intent");
    PASS();
}

// 2. Template worker fills getter pattern
void test_template_getter() {
    TEST(template_getter);
    TemplateWorker w;
    CHECK(w.type() == "template", "type");

    auto wi = makeItem("getName");
    auto ctx = makeContext();
    auto result = w.execute(wi, ctx);

    CHECK(result.generatedCode.find("self.name") != std::string::npos,
          "getter returns field: " + result.generatedCode);
    CHECK(result.confidence > 0.9f, "high confidence");
    PASS();
}

// 3. Template worker fills setter pattern
void test_template_setter() {
    TEST(template_setter);
    TemplateWorker w;
    auto wi = makeItem("setValue");
    auto ctx = makeContext();
    auto result = w.execute(wi, ctx);

    CHECK(result.generatedCode.find("self.value") != std::string::npos,
          "setter sets field: " + result.generatedCode);
    CHECK(result.confidence > 0.9f, "high confidence");
    PASS();
}

// 4. Agent worker (LLM) prepares context bundle
void test_llm_agent_context() {
    TEST(llm_agent_context);
    LLMAgentWorker w;
    CHECK(w.type() == "llm", "type");

    auto wi = makeItem("complexAlgo", "project");
    auto ctx = makeContext();
    ctx.projectSummary = json{{"buffers", 3}};
    auto result = w.execute(wi, ctx);

    CHECK(result.confidence == 0.0f, "deferred confidence");
    CHECK(result.generatedCode.empty(), "no code generated");
    CHECK(result.astJson.contains("workerType"), "has workerType in bundle");
    CHECK(result.astJson["workerType"] == "llm", "llm in bundle");
    CHECK(result.tokensBudget == 8000, "project budget");
    PASS();
}

// 5. Human worker marks for review
void test_human_worker() {
    TEST(human_worker);
    HumanWorker w;
    CHECK(w.type() == "human", "type");

    auto wi = makeItem("ambiguousLogic");
    auto ctx = makeContext();
    auto result = w.execute(wi, ctx);

    CHECK(result.confidence == 0.0f, "deferred confidence");
    CHECK(result.astJson.contains("status"), "has status");
    CHECK(result.astJson["status"] == "needs-human", "needs-human status");
    CHECK(result.reasoning.find("human") != std::string::npos, "reasoning");
    PASS();
}

// 6. Registry lookup
void test_registry_lookup() {
    TEST(registry_lookup);
    auto reg = WorkerRegistry::getDefaultRegistry();

    CHECK(reg.getWorker("deterministic") != nullptr, "has deterministic");
    CHECK(reg.getWorker("template") != nullptr, "has template");
    CHECK(reg.getWorker("slm") != nullptr, "has slm");
    CHECK(reg.getWorker("llm") != nullptr, "has llm");
    CHECK(reg.getWorker("human") != nullptr, "has human");
    CHECK(reg.getWorker("nonexistent") == nullptr, "null for unknown");
    PASS();
}

// 7. canHandle filtering
void test_can_handle() {
    TEST(can_handle);
    DeterministicWorker det;
    TemplateWorker tmpl;
    SLMAgentWorker slm;
    LLMAgentWorker llm;

    auto localItem = makeItem("compute", "local");
    auto projectItem = makeItem("compute", "project");
    auto getterItem = makeItem("getName", "local");

    CHECK(det.canHandle(localItem), "deterministic handles local");
    CHECK(!det.canHandle(projectItem), "deterministic rejects project");
    CHECK(tmpl.canHandle(getterItem), "template handles getter");
    CHECK(!tmpl.canHandle(localItem), "template rejects non-getter");
    CHECK(slm.canHandle(localItem), "slm handles local");
    CHECK(!slm.canHandle(projectItem), "slm rejects project");
    CHECK(llm.canHandle(projectItem), "llm handles anything");
    PASS();
}

// 8. Worker type strings
void test_worker_type_strings() {
    TEST(worker_type_strings);
    auto reg = WorkerRegistry::getDefaultRegistry();
    auto types = reg.registeredTypes();
    std::set<std::string> typeSet(types.begin(), types.end());

    CHECK(typeSet.count("deterministic"), "has deterministic");
    CHECK(typeSet.count("template"), "has template");
    CHECK(typeSet.count("slm"), "has slm");
    CHECK(typeSet.count("llm"), "has llm");
    CHECK(typeSet.count("human"), "has human");
    CHECK(typeSet.size() == 5, "5 types total");
    PASS();
}

// 9. Context assembly — SLM includes context bundle
void test_slm_context_assembly() {
    TEST(slm_context_assembly);
    SLMAgentWorker w;
    auto wi = makeItem("parseInput", "file");
    auto ctx = makeContext();
    ctx.bufferContent = "def parseInput(data): pass";

    auto result = w.execute(wi, ctx);
    CHECK(result.astJson.contains("bufferContent"), "includes buffer content");
    CHECK(result.astJson["workerType"] == "slm", "slm in bundle");
    CHECK(result.tokensBudget == 2000, "file budget");
    PASS();
}

// 10. Rejection feedback included in context
void test_rejection_feedback() {
    TEST(rejection_feedback);
    LLMAgentWorker w;
    auto wi = makeItem("retryFunc", "project");
    auto ctx = makeContext("", "needs better error handling");

    auto result = w.execute(wi, ctx);
    CHECK(result.reasoning.find("feedback") != std::string::npos, "feedback in reasoning");
    CHECK(result.reasoning.find("error handling") != std::string::npos,
          "specific feedback preserved");
    PASS();
}

// 11. Deterministic worker without intent → low confidence
void test_deterministic_no_intent() {
    TEST(deterministic_no_intent);
    DeterministicWorker w;
    auto wi = makeItem("mystery");
    auto ctx = makeContext(); // no intent

    auto result = w.execute(wi, ctx);
    CHECK(result.confidence < 0.6f, "low confidence without intent");
    CHECK(result.generatedCode.find("TODO") != std::string::npos, "generates stub");
    PASS();
}

// 12. Boolean accessor (isX) template
void test_boolean_accessor() {
    TEST(boolean_accessor);
    TemplateWorker w;
    auto wi = makeItem("isActive");
    auto ctx = makeContext();
    auto result = w.execute(wi, ctx);

    CHECK(result.generatedCode.find("self.active") != std::string::npos,
          "boolean accessor: " + result.generatedCode);
    CHECK(result.confidence > 0.9f, "high confidence");
    PASS();
}

int main() {
    std::cout << "=== Step 327: WorkerRegistry Worker Abstractions ===\n";
    try {
        test_deterministic_worker();
        test_template_getter();
        test_template_setter();
        test_llm_agent_context();
        test_human_worker();
        test_registry_lookup();
        test_can_handle();
        test_worker_type_strings();
        test_slm_context_assembly();
        test_rejection_feedback();
        test_deterministic_no_intent();
        test_boolean_accessor();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
