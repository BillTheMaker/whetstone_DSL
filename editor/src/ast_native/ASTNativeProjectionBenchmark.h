#pragma once
// Step 777: AST-native projection benchmark suite.

#include <nlohmann/json.hpp>

struct ASTNativeProjectionBenchmark {
    double equivalenceConfidence = 0.0;
    double reviewRequiredRate = 0.0;
    int corpusCount = 0;
};

class ASTNativeProjectionBenchmarkSuite {
public:
    static ASTNativeProjectionBenchmark run(int total, int reviewRequired) {
        ASTNativeProjectionBenchmark b;
        b.corpusCount = total;
        if (total <= 0) {
            b.equivalenceConfidence = 0.0;
            b.reviewRequiredRate = 1.0;
            return b;
        }
        b.reviewRequiredRate = static_cast<double>(reviewRequired) / static_cast<double>(total);
        b.equivalenceConfidence = 1.0 - b.reviewRequiredRate;
        return b;
    }

    static nlohmann::json toJson(const ASTNativeProjectionBenchmark& b) {
        return {
            {"equivalence_confidence", b.equivalenceConfidence},
            {"review_required_rate", b.reviewRequiredRate},
            {"corpus_count", b.corpusCount}
        };
    }
};
