    void registerTrainingDataTools() {
        // whetstone_export_training_data
        tools_.push_back({"whetstone_export_training_data",
            "Export annotated code as training data for LLM fine-tuning. "
            "Supports HuggingFace (instruction/input/output JSONL) and "
            "PairsJSONL (raw_code/annotated_code) formats. Includes statistics.",
            {{"type", "object"}, {"properties", {
                {"format", {{"type", "string"},
                    {"description",
                     "Export format: 'huggingface' or 'pairs' (default: 'huggingface')"}}},
                {"languages", {{"type", "array"},
                    {"description",
                     "Languages to include (default: all available)"},
                    {"items", {{"type", "string"}}}}}
            }}}
        });
        toolHandlers_["whetstone_export_training_data"] =
            [this](const json& args) {
                return callWhetstone("exportTrainingData", args);
            };

        // whetstone_generate_examples
        tools_.push_back({"whetstone_generate_examples",
            "Generate annotated code examples with Semanno comments. "
            "Takes raw source code and language, returns annotated version "
            "with inferred annotations from all 8 subjects.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"},
                    {"description", "Source code to annotate"}}},
                {"language", {{"type", "string"},
                    {"description",
                     "Programming language (python, cpp, rust, etc.)"}}}
            }}, {"required", json::array({"source", "language"})}}
        });
        toolHandlers_["whetstone_generate_examples"] =
            [this](const json& args) {
                return callWhetstone("generateExamples", args);
            };
    }

