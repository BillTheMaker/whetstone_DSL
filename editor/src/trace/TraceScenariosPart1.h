    // --- Scenario: Read & Understand ---
    Trace generateReadAndUnderstand(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "read_and_understand";
        trace.difficulty = "basic";
        trace.language = params.language;

        const auto& sample = pickSample(params.language);

        // User asks to understand the code
        trace.steps.push_back({"user", "Read the current AST and describe its structure. "
            "What functions are defined and what do they do?", "", {}, {}});

        // Assistant thinks
        trace.steps.push_back({"assistant", "I'll read the AST to understand the code structure.", "", {}, {}});

        // Tool call: getAST
        json astResult = simulateGetAST(sample.source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Assistant summarizes
        trace.steps.push_back({"assistant",
            "The module contains " + std::to_string(params.functionCount) +
            " functions in " + params.language + ". " + sample.description, "", {}, {}});

        return trace;
    }

    // --- Scenario: Add Annotations ---
    Trace generateAddAnnotations(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "add_annotations";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        const auto& sample = pickSample(params.language);

        trace.steps.push_back({"user",
            "Suggest memory annotations for all functions in the current module.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST first, then check each function for annotation suggestions.", "", {}, {}});

        // Get AST
        json astResult = simulateGetAST(sample.source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Suggest annotations
        json suggestInput = {{"nodeId", "mod1"}};
        json suggestResult = {
            {"scopeId", "mod1"},
            {"suggestions", json::array({{
                {"nodeId", "mod1"},
                {"annotationType", "ReclaimAnnotation"},
                {"strategy", "Tracing"},
                {"reason", "Python module uses garbage collection"},
                {"confidence", 0.85}
            }})},
            {"diagnostics", json::array()}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_suggest_annotations", suggestInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_suggest_annotations", {}, suggestResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_suggest_annotations");

        trace.steps.push_back({"assistant",
            "Found a suggestion: @Reclaim(Tracing) for the module (confidence: 0.85). "
            "This makes sense since Python uses garbage collection. Applying it.", "", {}, {}});

        // Apply annotation
        json applyInput = {
            {"nodeId", "mod1"}, {"annotationType", "ReclaimAnnotation"},
            {"strategy", "Tracing"}, {"reason", "Python GC"}, {"confidence", 0.85}
        };
        json applyResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_apply_annotation", applyInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_apply_annotation", {}, applyResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_apply_annotation");

        trace.steps.push_back({"assistant",
            "Applied @Reclaim(Tracing) annotation to the module. "
            "All functions now have appropriate memory strategy annotations.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Cross-Language ---
    Trace generateCrossLanguage(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "cross_language";
        trace.difficulty = "advanced";
        trace.language = params.language;

        std::string target = (params.language == "python") ? "cpp" : "python";

        trace.steps.push_back({"user",
            "Project this " + params.language + " code to " + target + ".", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll run the pipeline to project the code from " + params.language +
            " to " + target + ".", "", {}, {}});

        json pipelineInput = {
            {"source", pickSample(params.language).source},
            {"sourceLanguage", params.language},
            {"targetLanguage", target}
        };
        json pipelineResult = {
            {"success", true},
            {"generatedCode", "// Generated " + target + " code"},
            {"parseDiagnostics", json::array()},
            {"validationDiagnostics", json::array()},
            {"violations", json::array()},
            {"suggestions", json::array()},
            {"foldCount", 0}, {"dceCount", 0}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_run_pipeline", pipelineInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_run_pipeline", {}, pipelineResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_run_pipeline");

        trace.steps.push_back({"assistant",
            "Successfully projected the code to " + target + ". "
            "The annotations were adapted for the target language.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Refactor ---
    Trace generateRefactor(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "refactor";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Rename the first function in the module to 'compute'.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST, find the first function, and rename it.", "", {}, {}});

        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        json mutateInput = {
            {"type", "setProperty"}, {"nodeId", "fn1"},
            {"property", "name"}, {"value", "compute"}
        };
        json mutateResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_mutate", mutateInput, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_mutate", {}, mutateResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_mutate");

        trace.steps.push_back({"assistant",
            "Renamed the first function to 'compute'. The AST has been updated.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Security Audit ---
    Trace generateSecurityAudit(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "security_audit";
        trace.difficulty = "intermediate";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Check the project dependencies for security vulnerabilities.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll examine the AST to identify imported libraries, then check for vulnerabilities.", "", {}, {}});

        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        trace.steps.push_back({"assistant",
            "The module has no external imports with known vulnerabilities. "
            "The code uses standard library functions only.", "", {}, {}});

        return trace;
    }

    // --- Scenario: Multi-Step Debug ---
    Trace generateMultiStepDebug(const ScenarioParams& params) {
        Trace trace;
        trace.id = nextId();
        trace.scenario = "multi_step_debug";
        trace.difficulty = "advanced";
        trace.language = params.language;

        trace.steps.push_back({"user",
            "Check the code for annotation errors and fix them.", "", {}, {}});

        trace.steps.push_back({"assistant",
            "I'll get the AST, check for annotation issues, and fix any problems.", "", {}, {}});

        // Get AST
        json astResult = simulateGetAST(pickSample(params.language).source, params.language);
        trace.steps.push_back({"tool_call", "", "whetstone_get_ast", json::object(), {}});
        trace.steps.push_back({"tool_result", "", "whetstone_get_ast", {}, astResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_get_ast");

        // Suggest annotations
        json suggestResult = {
            {"scopeId", "mod1"},
            {"suggestions", json::array({{
                {"nodeId", "fn1"},
                {"annotationType", "ReclaimAnnotation"},
                {"strategy", "Tracing"},
                {"reason", "Unannotated function"},
                {"confidence", 0.7}
            }})},
            {"diagnostics", json::array({{
                {"severity", "warning"},
                {"message", "Function 'fn1' has no memory annotation"},
                {"nodeId", "fn1"}
            }})}
        };
        trace.steps.push_back({"tool_call", "", "whetstone_suggest_annotations", {{"nodeId", "fn1"}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_suggest_annotations", {}, suggestResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_suggest_annotations");

        trace.steps.push_back({"assistant",
            "Found a warning: function 'fn1' has no memory annotation. "
            "Suggested @Reclaim(Tracing) with confidence 0.7. Applying the fix.", "", {}, {}});

        // Apply fix
        json applyResult = {{"success", true}, {"warning", ""}};
        trace.steps.push_back({"tool_call", "", "whetstone_apply_annotation",
            {{"nodeId", "fn1"}, {"annotationType", "ReclaimAnnotation"},
             {"strategy", "Tracing"}, {"reason", "fix"}, {"confidence", 0.7}}, {}});
        trace.steps.push_back({"tool_result", "", "whetstone_apply_annotation", {}, applyResult});
        trace.toolCallCount += 1;
        addToolUsed(trace, "whetstone_apply_annotation");

        trace.steps.push_back({"assistant",
            "Fixed the annotation issue. All functions now have proper memory annotations.", "", {}, {}});

        return trace;
    }

