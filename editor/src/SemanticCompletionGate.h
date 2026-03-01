#pragma once
// Step 1879: Semantic completion gate.
// Closes GR-010: ImprovementPromotionGate had a threshold of 0.02 (nearly anything passes)
// and no depth tracking. SemanticCompletionGate enforces a meaningful semantic score
// threshold and caps recursive completion depth to prevent unbounded iteration.

#include <string>
#include <vector>

struct SemanticCompletionResult {
    bool passed = false;
    std::string reason;
    int depth = 0;
    float score = 0.0f;
};

class SemanticCompletionGate {
public:
    static constexpr float kMinScore = 0.70f;
    static constexpr int   kMaxDepth = 5;

    // Evaluate whether a completion attempt should be promoted or retried.
    // score: semantic equivalence/coverage score in [0,1]
    // depth: current recursive completion depth (starts at 0)
    // humanApproved: whether human review has been applied
    static SemanticCompletionResult evaluate(float score,
                                             int depth,
                                             bool humanApproved = false) {
        SemanticCompletionResult result;
        result.score = score;
        result.depth = depth;

        if (depth > kMaxDepth) {
            result.passed = false;
            result.reason = "max_depth_exceeded";
            return result;
        }

        if (score < kMinScore) {
            result.passed = false;
            result.reason = "score_below_semantic_threshold";
            return result;
        }

        // At or above threshold: pass if human-approved or on a stable path
        result.passed = humanApproved || score >= 0.90f;
        result.reason = result.passed ? "accepted" : "human_approval_required";
        return result;
    }
};
