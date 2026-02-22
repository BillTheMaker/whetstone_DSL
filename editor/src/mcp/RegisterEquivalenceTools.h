// Sprint 49 executable equivalence MCP tool (Step 726)
// Included inside MCPServer class body.

    void registerEquivalenceTools() {
        tools_.push_back({"whetstone_verify_executable_equivalence",
            "Run deterministic executable-equivalence checks between source and target outputs.",
            {{"type", "object"}, {"properties", {
                {"vectors", {{"type", "array"}}},
                {"property_trials", {{"type", "integer"}}},
                {"fuzz_seeds", {{"type", "array"}}}
            }}, {"required", nlohmann::json::array({"vectors"})}}
        });
        toolHandlers_["whetstone_verify_executable_equivalence"] =
            [this](const nlohmann::json& args) { return runVerifyExecutableEquivalence(args); };
    }

    nlohmann::json runVerifyExecutableEquivalence(const nlohmann::json& args) {
        if (!args.contains("vectors") || !args["vectors"].is_array()) {
            return {{"success", false}, {"error", "vectors_missing"}};
        }

        std::vector<TestVector> vectors;
        for (const auto& v : args["vectors"]) {
            vectors.push_back({
                v.value("id", ""),
                v.value("input", nlohmann::json::object()),
                v.value("expected", nlohmann::json::object())
            });
        }
        vectors = TestVectorSpec::normalize(std::move(vectors));

        auto rust = RustRunnerAdapter::run(vectors);
        auto cpp = CppRunnerAdapter::run(vectors);

        std::vector<DifferentialCaseResult> cases;
        for (size_t i = 0; i < vectors.size(); ++i) {
            cases.push_back({vectors[i].id, rust.outputs[i], cpp.outputs[i], false});
        }
        auto diff = DifferentialExecutionHarness::run(cases);

        int trials = args.value("property_trials", 16);
        auto prop = PropertyEquivalenceRunner::run(trials, 42);

        std::vector<int> seeds;
        for (const auto& s : args.value("fuzz_seeds", nlohmann::json::array({1, 2, 3}))) {
            if (s.is_number_integer()) seeds.push_back(s.get<int>());
        }
        auto fuzz = FuzzDifferentialRunner::run(seeds);

        auto evidence = EquivalenceEvidenceBundleModel::build(diff, prop, fuzz);

        bool equivalent = diff.totalCount > 0 && diff.equivalentCount == diff.totalCount && prop.success;
        return {
            {"success", true},
            {"equivalent", equivalent},
            {"differential", DifferentialExecutionHarness::toJson(diff)},
            {"property", PropertyEquivalenceRunner::toJson(prop)},
            {"fuzz", FuzzDifferentialRunner::toJson(fuzz)},
            {"evidence", EquivalenceEvidenceBundleModel::toJson(evidence)}
        };
    }
