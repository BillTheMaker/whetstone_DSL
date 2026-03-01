// Step 1886: MCP wiring for language fitness scoring
//
// Registers:
//   whetstone_score_language_fitness
//
// This file is #included inside the MCPServer class body.

    void registerLanguageFitnessTools() {
        tools_.push_back({"whetstone_score_language_fitness",
            "Analyze an AST subtree or spec text and return a ranked list of languages "
            "best suited to implement it, with per-language score and rationale.",
            {{"type", "object"}, {"properties", {
                {"ast", {{"type", "object"},
                    {"description", "JSON AST subtree to score (optional if spec_text provided)."}}},
                {"spec_text", {{"type", "string"},
                    {"description", "Spec text section to extract features from (used when ast absent)."}}},
                {"mutation_ratio", {{"type", "number"},
                    {"description", "Override mutation ratio [0,1] (optional)."}}},
                {"recursion_hint", {{"type", "string"},
                    {"description", "Override recursion shape: none/tail/tree/loop (optional)."}}},
                {"concurrency_hint", {{"type", "string"},
                    {"description", "Override concurrency: none/channels/shared/actors (optional)."}}},
                {"io_hint", {{"type", "string"},
                    {"description", "Override I/O pattern: none/blocking/async/event (optional)."}}},
                {"type_hint", {{"type", "string"},
                    {"description", "Override type complexity: none/generics/dependent (optional)."}}}
            }}, {"required", json::array()}}
        });
        toolHandlers_["whetstone_score_language_fitness"] =
            [this](const json& args) -> json {
                return runScoreLanguageFitness(args);
            };
    }

    json runScoreLanguageFitness(const json& args) {
        using AFF = whetstone::ASTFeatures;

        AFF features;

        // Extract from AST if provided
        if (args.contains("ast") && args["ast"].is_object()) {
            features = whetstone::ASTFeatureExtractor::extract(args["ast"]);
        }

        // Override individual dimensions if specified
        if (args.contains("mutation_ratio") && args["mutation_ratio"].is_number()) {
            features.mutationRatio = args["mutation_ratio"].get<float>();
        }
        if (args.contains("recursion_hint") && args["recursion_hint"].is_string()) {
            std::string h = args["recursion_hint"].get<std::string>();
            if (h == "tail")  features.recursionShape = AFF::RecursionShape::TailRecursive;
            else if (h == "tree") features.recursionShape = AFF::RecursionShape::TreeRecursive;
            else if (h == "loop") features.recursionShape = AFF::RecursionShape::FlatLoop;
            else features.recursionShape = AFF::RecursionShape::None;
        }
        if (args.contains("concurrency_hint") && args["concurrency_hint"].is_string()) {
            std::string h = args["concurrency_hint"].get<std::string>();
            if (h == "channels") features.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Channels;
            else if (h == "shared") features.concurrencyPrimitive = AFF::ConcurrencyPrimitive::SharedMemory;
            else if (h == "actors") features.concurrencyPrimitive = AFF::ConcurrencyPrimitive::Actors;
            else features.concurrencyPrimitive = AFF::ConcurrencyPrimitive::None;
        }
        if (args.contains("io_hint") && args["io_hint"].is_string()) {
            std::string h = args["io_hint"].get<std::string>();
            if (h == "async") features.ioPattern = AFF::IOPattern::Async;
            else if (h == "event") features.ioPattern = AFF::IOPattern::EventDriven;
            else if (h == "blocking") features.ioPattern = AFF::IOPattern::Blocking;
            else features.ioPattern = AFF::IOPattern::None;
        }
        if (args.contains("type_hint") && args["type_hint"].is_string()) {
            std::string h = args["type_hint"].get<std::string>();
            if (h == "generics") features.typeComplexity = AFF::TypeComplexity::SimpleGenerics;
            else if (h == "dependent") features.typeComplexity = AFF::TypeComplexity::DependentTypes;
            else features.typeComplexity = AFF::TypeComplexity::None;
        }

        json ranked = whetstone::LanguageFitnessScorer::score(features);
        return {{"success", true}, {"ranked", ranked}};
    }
