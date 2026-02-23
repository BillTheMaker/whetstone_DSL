#pragma once
// Step 884: Retry/backoff strategy for unstable pairs.
#include <string>
#include <nlohmann/json.hpp>

struct RetryDecision {
    std::string pairId;
    int attemptNumber = 0;
    int backoffMs = 0;
    bool shouldRetry = false;
    std::string reason;
};

class RetryBackoffStrategy {
public:
    static RetryDecision decide(const std::string& pairId, int attemptNumber,
                                 int maxAttempts = 3, int baseBackoffMs = 100) {
        RetryDecision d;
        d.pairId = pairId;
        d.attemptNumber = attemptNumber;
        d.shouldRetry = attemptNumber < maxAttempts;
        if (d.shouldRetry) {
            // Exponential backoff: base * 2^attempt
            int exp = 1;
            for (int i = 0; i < attemptNumber; ++i) exp *= 2;
            d.backoffMs = baseBackoffMs * exp;
            d.reason = "will_retry";
        } else {
            d.backoffMs = 0;
            d.reason = "max_attempts_reached";
        }
        return d;
    }

    static nlohmann::json toJson(const RetryDecision& d) {
        return {{"pair_id", d.pairId}, {"attempt_number", d.attemptNumber},
                {"backoff_ms", d.backoffMs}, {"should_retry", d.shouldRetry},
                {"reason", d.reason}};
    }
};
