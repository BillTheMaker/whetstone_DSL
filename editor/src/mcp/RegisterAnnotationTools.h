    void registerAnnotationTools() {
        // whetstone_suggest_annotations
        tools_.push_back({"whetstone_suggest_annotations",
            "Get memory annotation suggestions for a code region. Returns suggestions "
            "with confidence scores and diagnostics. Specify nodeId or line/col.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Node ID to suggest annotations for"}}},
                {"line", {{"type", "integer"}, {"description", "Line number (0-based, alternative to nodeId)"}}},
                {"col", {{"type", "integer"}, {"description", "Column number (0-based, alternative to nodeId)"}}}
            }}}
        });
        toolHandlers_["whetstone_suggest_annotations"] = [this](const json& args) {
            return callWhetstone("getAnnotationSuggestions", args);
        };

        // whetstone_apply_annotation
        tools_.push_back({"whetstone_apply_annotation",
            "Apply a memory annotation suggestion to the AST. Pass the suggestion "
            "object from whetstone_suggest_annotations.",
            {{"type", "object"}, {"properties", {
                {"nodeId", {{"type", "string"}, {"description", "Target node ID"}}},
                {"annotationType", {{"type", "string"}, {"description", "e.g., ReclaimAnnotation, OwnerAnnotation"}}},
                {"strategy", {{"type", "string"}, {"description", "e.g., Tracing, Single, RAII"}}},
                {"reason", {{"type", "string"}, {"description", "Why this annotation"}}},
                {"confidence", {{"type", "number"}, {"description", "Confidence score 0-1"}}}
            }}, {"required", {"nodeId", "annotationType", "strategy"}}}
        });
        toolHandlers_["whetstone_apply_annotation"] = [this](const json& args) {
            return callWhetstone("applyAnnotationSuggestion", args);
        };

        // whetstone_generate_code
        tools_.push_back({"whetstone_generate_code",
            "Generate code from a natural language specification. Uses available "
            "library primitives when preferImports is true (default).",
            {{"type", "object"}, {"properties", {
                {"spec", {{"type", "string"}, {"description", "Natural language description of code to generate"}}},
                {"preferImports", {{"type", "boolean"}, {"description", "Prefer imported library symbols (default true)"}}}
            }}, {"required", {"spec"}}}
        });
        toolHandlers_["whetstone_generate_code"] = [this](const json& args) {
            return callWhetstone("generateCode", args);
        };

        // whetstone_run_pipeline
        tools_.push_back({"whetstone_run_pipeline",
            "Run the full Whetstone pipeline: parse source code, infer annotations, "
            "validate, optimize, and generate target language code.",
            {{"type", "object"}, {"properties", {
                {"source", {{"type", "string"}, {"description", "Source code to process"}}},
                {"sourceLanguage", {{"type", "string"}, {"description", "Source language (python, cpp, rust, go, java, javascript, typescript, elisp)"}}},
                {"targetLanguage", {{"type", "string"}, {"description", "Target language for code generation"}}}
            }}, {"required", {"source", "sourceLanguage", "targetLanguage"}}}
        });
        toolHandlers_["whetstone_run_pipeline"] = [this](const json& args) {
            return callWhetstone("runPipeline", args);
        };

        // whetstone_project_language
        tools_.push_back({"whetstone_project_language",
            "Project the current AST to a different target language. Adapts memory "
            "annotations appropriately and generates code in the target language.",
            {{"type", "object"}, {"properties", {
                {"targetLanguage", {{"type", "string"}, {"description", "Target language to project to"}}}
            }}, {"required", {"targetLanguage"}}}
        });
        toolHandlers_["whetstone_project_language"] = [this](const json& args) {
            return callWhetstone("projectLanguage", args);
        };
    }

    // ---------------------------------------------------------------
    //  Step 210: Register resources
    // ---------------------------------------------------------------
