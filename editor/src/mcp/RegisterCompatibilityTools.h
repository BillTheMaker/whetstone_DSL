    void registerCompatibilityTools() {
        tools_.push_back({"whetstone_get_compatibility_ledger",
            "Return the MCP compatibility ledger and runtime version header so agents can "
            "check whether known issues are fixed in the current runtime.",
            {{"type", "object"}, {"properties", {
                {"record_id", {{"type", "string"},
                    {"description", "Optional ledger record id filter"}}}
            }}}
        });
        toolHandlers_["whetstone_get_compatibility_ledger"] =
            [this](const nlohmann::json& args) {
                return runWhetstoneGetCompatibilityLedger(args);
            };
    }

    nlohmann::json runWhetstoneGetCompatibilityLedger(const nlohmann::json& args) {
        const std::string ledgerText = readFileText(kCompatibilityLedgerPath);
        nlohmann::json ledger = nlohmann::json::object();
        if (!ledgerText.empty()) {
            ledger = nlohmann::json::parse(ledgerText, nullptr, false);
            if (ledger.is_discarded()) {
                ledger = {
                    {"error", "Failed to parse compatibility ledger JSON"},
                    {"path", kCompatibilityLedgerPath}
                };
            }
        } else {
            ledger = {
                {"error", "Compatibility ledger not found"},
                {"path", kCompatibilityLedgerPath}
            };
        }

        if (args.contains("record_id") && ledger.is_object() && ledger.contains("records") &&
            ledger["records"].is_array()) {
            const std::string recordId = args.value("record_id", "");
            nlohmann::json filtered = nlohmann::json::array();
            for (const auto& rec : ledger["records"]) {
                if (!rec.is_object()) continue;
                if (rec.value("id", "") == recordId) filtered.push_back(rec);
            }
            ledger["records"] = filtered;
        }

        return {
            {"runtime", buildVersionHeader("whetstone_get_compatibility_ledger")},
            {"ledger", ledger}
        };
    }
