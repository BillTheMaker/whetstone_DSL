    void registerReviewTools() {
        // whetstone_get_review_queue
        tools_.push_back({"whetstone_get_review_queue",
            "Get items currently awaiting human review with concise queue metadata.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_review_queue"] =
            [this](const json& args) {
                return callWhetstone("getReviewQueue", args);
            };

        // whetstone_get_review_context
        tools_.push_back({"whetstone_get_review_context",
            "Get full review context for one item: skeleton metadata, generated code, "
            "confidence, reasoning, and a human-readable summary.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Review item ID"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_get_review_context"] =
            [this](const json& args) {
                return callWhetstone("getReviewContext", args);
            };

        // whetstone_approve_item
        tools_.push_back({"whetstone_approve_item",
            "Approve a review item and mark it complete. Optional feedback is stored "
            "as reviewer note.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Review item ID"}}},
                {"feedback", {{"type", "string"},
                    {"description", "Optional reviewer note"}}}
            }}, {"required", json::array({"itemId"})}}
        });
        toolHandlers_["whetstone_approve_item"] =
            [this](const json& args) {
                return callWhetstone("approveReviewItem", args);
            };

        // whetstone_reject_item
        tools_.push_back({"whetstone_reject_item",
            "Reject a review item back to ready state. Feedback is required to explain "
            "what must change before re-review.",
            {{"type", "object"}, {"properties", {
                {"itemId", {{"type", "string"},
                    {"description", "Review item ID"}}},
                {"feedback", {{"type", "string"},
                    {"description", "Required rejection feedback"}}}
            }}, {"required", json::array({"itemId", "feedback"})}}
        });
        toolHandlers_["whetstone_reject_item"] =
            [this](const json& args) {
                return callWhetstone("rejectReviewItem", args);
            };

        // whetstone_set_review_policy
        tools_.push_back({"whetstone_set_review_policy",
            "Configure auto-approve rules for the review gate. Rules specify "
            "worker type, minimum confidence, and risk level thresholds.",
            {{"type", "object"}, {"properties", {
                {"policy", {{"type", "object"}, {"properties", {
                    {"defaultAction", {{"type", "string"},
                        {"enum", {"require-review", "auto-approve"}}}},
                    {"autoApproveRules", {{"type", "array"}, {"items", {{"type", "object"}}}}}
                }}}}
            }}}
        });
        toolHandlers_["whetstone_set_review_policy"] =
            [this](const json& args) {
                return callWhetstone("setReviewPolicy", args);
            };

        // whetstone_get_review_policy
        tools_.push_back({"whetstone_get_review_policy",
            "Get the current review policy including auto-approve rules.",
            {{"type", "object"}, {"properties", json::object()}}
        });
        toolHandlers_["whetstone_get_review_policy"] =
            [this](const json& args) {
                return callWhetstone("getReviewPolicy", args);
            };
    }

