    void registerBatchTools() {
        // whetstone_batch_query
        tools_.push_back({"whetstone_batch_query",
            "Execute multiple queries in a single round-trip. "
            "Pass an array of {method, params} objects. Each sub-query "
            "runs independently; errors in one don't affect others.",
            {{"type", "object"},
             {"properties", {
                 {"queries", {{"type", "array"},
                     {"items", {{"type", "object"},
                         {"properties", {
                             {"method", {{"type", "string"}}},
                             {"params", {{"type", "object"}}}
                         }},
                         {"required", json::array({"method"})}
                     }}
                 }}
             }},
             {"required", json::array({"queries"})}}
        });
        toolHandlers_["whetstone_batch_query"] = [this](const json& args) {
            return callWhetstone("batchQuery", args);
        };
    }

    // ---------------------------------------------------------------
    //  Project management tools
    // ---------------------------------------------------------------
