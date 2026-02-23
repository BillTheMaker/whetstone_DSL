// Sprint 65: Upgrade Queue MCP tools (Steps 885-886)
// Included inside MCPServer class body.

    void registerUpgradeQueueTools() {
        tools_.push_back({"whetstone_enqueue_pair_upgrade",
            "Enqueue a language-pair upgrade with priority class.",
            {{"type", "object"}, {"properties", {
                {"pair_id", {{"type", "string"}}},
                {"priority", {{"type", "string"}}},
                {"entry_id", {{"type", "string"}}}
            }}, {"required", nlohmann::json::array({"pair_id", "priority"})}}
        });
        toolHandlers_["whetstone_enqueue_pair_upgrade"] =
            [this](const nlohmann::json& args) { return runEnqueuePairUpgrade(args); };

        tools_.push_back({"whetstone_get_upgrade_queue",
            "Get the current upgrade queue, sorted by priority.",
            {{"type", "object"}, {"properties", {
                {"limit", {{"type", "integer"}}}
            }}, {"required", nlohmann::json::array()}}
        });
        toolHandlers_["whetstone_get_upgrade_queue"] =
            [this](const nlohmann::json& args) { return runGetUpgradeQueue(args); };
    }

    nlohmann::json runEnqueuePairUpgrade(const nlohmann::json& args) {
        if (!args.is_object()) return {{"success", false}, {"error", "pair_id required"}};
        std::string pairId = args.value("pair_id", "");
        std::string priority = args.value("priority", "experimental");
        std::string entryId = args.value("entry_id", "");
        if (pairId.empty()) return {{"success", false}, {"error", "pair_id required"}};
        if (!UpgradePriorityPolicy::validate(priority))
            return {{"success", false}, {"error", "invalid priority class"}};
        auto pol = UpgradePriorityPolicy::classify(pairId, priority);
        if (entryId.empty()) entryId = "E-" + pairId + "-" + priority;
        UpgradeQueueEntry entry{entryId, pairId, pol.priorityClass, pol.priorityLevel, true};
        std::string err;
        if (!upgradeQueue_.enqueue(entry, &err))
            return {{"success", false}, {"error", err}};
        return {{"success", true}, {"entry_id", entryId}, {"pair_id", pairId},
                {"priority_class", pol.priorityClass}, {"priority_level", pol.priorityLevel},
                {"queue_size", upgradeQueue_.activeCount()}};
    }

    nlohmann::json runGetUpgradeQueue(const nlohmann::json& args) {
        int limit = 50;
        if (args.is_object()) limit = args.value("limit", 50);
        auto active = upgradeQueue_.getActive();
        nlohmann::json arr = nlohmann::json::array();
        int count = 0;
        for (const auto& e : active) {
            if (count >= limit) break;
            arr.push_back(PairUpgradeQueue::toJson(e));
            ++count;
        }
        return {{"success", true}, {"entries", arr},
                {"total_active", upgradeQueue_.activeCount()},
                {"shown", count}};
    }
