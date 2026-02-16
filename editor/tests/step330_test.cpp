// Step 330: Review Gates (12 tests)
// Tests auto-approve rules, review policy, confidence thresholds,
// @Review override, custom policies, serialization, MCP tools.

#include "ReviewGate.h"
#include "MCPServer.h"
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& name, const std::string& workerType,
                          bool reviewReq = false) {
    WorkItem wi;
    wi.id = "wi-" + name;
    wi.nodeId = "n-" + name;
    wi.nodeName = name;
    wi.nodeType = "Function";
    wi.workerType = workerType;
    wi.reviewRequired = reviewReq;
    wi.priority = "medium";
    wi.createdAt = workItemTimestamp();
    return wi;
}

static WorkItemResult makeResult(float confidence) {
    WorkItemResult r;
    r.generatedCode = "return 42";
    r.confidence = confidence;
    return r;
}

ReviewGate gate;

// 1. Deterministic + high confidence → auto-approve
void test_deterministic_auto_approve() {
    TEST(deterministic_auto_approve);
    auto wi = makeItem("compute", "deterministic");
    auto result = makeResult(0.95f);
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(decision.approved, "auto-approved");
    CHECK(decision.ruleMatched.find("deterministic") != std::string::npos, "matched det rule");
    PASS();
}

// 2. LLM result → require review
void test_llm_requires_review() {
    TEST(llm_requires_review);
    auto wi = makeItem("complex", "llm");
    auto result = makeResult(0.8f);
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(!decision.approved, "not auto-approved");
    PASS();
}

// 3. Low confidence → require review even if deterministic
void test_low_confidence_rejected() {
    TEST(low_confidence_rejected);
    auto wi = makeItem("compute", "deterministic");
    auto result = makeResult(0.4f); // below 0.9 threshold
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(!decision.approved, "not approved with low confidence");
    PASS();
}

// 4. Template + high confidence → auto-approve
void test_template_auto_approve() {
    TEST(template_auto_approve);
    auto wi = makeItem("getName", "template");
    auto result = makeResult(0.95f);
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(decision.approved, "template auto-approved");
    PASS();
}

// 5. Custom policy respected
void test_custom_policy() {
    TEST(custom_policy);
    ReviewPolicy policy;
    policy.defaultAction = "require-review";
    AutoApproveRule rule;
    rule.workerType = "llm";
    rule.minConfidence = 0.99f;
    policy.autoApproveRules.push_back(rule);

    auto wi = makeItem("func", "llm");
    auto result = makeResult(0.995f);
    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(decision.approved, "llm approved with custom 0.99 threshold");
    PASS();
}

// 6. Default policy behavior
void test_default_policy() {
    TEST(default_policy);
    auto policy = ReviewPolicy::getDefault();
    CHECK(policy.defaultAction == "require-review", "default is require-review");
    CHECK(policy.autoApproveRules.size() == 2, "2 default rules");
    CHECK(policy.autoApproveRules[0].workerType == "deterministic", "first=deterministic");
    CHECK(policy.autoApproveRules[1].workerType == "template", "second=template");
    PASS();
}

// 7. Explicit @Review(required) overrides auto-approve
void test_review_required_override() {
    TEST(review_required_override);
    auto wi = makeItem("sensitive", "deterministic", true); // reviewRequired=true
    auto result = makeResult(0.99f);
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(!decision.approved, "not approved despite high confidence");
    CHECK(decision.ruleMatched.find("review-annotation") != std::string::npos,
          "overridden by @Review");
    PASS();
}

// 8. Wildcard rule matches any worker
void test_wildcard_rule() {
    TEST(wildcard_rule);
    ReviewPolicy policy;
    policy.defaultAction = "require-review";
    AutoApproveRule rule;
    rule.workerType = "*";
    rule.minConfidence = 0.95f;
    policy.autoApproveRules.push_back(rule);

    auto wi = makeItem("func", "slm");
    auto result = makeResult(0.96f);
    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(decision.approved, "wildcard matches slm");
    PASS();
}

// 9. Policy serialization roundtrip
void test_policy_roundtrip() {
    TEST(policy_roundtrip);
    auto policy = ReviewPolicy::getDefault();
    json j = policy.toJson();
    auto policy2 = ReviewPolicy::fromJson(j);

    CHECK(policy2.defaultAction == policy.defaultAction, "defaultAction");
    CHECK(policy2.autoApproveRules.size() == policy.autoApproveRules.size(), "rules count");
    CHECK(policy2.autoApproveRules[0].workerType == "deterministic", "first rule type");
    CHECK(policy2.autoApproveRules[0].minConfidence > 0.89f, "first rule confidence");
    PASS();
}

// 10. Empty policy uses default action
void test_empty_policy() {
    TEST(empty_policy);
    ReviewPolicy policy;
    policy.defaultAction = "auto-approve";
    // No rules

    auto wi = makeItem("func", "llm");
    auto result = makeResult(0.5f);
    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(decision.approved, "default auto-approve");
    CHECK(decision.ruleMatched.find("default") != std::string::npos, "matched default");
    PASS();
}

// 11. Default require-review with no matching rules
void test_default_require_review() {
    TEST(default_require_review);
    ReviewPolicy policy;
    policy.defaultAction = "require-review";
    // No rules

    auto wi = makeItem("func", "llm");
    auto result = makeResult(0.95f);
    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(!decision.approved, "requires review by default");
    PASS();
}

// 12. Human worker always requires review (no rule matches)
void test_human_requires_review() {
    TEST(human_requires_review);
    auto wi = makeItem("ambiguous", "human");
    auto result = makeResult(0.0f);
    auto policy = ReviewPolicy::getDefault();

    auto decision = gate.shouldAutoApprove(wi, result, policy);
    CHECK(!decision.approved, "human not auto-approved");
    PASS();
}

int main() {
    std::cout << "=== Step 330: Review Gates ===\n";
    try {
        test_deterministic_auto_approve();
        test_llm_requires_review();
        test_low_confidence_rejected();
        test_template_auto_approve();
        test_custom_policy();
        test_default_policy();
        test_review_required_override();
        test_wildcard_rule();
        test_policy_roundtrip();
        test_empty_policy();
        test_default_require_review();
        test_human_requires_review();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
