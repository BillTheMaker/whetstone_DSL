#pragma once
// Step 754: dynamic strictness policy engine.

#include <string>

#include <nlohmann/json.hpp>

struct DynamicStrictnessPolicy {
    std::string mode = "balanced";
    bool allowImplicitAny = false;
    bool requireReviewOnDynamicDispatch = true;
};

class DynamicStrictnessPolicyEngine {
public:
    static DynamicStrictnessPolicy forMode(const std::string& mode) {
        DynamicStrictnessPolicy p;
        p.mode = mode;
        if (mode == "lenient") {
            p.allowImplicitAny = true;
            p.requireReviewOnDynamicDispatch = false;
        } else if (mode == "strict") {
            p.allowImplicitAny = false;
            p.requireReviewOnDynamicDispatch = true;
        } else {
            p.mode = "balanced";
            p.allowImplicitAny = false;
            p.requireReviewOnDynamicDispatch = true;
        }
        return p;
    }

    static nlohmann::json toJson(const DynamicStrictnessPolicy& p) {
        return {{"mode", p.mode}, {"allow_implicit_any", p.allowImplicitAny},
                {"require_review_on_dynamic_dispatch", p.requireReviewOnDynamicDispatch}};
    }
};
